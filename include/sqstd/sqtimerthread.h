#pragma once

#include <thread>
#include <condition_variable>
#include <mutex>
#include <chrono>
#include <sqstd/sqtimerqueue.h>
#include <atomic>

namespace snqu{

    class TimerThread : public common::TimerQueue
    {
    public:
        TimerThread()
        {
            m_stop.store(true);
        }
        ~TimerThread()
        {
            if (!m_stop.load())
            {
                end();
            }
        }

        bool begin()
        {
            m_stop.store(false);
            m_thread = std::thread(std::bind(&TimerThread::thread_proc, this));
            return true;
        }

        void thread_proc()
        {
            while (!m_stop.load())
            {
                DoAllTimer();
                std::chrono::milliseconds dura(10);
                std::this_thread::sleep_for(dura);
            }
            Clear();
        }

        void end()
        {
            if (m_stop) return;

            m_stop.store(true);
            KillAllTimer();
            if (m_thread.native_handle() != nullptr && 
                m_thread.get_id() != std::this_thread::get_id() &&
                m_thread.joinable())
            {
                m_thread.join();
            }

            if (m_thread.joinable())
                m_thread.detach();
        }

    private:
        std::atomic_bool m_stop;
        std::thread m_thread;
    };

}