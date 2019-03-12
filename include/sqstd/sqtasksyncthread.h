#pragma once
#include "sqtaskqueue.h"
#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <memory>
#include <sqwin/thread/sqevent.h>

namespace snqu{

    struct SyncTaskParam
    {
        int result;
        std::function<int (void)> task_func;
    };
    typedef std::shared_ptr<SyncTaskParam> SyncTaskParam_ptr;

    template<typename Task>
    class SyncTaskThread
    {
    public:
        SyncTaskThread()
            : m_stop(true)
        {
            m_task_sequence = 1;
            m_cur_sequence = 0;
        }
        ~SyncTaskThread()
        {}

        bool begin()
        {
            if (!m_stop)
                return true;

            if (!m_event.Create())
                return false;
            if (!m_sync_event.Create())
                return false;

             m_stop = false;

            m_thread = std::thread(std::bind(&SyncTaskThread::thread_proc, this));

            m_thread.detach();

            return true;
        }

        virtual void end()
        {
            if (m_stop) return;

            m_stop = true;

            if (m_thread.get_id() != std::this_thread::get_id())
            {
                m_event.Set();
                m_sync_event.Set();
            }

            m_event.Destory();
            m_sync_event.Destory();
        }

        void post_task(const Task& task)
        {
            m_tasks_que.push(task);
            m_event.Set();
        }

        int do_task(Task task, int out_time = 5)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_tasks_que.insert(task);
            m_event.Set();
            task->task_id = m_task_sequence++;
            m_cur_sequence = task->task_id;
            if (!m_sync_event.Wait(out_time*1000))
            {
                m_cur_sequence = 0;
                m_sync_event.Reset();
                return 10505;// ³¬Ê±
            }
            m_sync_event.Reset();
            return 0;
        }

        DWORD get_thread_id() const{ return m_thread.get_id(); }

        int task_count() const
        {
            return m_tasks_que.task_count();
        }

    private:

        void thread_proc(void)
        {
            while (!m_stop)
            {
                m_event.Wait(INFINITE);
                m_event.Reset();
                do_all_task();
            }
        }

        void do_all_task()
        {
            Task task;
            while (m_tasks_que.pop(task))
            {
                task->result = task->task_func();
                if (task->task_id == m_cur_sequence)
                {
                    m_cur_sequence = 0;
                    m_sync_event.Set();
                }
            }
        }

    private:
        std::thread m_thread;
        snqu::Event m_sync_event;
        snqu::Event m_event;
        TaskQueue<Task> m_tasks_que;
        std::mutex m_mutex;
        bool m_stop;
        atomic_int m_task_sequence;
        atomic_int m_cur_sequence;
    };

}