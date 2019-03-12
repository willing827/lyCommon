#pragma once

#include <set>
#ifdef _WIN32
#include <sqwin/thread/sqlock.h>
#else
#include <mutex>
#endif
namespace snqu { 

template<typename data>
class SafeSet
{
public:
    typedef std::set<data> safe_set_t;
    typedef typename safe_set_t::value_type value_type;
	typedef typename safe_set_t::iterator iter_type;
    SafeSet()
    {
#ifdef _WIN32
        LockGuard<CriticalSection> lock(m_mutex);
#else
        std::lock_guard<std::mutex> m_lock(m_mutex);
#endif
    }
    ~SafeSet()
    {
        clear();
    }

    bool add(const data& data_item)
    {
#ifdef _WIN32
        LockGuard<CriticalSection> lock(m_mutex);
#else
        std::lock_guard<std::mutex> m_lock(m_mutex);
#endif
        if (m_std_set.find(data_item) != m_std_set.end())
            return false;
        m_std_set.insert(data_item);
        return true;
    }

    void erase(const data& data_item)
    {
#ifdef _WIN32
        LockGuard<CriticalSection> lock(m_mutex);
#else
        std::lock_guard<std::mutex> m_lock(m_mutex);
#endif
        m_std_set.erase(data_item);
    }

    bool exist(const data& key_value)
    {
#ifdef _WIN32
        LockGuard<CriticalSection> lock(m_mutex);
#else
        std::lock_guard<std::mutex> m_lock(m_mutex);
#endif
        return m_std_set.find(key_value) != m_std_set.end();
    }

    void clear()
    {
#ifdef _WIN32
        LockGuard<CriticalSection> lock(m_mutex);
#else
        std::lock_guard<std::mutex> m_lock(m_mutex);
#endif
        m_std_set.clear();
    }

    void foreach(std::function<bool (typename safe_set_t::value_type&)> func)
    {
#ifdef _WIN32
        LockGuard<CriticalSection> lock(m_mutex);
#else
        std::lock_guard<std::mutex> m_lock(m_mutex);
#endif
        for (auto iter = m_std_set.begin(); iter != m_std_set.end();)
        {
            if(func(iter->second))
                break;
        }
    }

    void del_if(std::function<bool (typename safe_set_t::value_type&)> func)
    {
#ifdef _WIN32
        LockGuard<CriticalSection> lock(m_mutex);
#else
        std::lock_guard<std::mutex> m_lock(m_mutex);
#endif
        for (auto iter = m_std_set.begin(); iter != m_std_set.end();)
        {
            data& a = const_cast<data>(*iter);
            if(func(a))
            {
                m_std_set.erase(iter++);
            }
            else
            {
                 ++iter;
            }
        }
    }

    void swap(std::set<data>& new_data)
    {
#ifdef _WIN32
        LockGuard<CriticalSection> lock(m_mutex);
#else
        std::lock_guard<std::mutex> m_lock(m_mutex);
#endif
        m_std_set.swap(new_data);
    }

private:
    std::set<data> m_std_set;
#ifdef _WIN32
    mutable CriticalSection m_mutex;
#else
    mutable std::mutex m_mutex;
#endif
};

}