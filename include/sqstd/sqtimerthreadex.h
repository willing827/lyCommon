#pragma once

#include <thread>
#include <condition_variable>
#include <mutex>
#include <sqstd/sqtypes.h>
#include <chrono>
#include <sqstd/sqsafemap.h>

namespace snqu{

    struct TimerInfoEx
    {
		inline TimerInfoEx()
		{
			timer_id = 0;
			last_time = 0;
			interval = 1;
			is_delete = false;
			timer_proc = nullptr;
		}

        uint32_t timer_id;
        time_t last_time;
        int32 interval;
		bool  is_delete;
        std::function<void(uint32_t)> timer_proc;
    };


class TimerThreadEx
{
public:
    TimerThreadEx()
        : m_stop(false)
        , m_interval(0)
        , m_on_timer(nullptr)
    {}
    ~TimerThreadEx(){}

	bool begin()
    {
		m_stop = false;
        m_thread = std::thread(std::bind(&TimerThreadEx::thread_proc, this));
        return true;
    }

	void set_timer(uint32_t id, uint32_t second, std::function<void(uint32_t)> proc)
	{
		if (m_timer_map.exist(id))
			return;

        std::pair<uint32_t, TimerInfoEx> timer_item;
        timer_item.first = id;
		timer_item.second.timer_id = id;
		timer_item.second.last_time = time(NULL);
		timer_item.second.timer_proc = proc;
		timer_item.second.interval = second;
        m_timer_map.insert(timer_item);
	}

	void kill_timer(uint32_t id)
	{
		if (m_thread.get_id() == this_thread::get_id())
		{
			auto pos = m_timer_map.get_map().find(id);
			if (pos != m_timer_map.get_map().end())
			{
				pos->second.is_delete = true;
			}
		}
		else
			m_timer_map.erase(id);
	}

    void thread_proc()
    {
		using namespace std::chrono;
        while (!m_stop)
        {
            m_timer_map.foreach([&](SafeMap<uint32_t, TimerInfoEx>::value_type& item)
            {
                if (m_stop) return;
                auto now = time(NULL);
                int delta = (int)(now - item.second.last_time);
                if (delta >= item.second.interval || delta < 0)
                {
                    item.second.last_time = now;
                    item.second.timer_proc(item.first);
                }
            });

			m_timer_map.del_if([&](SafeMap<uint32_t, TimerInfoEx>::value_type& item)->bool
			{
				return item.second.is_delete;
			});

           Sleep(50);
		}
        m_timer_map.clear();
    }

    void end()
    {
        m_stop = true;
		if (m_thread.native_handle() != nullptr && m_thread.joinable())
			m_thread.join();
    }

private:
    uint32 m_interval;
    bool m_stop;
    std::function<void(uint32_t)> m_on_timer;
    std::thread m_thread;
	SafeMap<uint32_t/*timer_id*/, TimerInfoEx> m_timer_map;
	std::mutex m_mutex;
};

}