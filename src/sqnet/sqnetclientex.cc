#include <sqnet/sqnet2.h>
#include <sqnet/sqnetclientex.h>
#include <sqstd/sqinc.h>
#include <sqlog/sqlog.h>


using namespace snqu::safe;
namespace snqu{ namespace net2 {

    NetClientEx::NetClientEx(uint32_t signature /* = SIGNATURE */)
        : m_started(false)
        , m_p_connection(NULL)
		, m_parser(signature)
    {
        m_sequence = 0;
        m_remote_port = 0;
        m_send_wait_second = 5;
        m_reconnect_time = 15; // second
        m_reconn = false;
        m_connected = false;
    }

    NetClientEx::~NetClientEx()
    {
        SAFE_DELETE(m_p_connection);
    }

    EnHandleResult NetClientEx::OnSend(IClient* pClient, const BYTE* pData, int iLength)
    {
        return HR_OK;
    }

    EnHandleResult NetClientEx::OnReceive(IClient* pClient, const BYTE* pData, int iLength)
    {
        m_parser.add_buffer((char*)pData, iLength);
        handle_packet_proc();
        return HR_OK;
    }

    EnHandleResult NetClientEx::OnClose(IClient* pClient)
    {
		m_connected = false;
        //SLOG(kTrace) << "NetClientExt OnClose called";
        if (NULL != m_p_connection && m_p_connection->GetConnectionID() == pClient->GetConnectionID())
        {
            m_p_connection = NULL;
        }
        m_sequence = 0;
        m_remote_ip = "";
        m_parser.clear_encry_key();
        on_close();
        return HR_OK;
    }

    EnHandleResult NetClientEx::OnError(IClient* pClient, EnSocketOperation enOperation, int iErrorCode)
    {
        m_connected = false;
        if (SO_CONNECT == enOperation)
        {
            return HR_ERROR;
        }
        else
        {
            //SLOG(kError) << "Network OnError, code: " << iErrorCode << ", oper: " << enOperation;
            disconnect();
            OnClose(pClient);
        }
        return HR_ERROR;
    }

    EnHandleResult NetClientEx::OnConnect(IClient* pClient)
    {
        m_p_connection = pClient;
        //SLOG(kTrace) << "NetClientEx OnConnect called ";
        // 连接第一个包要去获取通讯加密的密钥
        get_encry_key(pClient);
        return HR_OK;
    }

    bool NetClientEx::start(int reconn_time, int send_wait_time)
    {
        static bool called = false;
        if (!called)
        {
            called = true;
            //SLOG(kTrace) << "NetClientEx constructed.";
        }

        m_send_wait_second = send_wait_time;
        m_reconnect_time = reconn_time;

        if (m_started)
            return true;

        m_task_thread.begin();
        m_task_thread.SetTimer(9, HeartbeatDetectionSeconds * 1000, std::bind(&NetClientEx::hearbeat_timer_proc, this, std::placeholders::_1));
        m_started = true;
        return true;
    }

    void NetClientEx::stop()
    {
        if (!m_started)
            return;

        //SLOG(kInfo) << "NetClientEx stop called";
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

		m_parser.clear_encry_key();
    }

    void NetClientEx::set_session_data(const std::string& site_id, const std::string& session_id, const std::string& domain, bool is_v6_ver)
    {
        m_parser.m_site_id = site_id;
        m_parser.is_v6_ver = is_v6_ver;
        m_session_id = session_id;
        m_domain = domain;
    }

    bool NetClientEx::connect(unsigned short port, bool async, bool is_auto_reconn)
    {
        if (m_reconn)
        {
            if (is_auto_reconn == false)
            {
                m_reconn = false;
                //SLOG(kInfo) << "connect called, stop reconn";
                m_task_thread.KillTimer(10);
                return connect(async);
            }
            else
            {
                return false;
            }
        }

        m_parser.m_on_get_seesion_cb = std::bind(&NetClientEx::on_get_encry_key, this);

        m_remote_port = port;

        bool ret= connect(async);

        if (is_auto_reconn && !ret)
            reconnect();

        //SLOG(kInfo) << "NetClientEx Connect Failed";
        return ret;
    }

    bool NetClientEx::connect(bool async)
    {
        if (m_connected) return true;

        if (m_remote_ip.empty())
        {// 没有IP地址的时候解析DNS
            auto pHE = ::gethostbyname(m_domain.c_str());
            if (NULL != pHE)
            {
                m_remote_ip = fmt::Format("{0}.{1}.{2}.{3}", pHE->h_addr_list[0][0] & 0x00ff, pHE->h_addr_list[0][1] & 0x00ff, 
                    pHE->h_addr_list[0][2] & 0x00ff, pHE->h_addr_list[0][3] & 0x00ff);
            }
            else
            {
                SNLOG(snqu::kError, "connect failed gethostbyname err:%d  host:%s", GetLastError(), m_domain.c_str());
                return false;
            }
        }

        IClient* client = NULL;
        bool ret = false;
        do 
        {
            if (nullptr == m_p_connection)
            {
                client = Ye_Create_TcpClient(reinterpret_cast<ITcpClientListener *>(this));
                if (client == nullptr)
                {
					SNLOG(snqu::kError, "Ye_Create_TcpClient failed err:%d", SYS_WSAGetLastError());
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
                    //SLOG(kWarning) << "m_p_connection status is:" << m_p_connection->GetState();
                }
            }
			
            //SLOG(kInfo) << "NetClientExt Connect Start session_id:" << m_session_id;
            if (client->Start(m_remote_ip.c_str(), m_remote_port, async ? TRUE : FALSE))
            {
                std::unique_lock<std::mutex> lck(m_unique_lock);
                if (std::cv_status::timeout == m_condition.wait_for(lck, std::chrono::seconds(m_send_wait_second)))
                {
					SNLOG(snqu::kError, "NetClientExt Connect timeout err:%d", SYS_WSAGetLastError());
                    client->Stop();
                    ret = false;
                    m_p_connection = NULL;
                    break;
                }
                //SLOG(kInfo) << "NetClientExt Connect Success";
                ret = true;
                break;
            }
            else
            {
				SNLOG(snqu::kError, "NetClientExt Connect failed err:%d", client->GetLastError());
                client->Stop();
                ret = false;
                break;
            }
        } while (0);

        if (!ret && NULL != client)
            m_temp_conn.add(client);

        return ret;
    }

