#pragma once
#include <sqstd/sqinc.h>
#include <google/protobuf/message.h>
#include <memory>
#include <sqstd/sqtaskthread.h>
#include <sqnet/sqnethelper2.h>
#include <HPSocket/bufferptr.h>
#include <HPSocket/HPSocket.h>
#include <HPSocket/SocketInterface.h>
#include <sqnet/sqnet2.h>
#include <sqstd/sqtimerthread.h>
#include <sqstd/sqvecbuffhelper.h>
#include <sqnet/sqcrypthelper.h>
#include <sqsafe/sqsafemodel.h>
#include <vmp/sqvmsdk.h>

using namespace snqu::safe;
namespace snqu{ namespace net2 {
	//typedef std::shared_ptr<google::protobuf::Message> proto_msg_ptr;
	class NetClientWrapperImpl;
	class NetClientWrapper
	{
	public:
		NetClientWrapper();
		virtual ~NetClientWrapper();

	protected:
		bool send_data(
			CONNID connid, 
			uint32_t message_id, 
			google::protobuf::Message *msg, 
			const string& session_key,
			bool encrypt);

		bool send_raw_data(
			CONNID connid, 
			uint32_t message_id, 
			const string& xdata, 
			const string& session_key,
			bool encrypt);

		bool send_data2(
			CONNID sock, 
			uint32_t message_id, 
			uint8* msg, 
			uint32 size);

		string decrypt_packet(
			CONNID connid, 
			uint64_t ulpkey, 
			uint8* data, 
			uint32 size,
			const string& session_key);

		bool encrypt_session_key(
			CONNID connid, 
			uint8* buf, 
			uint32 size,
			std::function<void(const string& session_key)> func);

		void get_session_key(CONNID connid);

		EnHandleResult receive_packet(
			IClient* pClient, 
			const BYTE* pData, 
			int iLength,
			std::function<void(CONNID connid, uint8* buf, uint32 size)> save_key_func,
			std::function<string(CONNID connid, uint64_t ulpkey, uint8* data, uint32 size)> decrypt_func,
			std::function<void(IClient* client, uint32 message_id, google::protobuf::Message* Message)> add_packet_func,
			std::function<google::protobuf::Message*(uint32 message_id)> get_msg_func
			);

		bool send_hearbeat(CONNID connid);

	protected:
		NetClientWrapperImpl *m_wrapperImpl;
		
	};

	//////////////////////////////////////////////////////////////////////////
	class NetClientCallback : public CTcpClientListener
	{
	public:
		inline NetClientCallback(
			std::function<EnHandleResult(IClient* pClient, const BYTE* pData, int iLength)> onSend,
			std::function<EnHandleResult(IClient* pClient, const BYTE* pData, int iLength)> onReceive,
			std::function<EnHandleResult(IClient* pClient)> onClose,
			std::function<EnHandleResult(IClient* pClient, EnSocketOperation enOperation, int iErrorCode)> onError,
			std::function<EnHandleResult(IClient* pClient)> onConnect)
			: m_onSend(onSend)
			, m_onReceive(onReceive)
			, m_onClose(onClose)
			, m_onError(onError)
			, m_onConnect(onConnect)
		{}

		inline virtual ~NetClientCallback() {};
		inline virtual EnHandleResult OnSend(IClient* pClient, const BYTE* pData, int iLength) override
		{
			return m_onSend(pClient, pData, iLength);
		}

		inline virtual EnHandleResult OnReceive(IClient* pClient, const BYTE* pData, int iLength) override
		{
			return m_onReceive(pClient, pData, iLength);
		}

		inline virtual EnHandleResult OnClose(IClient* pClient) override
		{
			return m_onClose(pClient);
		}

		inline virtual EnHandleResult OnError(IClient* pClient, EnSocketOperation enOperation, int iErrorCode) override
		{
			return m_onError(pClient, enOperation, iErrorCode);
		}

		inline virtual EnHandleResult OnConnect(IClient* pClient) override
		{
			return m_onConnect(pClient);
		}

