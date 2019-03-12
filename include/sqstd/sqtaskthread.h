#pragma once
#include "sqtaskqueue.h"
#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <sqwin/thread/sqevent.h>

namespace snqu{

    template<typename Task>
    class TaskThread
    {
    public:
        TaskThread()
        {
            m_stop.store(false);
        }

        ~TaskThread()
        {
            m_event.Destory();
        }

        virtual bool begin(const std::function<void(const Task&)>& on_task)
        {
			m_stop.store(false);
            m_on_task = on_task;

            if (!m_event.Create())
                return false;
			
            m_thread = std::thread(std::bind(&TaskThread::thread_proc, this));

            return true;
        }

        virtual void end()
        {
            m_stop.store(true);

            if (m_thread.get_id() != std::this_thread::get_id())
            {
                m_event.Set();
            }

            if (m_thread.joinable())
                m_thread.join();
        }

        void post_task(const Task& task)
        {
            m_tasks_que.push(task);
            m_event.Set();
        }

        int task_count() const
        {
            return m_tasks_que.task_count();
        }

        void thread_proc(void)
        {
            while (!m_stop.load())
            {
                m_event.Wait(INFINITE);
                m_event.Reset();
                do_all_task();
            }
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

    protected:
        std::function<void(const Task&)> m_on_task;
        std::thread m_thread;
        snqu::Event m_event;
        TaskQueue<Task> m_tasks_que;
        std::atomic_bool m_stop;
    };

}