#ifndef SQSESSION_H
#define SQSESSION_H

#include <sqstd/sqinc.h>
#include <HPSocket/HPSocket.h>
#include <sqnet/sqnet2.h>
#include <sqnet/sqnethelper2.h>


namespace snqu { namespace net2{

class Session
{
public:
    Session(void)
        : m_handle(SQ_INVALID_SOCKET)
        , m_expire_time(::time(NULL))
    {
		m_session_key = "";
		m_send_func = nullptr;
		m_send_raw_func = nullptr;
    }
    virtual ~Session(void)
    {
    }

    unsigned int get_handle() const { return m_handle; }
    void set_handle(CONNID linker_handle) { m_handle = linker_handle; }

    const std::string& get_remote_ip() const { return m_remote_ip; }
    void set_remote_ip(const std::string& linker_ip) { m_remote_ip = linker_ip; }

    ::time_t get_expire_time() const { return m_expire_time; }
    void set_expire_time(int64 expire_time) { m_expire_time = expire_time; }
    
	void bind_send_func(send_func_t func) {m_send_func = func;}
	void bind_send_raw_func(send_raw_func_t func) {m_send_raw_func = func;}

    bool send(uint32_t message_id, google::protobuf::Message* data, bool encrypt = true)
    {
		if (m_session_key.empty())
			return false;

		if (nullptr != m_send_func)
		{
 			return m_send_func(m_handle, message_id, data, encrypt);
		}
        return false;
    }

	bool send_raw(uint32_t message_id, const string& data, bool encrypt = true)
	{
		if (m_session_key.empty())
			return false;

		if (nullptr != m_send_raw_func)
		{
			return m_send_raw_func(m_handle, message_id, data, encrypt);
		}
		return false;
	}

	inline TPkgInfo* get_pkg_info() { return &m_pkg_info; }

	inline void set_session_key(const string& key) { m_session_key = key; }
	inline const string& get_session_key() { return m_session_key; }
	inline bool can_send_data() {return m_session_key.empty() ? false : true;}

private:
    CONNID       m_handle;
    time_t       m_expire_time;
    std::string  m_remote_ip;
	send_func_t	 m_send_func;
	send_raw_func_t m_send_raw_func;
	TPkgInfo     m_pkg_info;
	std::string  m_session_key;
};

typedef std::shared_ptr<Session> SessionPtr;
}}
#endif //SQSESSION_H