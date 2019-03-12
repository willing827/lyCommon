#ifndef SQTASK_QUEUE_H
#define SQTASK_QUEUE_H
#include <deque>
#ifdef _WIN32
#include <sqwin/thread/sqlock.h>
#else
#include <mutex>
#endif

namespace snqu{

template<typename Task>
class TaskQueue
{
public:
    TaskQueue()
    {
#ifdef _WIN32
        LockGuard<CriticalSection> lock(m_mutex);
#else
        std::lock_guard<std::mutex> m_lock(m_mutex);
#endif
    }

    void push(const Task& task)
    {
#ifdef _WIN32
        LockGuard<CriticalSection> lock(m_mutex);
#else
        std::lock_guard<std::mutex> m_lock(m_mutex);
#endif
        m_tasks_que.emplace_back(task);
    }

    void insert(const Task& task)
    {
#ifdef _WIN32
        LockGuard<CriticalSection> lock(m_mutex);
#else
        std::lock_guard<std::mutex> m_lock(m_mutex);
#endif
        m_tasks_que.push_front(task);
    }

    bool pop(Task& task)
    {
#ifdef _WIN32
        LockGuard<CriticalSection> lock(m_mutex);
#else
        std::lock_guard<std::mutex> m_lock(m_mutex);
#endif
        if (!m_tasks_que.empty())
        {
            task = m_tasks_que.front();
            m_tasks_que.pop_front();
            return true;
        }
        else
        {
            return false;
        }
    }

    bool is_empty() const
    {
#ifdef _WIN32
        LockGuard<CriticalSection> lock(m_mutex);
#else
        std::lock_guard<std::mutex> m_lock(m_mutex);
#endif
        bool empty = m_tasks_que.empty();
        return empty;
    }

    int task_count() const
    {
#ifdef _WIN32
        LockGuard<CriticalSection> lock(m_mutex);
#else
        std::lock_guard<std::mutex> m_lock(m_mutex);
#endif
        int count = m_tasks_que.size();
        return count;
    }

    void clear()
    {
#ifdef _WIN32
        LockGuard<CriticalSection> lock(m_mutex);
#else
        std::lock_guard<std::mutex> m_lock(m_mutex);
#endif
        m_tasks_que.clear();
    }

private:
#ifdef _WIN32
    mutable CriticalSection m_mutex;
#else
    mutable std::recursive_mutex m_mutex;
#endif
    std::deque<Task> m_tasks_que;
};
}

#endif