#include <sqnet/sqnet2.h>
#include <sqnet/sqnetclientbot.h>
#include <sqstd/sqinc.h>
#include <sqlog/sqlog.h>


using namespace snqu::safe;
namespace snqu { namespace net {

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
            UninitLogger();
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
			SNLOG(kTrace, "NetClientExt OnClose called");
            if (NULL != m_p_connection && m_p_connection->GetConnectionID() == pClient->GetConnectionID())
            {
                m_p_connection = NULL;
            }
            m_sequence = 0;
            m_parser.clear_encry_key();
            on_close();
            return HR_OK;
        }

        EnHandleResult NetClientEx::OnError(IClient* pClient, EnSocketOperation enOperation, int iErrorCode)
        {
            m_connected = false;
            if (SO_CONNECT == enOperation)
            {}
            else
            {
				SNLOG(kError, "Network OnError, code: %d, open: %d", iErrorCode, enOperation);
                disconnect();
                OnClose(pClient);
            }
            return HR_ERROR;
        }

        EnHandleResult NetClientEx::OnConnect(IClient* pClient)
        {
            {
                m_reconn = false;
                m_p_connection = pClient;
                m_connected = true;
            }
            
            on_connected();
            return HR_OK;
        }

        bool NetClientEx::start(int reconn_time, int send_wait_time)
        {
            static bool called = false;
            if (!called)
            {
                called = true;
                LogInitParam param;
                InitLogger(param);
				SNLOG(kTrace, "NetClientEx constructed.");
            }

            m_send_wait_second = send_wait_time;
            m_reconnect_time = reconn_time;

            if (m_started)
                return true;

            m_task_thread.begin();
            m_task_thread.SetTimer(9, net2::HeartbeatDetectionSeconds * 1000, std::bind(&NetClientEx::hearbeat_timer_proc, this, std::placeholders::_1));
            m_started = true;
            return true;
        }

        void NetClientEx::stop()
        {
            if (!m_started)
                return;

			SNLOG(kInfo, "NetClientEx stop called");
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

        void NetClientEx::set_session_data(const std::string& session_id, const std::string& ip_addr, const std::string& domain)
        {
            m_remote_ip = ip_addr;
            m_domain = domain;
        }

        bool NetClientEx::connect(unsigned short port, bool async, bool is_auto_reconn)
        {
            if (m_reconn)
            {
                if (is_auto_reconn == false)
                {
                    m_reconn = false;
					SNLOG(kInfo, "connect called, stop reconn");
                    m_task_thread.KillTimer(10);
                    return connect(async);
                }
                else
                {
                    return false;
                }
            }

            m_remote_port = port;

            bool ret = connect(async);

            if (is_auto_reconn && !ret)
                reconnect();

			SNLOG(kInfo, "NetClientEx Connect Failed");
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
					SNLOG(kError, "connect failed gethostbyname err:%d  host:%s", GetLastError(), m_domain.c_str());
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
						SNLOG(kError, "Ye_Create_TcpClient failed");
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
						SNLOG(kWarning, "m_p_connection status is: %d", m_p_connection->GetState());
                    }
                }

                if (client->Start(m_remote_ip.c_str(), m_remote_port, async ? TRUE : FALSE))
                {
					SNLOG(kInfo, "NetClientExt Connect Success");
                    ret = true;
                    break;
                }
                else
                {
                    SNLOG(kError, "NetClientExt Connect failed err:%d msg:%s sys_err:%d", 
						client->GetLastError(), client->GetLastErrorDesc(), GetLastError());
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
            SNLOG(kInfo, "NetClientEx disconnect called");
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

        bool NetClientEx::register_message(uint32_t message_id, const callback_func_center_t& callback)
        {
            if (m_parser.m_message_map.find(message_id) != m_parser.m_message_map.end())
                return false;

            m_parser.m_message_map[message_id] = callback;
            return true;
        }

        void NetClientEx::hearbeat_timer_proc(uint32_t id)
        {
            if (!fire_hearbeat())
            {
                if (m_connected)
                {
					SNLOG(kError, "Heartbeat error.");
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
                std::string message;

                pop_ret = m_parser.pop_packet(message_id, message);

                if (-1 == pop_ret)
                {
					SNLOG(kTrace, "pop_packet err");
                    disconnect();
                    break;
                }

                if (0 == message_id || 27 == message_id)
                {// 心跳和握手包 不处理
                    continue;
                }

                if (message.empty())
                {
					SNLOG(kError, "message_id: %d is nullptr", message_id);
                    continue;
                }

                // 过滤包
                if (!on_filter(message_id, message))
                {
					SNLOG(kTrace, "message_id: %d ignored", message_id);
                    continue;
                }

                if (0 == message_id || 27 == message_id)
                {//心跳
                    continue;
                }

                auto it = m_parser.m_message_map.find(message_id);
                if (it == m_parser.m_message_map.end())
                {
                    SQLOG(kTrace, "cant found deal function message_id:{0}", message_id);
                    continue;
                }

                if (it->second != nullptr)
                {
                    it->second(message_id, message);
                    SQLOG(kTrace, "message_id:{0} deal function called", message_id);
                }
                else
                SQLOG(kTrace, "message_id: {0} deal function is nullptr", message_id);

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
                rel = m_p_connection->Send(reinterpret_cast<const BYTE*>(send_data.c_str()), send_data.size()) ? true : false;

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

        void NetClientEx::reconnect()
        {
            if (m_reconn)
                return;

            SQLOG(kInfo, "NetClientEx reconnect called");
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
                SQLOG(kInfo, "try reconnect result:{0}, remote_ip:{1}", ret, m_remote_ip);
                if (ret)
                {
                    SQLOG(kInfo, "reconnect success, stop reconn");
                    m_task_thread.KillTimer(10); // 连接成功后删除重连
                }
            });
            m_reconn = true;
        }

        bool NetClientEx::send_msg(uint32_t message_id, const std::string& message)
        {
            if (!m_connected)
                return false;

            bool rel = false;
            if (m_p_connection != NULL)
            {
                std::string send_data = message;
                m_parser.make_send_packet(message_id, send_data);
                if (!send_data.empty())
                {
                    rel = m_p_connection->Send(reinterpret_cast<const BYTE*>(send_data.c_str()), send_data.size()) ? true : false;
                    if (rel)
                    {
                        m_sequence++;
                    }
                }
            }

            return rel;
        }
    }
}