    bool NetClientEx::disconnect()
    {
        bool rel = false;
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        //SLOG(kInfo) << "NetClientEx disconnect called";
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

    bool NetClientEx::register_message(uint32_t message_id, 
        const callback_func_center_t& callback, 
        google::protobuf::Message* factory)
    {
        if (m_parser.m_message_map.find(message_id) != m_parser.m_message_map.end())
            return false;

        m_parser.m_message_map[message_id] = make_pair(callback, factory);
        return true;
    }

    void NetClientEx::on_get_encry_key()
    {
        m_sequence  = 1;
        m_connected = true;
        std::thread notify_thread(std::bind(&NetClientEx::connected_notify_proc, this));
        notify_thread.detach();
    }

    void NetClientEx::hearbeat_timer_proc(uint32_t id)
    {
        if (!fire_hearbeat())
        {
            if (m_connected)
            {
                //SLOG(kError) << "Heartbeat error.";
                disconnect();
            }
        }
    }

    void NetClientEx::handle_packet_proc()
    {
        int pop_ret = 0;
        do 
        {
            uint32_t message_id;
            proto_msg_ptr message = nullptr;

            pop_ret = m_parser.pop_packet(message_id, message);

            if (-1 == pop_ret) 
            {
                //SLOG(kTrace) << "pop_packet err";
                disconnect();
                break;
            }

            if (0 == message_id || 27 == message_id)
            {// 心跳和握手包 不处理
                continue;
            }

            if (nullptr == message)
            {
                //SLOG(kError) << "message_id: " << message_id << " is nullptr";
                continue;
            }

            // 过滤包
            if (!on_filter(message_id, message))
            {
                //SLOG(kTrace) << "message_id: " << message_id << " ignored";
                continue;
            }

            if (0 == message_id || 27 == message_id)
            {//心跳
                continue;
            }

            auto it = m_parser.m_message_map.find(message_id);
            if (it == m_parser.m_message_map.end())
            {
                //SLOG(kTrace) << "cant found deal function message_id: " << message_id;
                continue;
            }

            if (it->second.first != nullptr)
            {
                it->second.first(message_id, message);
                //SLOG(kTrace) << "message_id: " << message_id << " deal function called";
            }
            //else
                //SLOG(kTrace) << "message_id: " << message_id << " deal function is nullptr";

        } while (pop_ret == 1);
    }

    bool NetClientEx::fire_hearbeat()
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

    bool NetClientEx::is_connected()
    {
        if (NULL == m_p_connection)
            return false;

        return m_p_connection->GetState() == SS_STARTED;
    }
    
	void NetClientEx::clear_stop_client()
    {
        m_temp_conn.del_if([](snqu::SafeSet<IClient*>::value_type& piter)->bool
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

    void NetClientEx::connected_notify_proc()
    {
        m_reconn = false;
        std::unique_lock<std::mutex> lck(m_unique_lock);
        m_condition.notify_one();
        on_connected();
    }

    void NetClientEx::get_encry_key(IClient* pclient)
    {
        _VMProtectBegin(__FUNCTION__);
        std::string send_data;
        send_data.append("{\"session_id\": \"").append(m_session_id);
        send_data.append("\",\"site_id\": \"").append(m_parser.m_site_id);
        send_data.append("\",\"sequence\": \"0\"}");
        bool rel = false;
        m_parser.make_send_packet(net2::InternalQueryKeyMsg, send_data);
        rel = pclient->Send(reinterpret_cast<const BYTE*>(send_data.c_str()), send_data.size()) ? true: false;
        if (rel)
        {
            //SNLOG(kTrace, "get_encry_key send success. session_id:%s data:%s", m_session_id.c_str(), net::BinToHex(send_data).c_str());
        }
        else
        {
            //SLOG(kError) << "get_encry_key send failed.";
        }
        _VMProtectEnd();
    }

    void NetClientEx::reconnect()
    {
        if (m_reconn)
            return;

        //SLOG(kInfo) << "NetClientEx reconnect called";
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
            //SLOG(kInfo) << "try reconnect result:" << ret << " remote_ip:" << m_remote_ip;
            if (ret)
            {
                //SLOG(kInfo) << "reconnect success, stop reconn";
                stop_reconn(); // 连接成功后删除重连
            }
        });
        m_reconn = true;
    }

    void NetClientEx::stop_reconn()
    {
        m_task_thread.KillTimer(10); // 连接成功后删除重连
		m_reconn = false;
    }
}}