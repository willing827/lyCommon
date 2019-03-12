#include <sqnet/sqnet2.h>
#include <sqnet/sqnetclienttt.h>
#define ktLoggerId "ViewNetLib"
#include <sqstd/sqinc.h>


using namespace snqu::safe;
namespace snqu{ namespace net2 {

    NetClientExt::NetClientExt()
        : m_started(false)
        , m_p_connection(NULL)
    {
        m_sequence = 0;
        m_remote_port = 0;
        m_send_wait_second = 5;
        m_reconnect_time = 15; // second
        m_reconn = false;
        m_connected = false;
    }

    NetClientExt::~NetClientExt()
    {
        SAFE_DELETE(m_p_connection);
    }

    EnHandleResult NetClientExt::OnSend(IClient* pClient, const BYTE* pData, int iLength)
    {
        return HR_OK;
    }

    EnHandleResult NetClientExt::OnReceive(IClient* pClient, const BYTE* pData, int iLength)
    {
        m_parser.add_buffer((char*)pData, iLength);
        handle_packet_proc();
        return HR_OK;
    }

    EnHandleResult NetClientExt::OnClose(IClient* pClient)
    {
		m_connected = false;
       //SNLOG((kTrace) << "NetClientExt OnClose called";
        if (NULL != m_p_connection && m_p_connection->GetConnectionID() == pClient->GetConnectionID())
        {
            m_p_connection = NULL;
        }
        m_sequence = 0;
        m_parser.clear_encry_key();
        on_close();
        return HR_OK;
    }

    EnHandleResult NetClientExt::OnError(IClient* pClient, EnSocketOperation enOperation, int iErrorCode)
    {
        m_connected = false;
        if (SO_CONNECT == enOperation)
        {
            return HR_ERROR;
        }
        else
        {
           //SNLOG((kError) << "Network OnError, code: " << iErrorCode << ", oper: " << enOperation;
            disconnect();
            OnClose(pClient);
        }
        return HR_ERROR;
    }

    EnHandleResult NetClientExt::OnConnect(IClient* pClient)
    {
        m_p_connection = pClient;
       //SNLOG((kTrace) << "NetClientExt OnConnect called ";
        // 连接第一个包要去获取通讯加密的密钥
        get_encry_key(pClient);
        return HR_OK;
    }

    bool NetClientExt::start(int reconn_time, int send_wait_time)
    {
        static bool called = false;
        if (!called)
        {
            called = true;
            //SQInitLogger(ktLoggerId);
           //SNLOG((kTrace) << "NetClientExt constructed.";
        }

        m_send_wait_second = send_wait_time;
        m_reconnect_time = reconn_time;

        if (m_started)
            return true;

        m_task_thread.begin();
        // 必跳
        m_task_thread.SetTimer(9, HeartbeatDetectionSeconds * 1000, 
            std::bind(&NetClientExt::hearbeat_timer_proc, this, std::placeholders::_1));

        m_started = true;
        return true;
    }

    void NetClientExt::stop()
    {
        if (!m_started)
            return;

       //SNLOG((kInfo) << "NetClientExt stop called";
        m_started = false;
        m_task_thread.KillAllTimer();
        m_reconn = false;
        m_task_thread.end();

        if (m_connected)
        {
            disconnect();
            clear_stop_client();
            m_connected = false;
        }
    }

    bool NetClientExt::connect(const std::string& ipaddr, unsigned short port, bool async, bool is_auto_reconn)
    {
        if (m_reconn)
        {
            if (is_auto_reconn == false)
            {
                m_reconn = false;
                m_task_thread.KillTimer(10);
                return connect(async);
            }
            else
                return false;
        }

        m_parser.m_on_get_seesion_cb = std::bind(&NetClientExt::on_get_encry_key, this);
        m_remote_port = port;
        m_remote_ip = ipaddr;
        
        bool ret= connect(async);

        if (is_auto_reconn && !ret)
            reconnect();

        return ret;
    }

