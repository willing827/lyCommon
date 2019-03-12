
#include <sqstd/sqinc.h>
#include <functional>
#include <sqstd/sqtaskthread.h>

namespace snqu{ namespace net {

template <typename session_ptr, typename Message_ptr>
class Dispatcher
{
public:
	Dispatcher() {};
	~Dispatcher() {};

	typedef std::function<void(session_ptr, Message_ptr)> callback_func_t;
public:
    inline void register_packet(uint32_t message_id, callback_func_t callback)
	{
		if (m_message_map.find(message_id) != m_message_map.end())
			return;

		m_message_map[message_id] = callback;
		return;
	}

	inline void add_user_packet(uint32_t message_id, Message_ptr msg)
	{
		m_task_thread.post_task(std::make_pair(message_id, msg));
	}

	void detect_packet_proc(const std::pair<uint32_t, Message_ptr>& msg)
	{
		auto item =  m_message_map.find(message_id);
		if (item != m_message_map.end())
		{
			
		}
	}

private:
    std::map<uint32_t, std::function<void(session_ptr, packet_ptr)>> m_message_map;
	snqu::TaskThread<std::pair<uint32_t, Message_ptr>> m_task_thread;
};

}}