	private:
		std::function<EnHandleResult(IClient* pClient, const BYTE* pData, int iLength)> m_onSend;
		std::function<EnHandleResult(IClient* pClient, const BYTE* pData, int iLength)> m_onReceive;
		std::function<EnHandleResult(IClient* pClient)> m_onClose;
		std::function<EnHandleResult(IClient* pClient, EnSocketOperation enOperation, int iErrorCode)> m_onError;
		std::function<EnHandleResult(IClient* pClient)> m_onConnect;
	};

	//////////////////////////////////////////////////////////////////////////
	template <typename session>
	class NetClient : public NetClientWrapper
	{
	public:
		NetClient() : m_started(false)
			, m_connected(false)
            , m_netCallback(nullptr)
		{
			
		}

		typedef snqu::TaskThread<std::tuple<IClient*, uint32_t, google::protobuf::Message*> > task_thread_t;
		typedef std::shared_ptr<session> session_ptr;
		typedef function<void (session_ptr, google::protobuf::Message*)> callback_func_t;

	public:
		bool start()
		{
			MDLOG(kTrace, "NetClient initing.");
            if (nullptr == m_netCallback)
            {
                m_netCallback = new NetClientCallback(
                    std::bind(&NetClient<session>::OnSend, this, std::tr1::placeholders::_1, std::tr1::placeholders::_2, std::tr1::placeholders::_3),
                    std::bind(&NetClient<session>::OnReceive, this, std::tr1::placeholders::_1, std::tr1::placeholders::_2, std::tr1::placeholders::_3),
                    std::bind(&NetClient<session>::OnClose, this, std::tr1::placeholders::_1),
                    std::bind(&NetClient<session>::OnError, this, std::tr1::placeholders::_1, std::tr1::placeholders::_2, std::tr1::placeholders::_3),
                    std::bind(&NetClient<session>::OnConnect, this, std::tr1::placeholders::_1)
                    );
            }

			if (m_started)
				return true;

			m_task_thread.begin(std::bind(&NetClient<session>::detect_packet_proc, this, std::placeholders::_1));

			m_timer.begin();
			m_timer.SetTimer(9, HeartbeatDetectionSeconds*1000, 
				std::bind(&NetClient<session>::hearbeat_timer_proc, this, std::placeholders::_1));

			m_started = true;
			return true;
		}

		void stop()
		{
			if (!m_started)
				return;

			m_timer.end();
			m_task_thread.end();
			auto temp = m_linker_map;
			for (auto item = temp.begin(); item != temp.end(); )
			{
				if (item->second != nullptr)
					reinterpret_cast<IClient*>(item->first)->Stop();

				temp.erase(item++);
			}

			release_message_map();
			m_started = false;
			m_connected = false;
		}

		CONNID connect(const std::string& ip, unsigned short port, bool async = false)
		{
			ITcpClient* client = Ye_Create_TcpClient(reinterpret_cast<ITcpClientListener *>(m_netCallback));
			if (nullptr == client)
				return 0;
			
			if (client->Start(ip.c_str(), port, async ? TRUE : FALSE))
			{
				return reinterpret_cast<CONNID>(client);
			}
			else 
				Ye_Destroy_TcpClient(client);

			return 0;
		}

		bool disconnect(CONNID connid)
		{
			bool rel = false;
			std::lock_guard<std::recursive_mutex> guard(m_lock);
			auto item = m_linker_map.find(connid);
			if (item != m_linker_map.end())
			{
				ITcpClient *pclient = reinterpret_cast<ITcpClient*>(connid);
				rel = pclient->Stop() ? true : false;
				if (rel)
				{
					Ye_Destroy_TcpClient(pclient);
					m_linker_map.erase(item);
				}
			}

			return rel;
		}

		bool register_message(uint32_t message_id, 
			const callback_func_t& callback, 
			google::protobuf::Message* factory = nullptr)
		{
			if (m_message_map.find(message_id) != m_message_map.end())
				return false;

			m_message_map[message_id] = make_pair(callback, factory);
			return true;
		}

	protected:
		virtual void on_connected(session_ptr) = 0;
		virtual void on_close(session_ptr) = 0;
		virtual void on_error(session_ptr) = 0;
		virtual bool on_filter(session_ptr, uint32_t, google::protobuf::Message*) { return true; }

