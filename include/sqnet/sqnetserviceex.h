#ifndef SNQU_NETSERVICEEX_H
#define SNQU_NETSERVICEEX_H

#include <sqnet/sqnetservice.h>
#include <sqnet/sqthreadinterface.h>
#include <sqnet/sqsession.h>

using namespace snqu::safe;
namespace snqu{ namespace net2{ 

typedef function<void (SessionPtr, proto_msg_ptr)> callback_func_t;
typedef std::tuple<SessionPtr, uint32, proto_msg_ptr, callback_func_t> NtSeTask;

template <typename session>
class NetServiceEx : public CTcpPullServerListener, public NetServiceWrapper
{
public:
	NetServiceEx(){}

public:
    typedef std::shared_ptr<session> sesstion_ptr;
    
public:
    // 启动监听器
    bool start(ThreadInterface<NtSeTask>* task_mgr_ptr, const std::string& ip, unsigned short port, const string& site_id, 
			   unsigned int threadcount)
    {
		SQLOG(kTrace, "NetServiceex initing.");
		bool rel = false;
        m_task_mgr_ptr = task_mgr_ptr;
        if (nullptr == m_task_mgr_ptr) return false;
		
		m_server_engine = Ye_Create_TcpPullServer(this);
		if (nullptr == m_server_engine)
			return rel;

		if (SS_STARTED == m_server_engine->GetState() || 
			SS_STARTING == m_server_engine->GetState())
			return true;

		char *ipaddr = NULL;
		if (!ip.empty())
			ipaddr = (char *)ip.c_str();

		m_server_engine->SetWorkerThreadCount(threadcount);
		m_server_engine->SetSendPolicy(SP_PACK);
		if (m_server_engine->Start(ipaddr, port))
			rel = true;
        
		m_server_engine->SetSocketBufferSize(50*1024);

		if (rel)
		{
			if (m_server_engine->GetState() == SS_STARTING || 
				m_server_engine->GetState() == SS_STARTED)
			{
				m_task_mgr_ptr->set_timer_task(port, HeartbeatDetectionSeconds, 
                    std::bind(&NetServiceEx<session>::keepalive_timer_proc, this, std::placeholders::_1));
				m_site_id = site_id;
			}
		}

		return rel;
    }

    // 停止监听器
    void stop()
    {
		if (nullptr == m_server_engine)
			return;

		if (SS_STARTING == m_server_engine->GetState() ||
			SS_STARTED == m_server_engine->GetState())
		{
			m_server_engine->Stop();
			Ye_Destroy_TcpPullServer(m_server_engine);
			m_server_engine = nullptr;
			release_message_map();
		}
    }

	void release_message_map()
	{
		for (auto& item : m_message_map)
		{
			SAFE_DELETE(item.second.second);
		}

		m_message_map.clear();
	}

    // 关闭连接
    void close_handle(CONNID dwConnID)
    {
		//SLOG(kTrace) << "主动关闭sock句柄: " << dwConnID;
        if (SQ_INVALID_SOCKET != dwConnID)
        {
			m_server_engine->Disconnect(dwConnID);
        }
    }

    // 根据句柄查找连接者
    const sesstion_ptr find_session(CONNID dwConnID)
    {
        sesstion_ptr linker_ptr;
        {
            std::lock_guard<std::recursive_mutex> guard(m_lock);
            auto item = m_linker_map.find(dwConnID);
            if (item != m_linker_map.end())
            {
                linker_ptr = item->second;
            }
        }

        return linker_ptr;
    }

    // 根据方法op查找连接者
    template <typename OP>
    const sesstion_ptr find_session(const OP& op)
    {
        sesstion_ptr linker_ptr = nullptr;
        {
            std::lock_guard<std::recursive_mutex> guard(m_lock);
            for (auto& item : m_linker_map)
            {
                if (op(item.second))
                {
                    linker_ptr = item.second;
                    break;
                }
            }
        }

        return linker_ptr;
    }