    bool NetClientExt::connect(bool async)
    {
        if (m_connected) return true;

        IClient* client = NULL;
        bool ret = false;
        do 
        {
            if (nullptr == m_p_connection)
            {
                client = Ye_Create_TcpClient(reinterpret_cast<ITcpClientListener *>(this));
                if (client == nullptr)
                {
                   //SNLOG((kError) << "Ye_Create_TcpClient failed";
                    ret = false;
                    break;
                }
            }
            else
            {
                if (m_p_connection->GetState() == SS_STARTED)
                {
                    ret = true;
                    break;
                }
                else
                {
                   //SNLOG((kWarning) << "m_p_connection status is:" << m_p_connection->GetState();
                }
            }

           //SNLOG((kInfo) << "NetClientExt Connect Start session_id:" << m_session_id;
            if (client->Start(m_remote_ip.c_str(), m_remote_port, async ? TRUE : FALSE))
            {
                std::unique_lock<std::mutex> lck(m_unique_lock);
                if (std::cv_status::timeout == m_condition.wait_for(lck, std::chrono::seconds(m_send_wait_second)))
                {
                   //SNLOG((kError) << "NetClientExt Connect timeout";
                    client->Stop();
                    ret = false;
                    m_p_connection = NULL;
                    break;
                }
               //SNLOG((kInfo) << "NetClientExt Connect Success";
                ret = true;
                break;
            }
            else
            {
               //SNLOG((kError) << "NetClientExt Connect failed err:" << client->GetLastError();
                client->Stop();
                ret = false;
                break;
            }
        } while (0);

        if (!ret && NULL != client)
            m_temp_conn.add(client);

        return ret;
    }

    bool NetClientExt::disconnect()
    {
        bool rel = false;
        std::lock_guard<std::recursive_mutex> guard(m_lock);
       //SNLOG((kInfo) << "NetClientExt disconnect called";
        if (NULL != m_p_connection)
        {
            rel = m_p_connection->Stop() ? true : false;
            if (rel)
            {
                m_temp_conn.add(m_p_connection);
                m_connected = false;
                m_p_connection = NULL;
            }
        }
        return rel;
    }

    bool NetClientExt::register_message(uint32_t message_id, 
        const callback_func_t& callback, 
        google::protobuf::Message* factory)
    {
        if (m_parser.m_message_map.find(message_id) != m_parser.m_message_map.end())
            return false;

        m_parser.m_message_map[message_id] = make_pair(callback, factory);
        return true;
    }

    void NetClientExt::on_get_encry_key()
    {
        m_sequence = 1;
        m_connected = true;
        std::thread notify_thread(std::bind(&NetClientExt::connected_notify_proc, this));
        notify_thread.detach();
    }

    void NetClientExt::hearbeat_timer_proc(uint32_t id)
    {
        if (!fire_hearbeat())
        {
            if (m_connected)
            {
               //SNLOG((kError) << "Heartbeat error.";
                disconnect();
            }
        }
    }

    void NetClientExt::handle_packet_proc()
    {
        int pop_ret = 0;
        do 
        {
            uint32_t message_id;
            proto_msg_ptr message = nullptr;

            pop_ret = m_parser.pop_packet(message_id, message);

            if (-1 == pop_ret) 
            {
               //SNLOG((kTrace) << "pop_packet err";
                disconnect();
                break;
            }

            if (0 == message_id || 27 == message_id)
            {// 心跳和握手包 不处理
                continue;
            }

            if (nullptr == message)
            {
               //SNLOG((kError) << "message_id: " << message_id << " is nullptr";
                continue;
            }

            // 过滤包
            if (!on_filter(message_id, message))
            {
               //SNLOG((kTrace) << "message_id: " << message_id << " ignored";
                continue;
            }

            if (0 == message_id || 27 == message_id)
            {//心跳
                continue;
            }

            auto it = m_parser.m_message_map.find(message_id);
            if (it == m_parser.m_message_map.end())
            {
               //SNLOG((kTrace) << "cant found deal function message_id: " << message_id;
                continue;
            }

            if (it->second.first != nullptr)
            {
                it->second.first(message);
               //SNLOG((kTrace) << "message_id: " << message_id << " deal function called";
            }
            //else
               //SNLOG((kTrace) << "message_id: " << message_id << " deal function is nullptr";

        } while (pop_ret == 1);
    }