	private:
		void add_user_packet(IClient* client, uint32 message_id, google::protobuf::Message* Message)
		{
			m_task_thread.post_task(std::make_tuple(client, message_id, Message));
		}

		void detect_packet_proc(const std::tuple<IClient*, uint32_t, google::protobuf::Message*>& task)
		{
			auto p_session = find_session(reinterpret_cast<CONNID>(std::get<0>(task)));
			if (nullptr != p_session)
			{
				// ¹ýÂË°ü1
				if (!on_filter(p_session, std::get<1>(task), std::get<2>(task)))
				{
					google::protobuf::Message* filter_msg = std::get<2>(task);
					if (filter_msg != nullptr)
						delete filter_msg;
					return;
				}

				auto it = m_message_map.find(std::get<1>(task));
				if (it != m_message_map.end())
				{
					if (it->second.first != nullptr)
						it->second.first(p_session, std::get<2>(task));
				}
				else
				{
					//SNLOG(kTrace, __FUNCTION__ << ", cant found message_id: " << std::get<1>(task);
				}
			}
			else
			{
				//SNLOG(kFatal, << __FUNCTION__ << ", find_session error, message_id: " << std::get<1>(task);
			}

			delete std::get<2>(task);
		}

		void hearbeat_timer_proc(uint32_t id)
		{
			for (auto& item : m_linker_map)
			{
				if (item.second != nullptr)
				{
					if (!fire_hearbeat(item.first))
					{
						if (m_connected)
						{
						    //SNLOG(kFatal) << "Hearbeat error.";
							m_connected = false;
							OnClose(reinterpret_cast<IClient*>(item.first));
						}
					}
				}
			}
		}

		bool fire_hearbeat(CONNID connid)
		{
			if (!m_connected)
				return true;

			return NetClientWrapper::send_hearbeat(connid);
		}

		void connected_notify_proc(CONNID connid)
		{
			auto iter = m_linker_map.find(connid);
			if (iter != m_linker_map.end())
				on_connected(iter->second);

			fire_hearbeat(connid);
		}

		void release_message_map()
		{
			for (auto& item : m_message_map)
			{
				SAFE_DELETE(item.second.second);
			}

			m_message_map.clear();
		}

	private:
		EnHandleResult OnSend(IClient* pClient, const BYTE* pData, int iLength)
		{
			return HR_OK;
		}

		EnHandleResult OnReceive(IClient* pClient, const BYTE* pData, int iLength)
		{
			return NetClientWrapper::receive_packet(
				pClient,
				pData,
				iLength,
				std::bind(&NetClient<session>::on_save_session_key, this, std::tr1::placeholders::_1, std::tr1::placeholders::_2, std::tr1::placeholders::_3),
				std::bind(&NetClient<session>::decrypt_packet, this, std::tr1::placeholders::_1, std::tr1::placeholders::_2, std::tr1::placeholders::_3, std::tr1::placeholders::_4),
				std::bind(&NetClient<session>::add_user_packet, this, std::tr1::placeholders::_1, std::tr1::placeholders::_2, std::tr1::placeholders::_3), 
				std::bind(&NetClient<session>::get_message_factory, this, std::tr1::placeholders::_1)
				);
		}

		EnHandleResult OnClose(IClient* pClient)
		{
			session_ptr linker_ptr;
			{
				std::lock_guard<std::recursive_mutex> guard(m_lock);
				auto iter = m_linker_map.find(reinterpret_cast<CONNID>(pClient));
				if (iter != m_linker_map.end())
				{
					linker_ptr = iter->second;
					m_linker_map.erase(iter);
				}
			}
           //SNLOG((kFatal) << "Network OnClose called: " << reinterpret_cast<CONNID>(pClient);
			m_connected = false;
			on_close(linker_ptr);
			return HR_OK;
		}

		EnHandleResult OnError(IClient* pClient, EnSocketOperation enOperation, int iErrorCode)
		{
			m_connected = false;
			if (SO_CONNECT == enOperation)
			{
				return HR_ERROR;
			}
			else
			{
			    //SNLOG(kFatal) << "Network error, code: " << iErrorCode << ", oper: " << enOperation;
				OnClose(pClient);
			}
			return HR_OK;
		}