    void get_all_linker(std::vector<sesstion_ptr>& linker_vec)
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        for (auto& item : m_linker_map)
        {
            linker_vec.push_back(item.second);
        }
    }

    // 遍历每个连接执行一次op
    template <typename OP>
    void for_each_linker(const OP& op)
    {
        std::vector<sesstion_ptr> linker_vec;
        get_all_linker(linker_vec);
        std::for_each(linker_vec.begin(), linker_vec.end(), op);
    }

    // 校验连接是否存在长期未连接的
//     void check_linker(unsigned int expire_time, const std::function<void(unsigned int)>& expire_callback)
//     {
//         std::lock_guard<std::recursive_mutex> guard(m_lock);
//         for (auto& it : expire_handles)
//         {
//             if (nullptr != expire_callback)
//             {
//                 expire_callback(*it);
//             }
//             close_handle(*it);
//         }
//     }

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
    virtual void on_accept(sesstion_ptr) = 0;
    virtual void on_close(sesstion_ptr) = 0;
    virtual void on_error(sesstion_ptr) = 0;
    virtual bool on_filter(sesstion_ptr, uint32_t, proto_msg_ptr) { return true; }
    virtual void on_connect(sesstion_ptr){};

private:
	void add_user_packet(CONNID dwConnID, uint32 message_id, proto_msg_ptr Message)
	{
        auto p_session = find_session(dwConnID);
        if (nullptr != p_session)
        {
            // 过滤包
            if (!on_filter(p_session, message_id, Message))
                return;

            auto it = m_message_map.find(message_id);
            if (it != m_message_map.end())
            {
                if (it->second.first != nullptr)
                {
                    m_task_mgr_ptr->post_task(std::make_tuple(p_session, message_id, Message, it->second.first));
                }
            }
        }
	}

	void keepalive_timer_proc(uint32_t id)
	{
#ifndef _DEBUG
		time_t now = time(NULL);
		std::lock_guard<std::recursive_mutex> guard(m_lock);

        for (auto& item : m_linker_map)
        {
            if ((now - item.second->get_expire_time()) <= HeartbeatDurationSeconds)
                continue;

			if (m_server_engine != nullptr)
			{
				SQLOG(kTrace, "**expire_time time out, disconnect %s", item.second->get_remote_ip().c_str());
				m_server_engine->Disconnect(item.first);
			}
        }
#endif
	}

	void on_heart_beat(CONNID dwConnID)
	{
		std::lock_guard<std::recursive_mutex> guard(m_lock);
		auto iter = m_linker_map.find(dwConnID);
		if (iter != m_linker_map.end())
		{
			iter->second->set_expire_time(time(NULL));
		}
	}

	void on_query_internal_key(CONNID connid)
	{
		NetServiceWrapper::query_internal_key(connid, 
			std::bind(&NetServiceEx<session>::save_session_key, this, std::placeholders::_1, std::placeholders::_2), 
            std::bind(&NetServiceEx<session>::fire_connected, this, std::placeholders::_1));
	}

    void fire_connected(CONNID connid)
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        auto item = m_linker_map.find(connid);
        if (item != m_linker_map.end())
        {
            on_connect(item->second);
        }
    }

	void save_session_key(CONNID connid, const string& local_key)
	{
		std::lock_guard<std::recursive_mutex> guard(m_lock);
		auto item = m_linker_map.find(connid);
		if (item != m_linker_map.end())
		{
			item->second->set_session_key(local_key);
		}
	}

