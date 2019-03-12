#pragma once
#include "sqtaskqueue.h"
#include <thread>
#include <condition_variable>
#include <mutex>
#include <sqstd/sqtypes.h>
#include <chrono>
#include <map>
#include <atomic>
#include <sqstd/sqtimerqueue.h>


namespace snqu{

	struct TimerInfot
	{
		uint32_t timer_id;
		time_t last_time;
		int32 interval;
		std::function<void(uint32_t)> timer_proc;
	};

template<typename Task>
class TimerTaskThread
{
public:
    TimerTaskThread()
        : m_on_timer(nullptr)
    {
        m_stop.store(false);
    }
    ~TimerTaskThread(){}

	bool begin(const std::function<void(const Task&)>& on_task)
    {
        m_stop.store(false);
        m_on_task = on_task;

        m_thread = std::thread(std::bind(&TimerTaskThread::thread_proc, this));

        return true;
    }

    void post_task(const Task& task)
    {
        m_tasks_que.push(task);
    }

    int task_count() const
    {
        return m_tasks_que.task_count();
    }

	void SetTimer(uint32_t id, uint32_t second, std::function<void(uint32_t)> proc)
	{
		m_timers.SetTimer(id, second, proc);
	}

    void notify_timer(uint32_t id)
    {
		m_timers.NotifyTimer(id);
    }

	void kill_timer(uint32_t id)
	{
		m_timers.KillTimer(id);
	}

    void thread_proc()
    {
		using namespace std::chrono;
        while (!m_stop.load())
        {
            do_all_task();

			m_timers.DoAllTimer();

			Sleep(50);
		}
    }

    void end()
    {
        m_stop.store(true);
        if (m_thread.native_handle() != nullptr && 
            m_thread.get_id() != std::this_thread::get_id() &&
            m_thread.joinable())
        {
            m_thread.join();
        }

        if (m_thread.joinable())
            m_thread.detach();
    }

    DWORD get_thread_id() const{ return m_thread.get_id(); }

private:
    virtual void do_all_task()
    {
        Task task;
        while (m_tasks_que.pop(task))
        {
            if (m_on_task != nullptr)
            {
                m_on_task(task);
            }
        }
    }

private:
    atomic_bool m_stop;
    std::function<void(uint32_t)> m_on_timer;
    std::thread m_thread;
	common::TimerQueue m_timers;
	std::map<uint32_t/*timer_id*/, TimerInfot> m_timer_map;
    TaskQueue<Task> m_tasks_que;
    std::function<void(const Task&)> m_on_task;
};

}