		EnHandleResult OnConnect(IClient* pClient)
		{
			std::string ip;
			CHAR szAddress[40];
			int iAddressLen = sizeof(szAddress) / sizeof(CHAR);
			USHORT usPort = 0;

			pClient->GetRemoteAddress(szAddress, iAddressLen, usPort);
			session_ptr linker_ptr = std::make_shared<session>();
			if (linker_ptr != nullptr)
			{
				string ip = szAddress;
				linker_ptr->set_handle(reinterpret_cast<CONNID>(pClient));
				linker_ptr->set_remote_ip(ip);
				linker_ptr->bind_send_func(std::bind(&NetClient<session>::send_data, this, 
					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
				
				linker_ptr->bind_send_raw_func(std::bind(&NetClient<session>::send_raw_data, this, 
					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

				std::lock_guard<std::recursive_mutex> guard(m_lock);
				m_linker_map.insert(std::make_pair(reinterpret_cast<CONNID>(pClient), linker_ptr));

				get_session_key(reinterpret_cast<CONNID>(pClient));
			}

			return HR_OK;
		}

		TPkgInfo* find_header(CONNID dwConnID)
		{
			auto iter = m_linker_map.find(connid);
			if (iter != m_linker_map.end()) 
				return iter->second->get_pkg_info();

			return nullptr;
		}

		bool send_data(CONNID connid, uint32_t message_id, google::protobuf::Message *msg, bool encrypt)
		{
			bool rel = false;
			auto p_session = find_session(connid);
			if (p_session != nullptr)
			{
				rel = NetClientWrapper::send_data(connid, message_id, msg, p_session->get_session_key(), encrypt);
			}

			return rel;
		}

		bool send_raw_data(CONNID connid, uint32_t message_id, const string& msg, bool encrypt)
		{
			bool rel = false;
			auto p_session = find_session(connid);
			if (p_session != nullptr)
			{
				rel = NetClientWrapper::send_raw_data(connid, message_id, msg, p_session->get_session_key(), encrypt);
			}

			return rel;
		}

		session_ptr find_session(CONNID connid)
		{
			session_ptr linker_ptr;
			{
				std::lock_guard<std::recursive_mutex> guard(m_lock);
				auto item = m_linker_map.find(connid);
				if (item != m_linker_map.end())
				{
					linker_ptr = item->second;
				}
			}

			return linker_ptr;
		}

		void on_save_session_key(CONNID connid, uint8* buf, uint32 size)
		{
			if (size != AES_KEY_LEN*2)
				return;

			auto iter = m_linker_map.find(connid);
			if (iter != m_linker_map.end()) 
			{
				NetClientWrapper::encrypt_session_key(connid, buf, size, [&](const string& session_key)->void {
					iter->second->set_session_key(session_key);
				});

				m_connected = true;
				std::thread notify_thread(std::bind(&NetClient<session>::connected_notify_proc, 
					this, std::placeholders::_1), connid);
				notify_thread.detach();
			}
			else
			{
				//SNLOG(kFatal, << "CAN`t found session!";
			}
		}
		
		string decrypt_packet(CONNID connid, uint64_t ulpkey, uint8* data, uint32 size)
		{
			auto p_session = find_session(connid);
			if (nullptr == p_session)
			{
				return "";
			}

			return NetClientWrapper::decrypt_packet(connid, ulpkey, data, size, p_session->get_session_key());
		}

		bool send_data2(CONNID sock, uint32_t message_id, uint8* msg, uint32 size)
		{
			return NetClientWrapper::send_data2(sock, message_id, msg, size);
		}

		google::protobuf::Message* get_message_factory(uint32_t message_id)
		{
			auto pos = m_message_map.find(message_id);
			if (pos != m_message_map.end())
				return pos->second.second;

			return nullptr;
		}

	private:
		typedef std::map<CONNID, session_ptr> linker_map_t;
		linker_map_t m_linker_map;
		
		bool m_started;
		bool m_connected;

		typedef unordered_map<uint32_t, pair<callback_func_t, google::protobuf::Message*> > message_map_t;
		message_map_t m_message_map;
		NetClientCallback *m_netCallback;

		std::recursive_mutex m_lock;
		task_thread_t m_task_thread;
		TimerThread m_timer;
	};
}}