#pragma once
#include <sqstd/sqtaskqueue.h>


namespace snqu {

    template<typename Task>
    class TaskLoop
    {
    public:
        TaskLoop()
        {
            m_stop.store(true);
        }
        ~TaskLoop()
        {
            m_event.Destory();
        }

        void loop(const std::function<void(const Task&)>& on_task)
        {
            m_thd_id = std::this_thread::get_id();
            m_stop = false;

            if (!m_event.Create())
                return;

            while (!m_stop.load())
            {
                m_event.Wait(INFINITE);
                m_event.Reset();

                Task task;
                while (m_tasks_que.pop(task))
                {
                    if (on_task != nullptr)
                    {
                        on_task(task);
                    }
                }
            }
            
        }

        void quit()
        {
            if (m_stop) return;

            m_stop.store(true);

            if (m_thd_id != std::this_thread::get_id())
            {
                m_event.Set();
            }
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

    private:
        snqu::Event m_event;
        TaskQueue<Task> m_tasks_que;
        std::atomic_bool m_stop;
        thread::id m_thd_id;
    };



}