    bool NetClientExt::fire_hearbeat()
    {
        if (!m_connected)
            return false;

        bool rel = false;
        std::string send_data;
        m_parser.make_send_packet(net2::InternalHeartMsg, send_data);
        if (NULL != m_p_connection)
            rel = m_p_connection->Send(reinterpret_cast<const BYTE*>(send_data.c_str()), send_data.size()) ? true: false;

        return rel;
    }

    bool NetClientExt::is_connected()
    {
        if (NULL == m_p_connection)
            return false;

        return m_p_connection->GetState() == SS_STARTED;
    }

    void NetClientExt::clear_stop_client()
    {
        m_temp_conn.del_if([](snqu::SafeSet<IClient*>::value_type & piter)->bool
        {
            if (NULL != piter && piter->GetState() == SS_STOPPED)
            {
                Ye_Destroy_TcpClient((ITcpClient*)piter);
                piter = NULL;
                return true;
            }
            return false;
        });
    }

    void NetClientExt::connected_notify_proc()
    {
        m_reconn = false;
        std::unique_lock<std::mutex> lck(m_unique_lock);
        m_condition.notify_one();
        on_connected();
    }

    bool NetClientExt::send_data(
        IClient* pClient, 
        uint32_t message_id, 
        uint8* msg, 
        uint32 size)
    {
        _VMProtectBegin(__FUNCTION__);
        auto len = msg ? sizeof(SQHeader) + size + sizeof(uint32_t) : sizeof(SQHeader) + sizeof(uint32_t);
        char *buf = new char[len];

        auto header = reinterpret_cast<SQHeader*>(buf);

        header->signature = SIGNATURE;
        header->message_id = message_id;
        header->length = len;
        header->pb_length = 0;
        header->option = 0;
        header->reserved = 0;

        if (msg)
            memcpy(&header->body, msg, size);

        // TODO: calc checksum
        uint32_t check_sum = 
            SQEncryptHelper::checksum((char *)&header->body, len - sizeof(SQHeader) - sizeof(uint32_t));
        *reinterpret_cast<uint32_t*>(((char*)header) + header->length - sizeof(uint32_t)) = check_sum;

        bool rel = false;
        rel = pClient->Send(reinterpret_cast<const BYTE*>(buf), len) ? true: false;
        SAFE_DELETE(buf);

        _VMProtectEnd();
        return rel;
    }

    void NetClientExt::get_encry_key(IClient* pclient)
    {
        _VMProtectBegin(__FUNCTION__);
        send_data(pclient, net2::InternalQueryKeyMsg, nullptr, 0);
        _VMProtectEnd();
    }

    void NetClientExt::reconnect()
    {
        if (m_reconn)
            return;

       //SNLOG((kInfo) << "NetClientExt reconnect called";
        m_task_thread.SetTimer(10, m_reconnect_time * 1000, [&](uint32_t timer_id)
        {
            if (!m_started)
                return;

            if (m_connected)
            {
                m_task_thread.KillTimer(10); // 连接成功后删除重连
                return;
            }

            if (NULL != m_p_connection && m_p_connection->GetState() <= SS_STOPPED)
            {
                m_p_connection->Stop();
                m_p_connection = NULL;
            }

            clear_stop_client();
            bool ret = connect(false);
            //SNLOG((kInfo) << "try reconnect result:" << ret;
            if (ret)
                m_task_thread.KillTimer(10); // 连接成功后删除重连
        });
        m_reconn = true;
    }
}}