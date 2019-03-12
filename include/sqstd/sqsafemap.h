#ifndef SNQU_SAFE_MAP_H
#define SNQU_SAFE_MAP_H

#include <map>
#ifdef _WIN32
#include <sqwin/thread/sqlock.h>
#else
#include <mutex>
#endif

namespace snqu { 

template<typename key, typename data>
class SafeMap
{
public:
    typedef std::map<key, data> safe_map_t;
    typedef typename safe_map_t::value_type value_type;
	typedef typename safe_map_t::iterator iter_type;
    SafeMap()
    {
#ifdef _WIN32
        LockGuard<CriticalSection> lock(m_mutex);
#else
        std::lock_guard<std::mutex> m_lock(m_mutex);
#endif
    }

    ~SafeMap()
    {
        clear();
    }
    void SetVal(const key& key_in, const data& data_in)
    {
#ifdef _WIN32
        LockGuard<CriticalSection> lock(m_mutex);
#else
        std::lock_guard<std::mutex> m_lock(m_mutex);
#endif
        auto iter = m_std_map.find(key_in);
        if (iter == m_std_map.end())
        {
            m_std_map.emplace(std::make_pair(key_in, data_in));
        }
        else
            m_std_map[key_in] = data_in;
    }

    void erase(typename safe_map_t::const_iterator data_iter)
    {
#ifdef _WIN32
        LockGuard<CriticalSection> lock(m_mutex);
#else
        std::lock_guard<std::mutex> m_lock(m_mutex);
#endif
        m_std_map.erase(data_iter);
    }

    void erase(const key& key_value)
    {
#ifdef _WIN32
        LockGuard<CriticalSection> lock(m_mutex);
#else
        std::lock_guard<std::mutex> m_lock(m_mutex);
#endif
        m_std_map.erase(key_value);
    }

    std::map<key, data> copy()
    {
#ifdef _WIN32
        LockGuard<CriticalSection> lock(m_mutex);
#else
        std::lock_guard<std::mutex> m_lock(m_mutex);
#endif
        std::map<key, data> ret;
        ret = m_std_map;
        return std::move(ret);
    }

    bool exist(const key& key_value)
    {
#ifdef _WIN32
        LockGuard<CriticalSection> lock(m_mutex);
#else
        std::lock_guard<std::mutex> m_lock(m_mutex);
#endif
        return m_std_map.find(key_value) != m_std_map.end ();
    }

    void clear()
    {
#ifdef _WIN32
        LockGuard<CriticalSection> lock(m_mutex);
#else
        std::lock_guard<std::mutex> m_lock(m_mutex);
#endif
        m_std_map.clear();
    }

    void foreach(std::function<bool (typename safe_map_t::value_type&)> func)
    {
#ifdef _WIN32
        LockGuard<CriticalSection> lock(m_mutex);
#else
        std::lock_guard<std::mutex> m_lock(m_mutex);
#endif
        for (auto iter = m_std_map.begin(); iter != m_std_map.end(); iter++)
        {
            if(func(*iter))
                break;
        }
    }

    void del_if(std::function<bool (typename safe_map_t::value_type&)> func)
    {
#ifdef _WIN32
        LockGuard<CriticalSection> lock(m_mutex);
#else
        std::lock_guard<std::mutex> m_lock(m_mutex);
#endif
        for (auto iter = m_std_map.begin(); iter != m_std_map.end();)
        {
            if(func(*iter))
            {
                m_std_map.erase(iter++);
            }
            else
            {
                 ++iter;
            }
        }
    }

    void oper(const key& _key, std::function<void(data&)> func)
    {
#ifdef _WIN32
        LockGuard<CriticalSection> lock(m_mutex);
#else
        std::lock_guard<std::mutex> m_lock(m_mutex);
#endif
        if (m_std_map.find(_key) != m_std_map.end())
            func(m_std_map[_key]);
    }

	bool get(const key& _key, data& ret)
	{
#ifdef _WIN32
        LockGuard<CriticalSection> lock(m_mutex);
#else
        std::lock_guard<std::mutex> m_lock(m_mutex);
#endif
		auto iter = m_std_map.find(_key);
        if (iter != m_std_map.end())
            ret = iter->second;
		return iter != m_std_map.end();
	}

    void swap(std::map<key, data>& new_data)
    {
#ifdef _WIN32
        LockGuard<CriticalSection> lock(m_mutex);
#else
        std::lock_guard<std::mutex> m_lock(m_mutex);
#endif
        m_std_map.swap(new_data);
    }

    int size()
    {
#ifdef _WIN32
        LockGuard<CriticalSection> lock(m_mutex);
#else
        std::lock_guard<std::mutex> m_lock(m_mutex);
#endif
        return m_std_map.size();
    }

private:
	std::map<key, data>& get_map()
	{
		return m_std_map;
	}
    SafeMap(const SafeMap&);
    const SafeMap& operator=(const SafeMap&);

private:
    std::map<key, data> m_std_map;
#ifdef _WIN32
    mutable CriticalSection m_mutex;
#else
    mutable std::mutex m_mutex;
#endif
    
};

}
#endif