    virtual EnHandleResult OnReceive(CONNID dwConnID, int iLength) override
    {
		EnHandleResult rel = HR_IGNORE;
		rel = NetServiceWrapper::receive_packet(dwConnID, iLength,
				std::bind(&NetServiceEx<session>::on_heart_beat, this, std::placeholders::_1),
				std::bind(&NetServiceEx<session>::on_query_internal_key, this, std::placeholders::_1),
				std::bind(&NetServiceEx<session>::decrypt_packet, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
				std::bind(&NetServiceEx<session>::add_user_packet, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
				std::bind(&NetServiceEx<session>::get_message_factory, this, std::placeholders::_1));

		return rel;
    }

	string decrypt_packet(CONNID connid, uint64_t ulpkey, uint8* data, uint32 size)
	{
		auto p_session = find_session(connid);
		if (nullptr == p_session)
		{
			return "";
		}

		return NetServiceWrapper::decrypt_packet(
									connid, 
									ulpkey, 
									data, 
									size, 
									p_session->get_session_key());
	}

	void remove_headers(CONNID dwConnID)
	{
		TPkgInfo* pInfo = find_header(dwConnID);
		SAFE_DELETE(pInfo);
	}

    virtual EnHandleResult OnAccept(CONNID dwConnID, SOCKET soClient) override
    {
        std::string ip;
		CHAR szAddress[40];
		int iAddressLen = sizeof(szAddress) / sizeof(CHAR);
		USHORT usPort = 0;

		m_server_engine->SetConnectionExtra(dwConnID, new TPkgInfo(true, sizeof(SQHeader)));
		m_server_engine->GetRemoteAddress(dwConnID, szAddress, iAddressLen, usPort);

		sesstion_ptr linker_ptr = std::make_shared<session>();
        if (linker_ptr)
        {
			ip = szAddress;
			linker_ptr->set_handle(dwConnID);
			linker_ptr->set_remote_ip(ip);
			linker_ptr->bind_send_func(std::bind(&NetServiceEx<session>::send_data, 
                this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
            std::lock_guard<std::recursive_mutex> guard(m_lock);
            m_linker_map.insert(std::make_pair(dwConnID, linker_ptr));
        }

        on_accept(linker_ptr);
		return HR_OK;
    }

    virtual EnHandleResult OnClose(CONNID dwConnID) override
    {
        if (m_listen_handle == dwConnID)
        {
            m_listen_handle = 0;
        }
        else
        {
			remove_headers(dwConnID);
            sesstion_ptr linker_ptr = nullptr;
            {
                std::lock_guard<std::recursive_mutex> guard(m_lock);
                auto iter = m_linker_map.find(dwConnID);
                if (iter != m_linker_map.end()) 
                {
                    linker_ptr = iter->second;
                    m_linker_map.erase(iter);
                }
            }

			if (linker_ptr != nullptr)
				on_close(linker_ptr);
        }

		return HR_OK;
    }

    virtual EnHandleResult OnError(CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode) override
    {
		/*
		// on_error对上层暂时不调用，直接抛on_close给上层，因为下面也会关闭连接
        auto p_session = find_session(dwConnID);
        if (nullptr != p_session)
        {
            on_error(p_session);
        }
		//*/
		OnClose(dwConnID);
		return HR_OK;
    }
	
	bool send_data(
		CONNID sock, 
		uint32_t message_id, 
		google::protobuf::Message *msg, 
		bool encrypt)
	{
		bool rel = false;
		auto p_session = find_session(sock);
		if (nullptr != p_session)
		{
			rel = NetServiceWrapper::send_data(
									sock, 
									message_id, 
									msg, 
									p_session->get_session_key(), 
									encrypt);
		}

		return rel;
	}

	bool send_data2(
		CONNID sock, 
		uint32_t message_id, 
		uint8* msg, 
		uint32 size)
	{
		return NetServiceWrapper::send_data2(
									sock,
									message_id,
									msg,
									size);
	}

    google::protobuf::Message* get_message_factory(
        uint32_t message_id)
    {
        auto pos = m_message_map.find(message_id);
        if (pos != m_message_map.end())
            return pos->second.second;

        return nullptr;
    }

private:
    uint32 m_listen_handle;
    uint32 m_listen_port;
    std::string m_listen_ip;
    std::map<CONNID, sesstion_ptr> m_linker_map;

    unordered_map<uint32_t, pair<callback_func_t, google::protobuf::Message*> > m_message_map;

    std::recursive_mutex m_lock;
    ThreadInterface<NtSeTask>* m_task_mgr_ptr;
};

}}

#endif