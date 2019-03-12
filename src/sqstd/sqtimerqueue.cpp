#include <sqstd/sqtimerqueue.h>
#include <atomic>
#include <sqstd/sqtime.h>

namespace snqu {namespace common{

struct TimerData
{
    TimerData()
        : timer_proc_(nullptr), timer_id_(0),
        interval_(0), is_delete(false)
    {
        last_time_ = 0;
    }

    int timer_id_;
    __int64 last_time_;
    unsigned int interval_;
    volatile bool is_delete;
    std::function<void(uint32_t)> timer_proc_;
};

typedef std::shared_ptr<TimerData> TimerData_ptr;


TimerQueue::TimerQueue()
{
#ifdef _WIN32
    LockGuard<CriticalSection> lock(m_mutex);
#else
    std::lock_guard<std::recursive_mutex> m_lock(m_mutex);
#endif
}
TimerQueue::~TimerQueue(){}


void TimerQueue::SetTimer(int timer_id, unsigned int millisecond, std::function<void(int timer_id)> proc)
{
#ifdef _WIN32
    LockGuard<CriticalSection> lock(m_mutex);
#else
    std::lock_guard<std::recursive_mutex> m_lock(m_mutex);
#endif
    TimerData timer;
    timer.timer_id_ = timer_id;
    timer.last_time_ = snqu::GetTickCount64();
    timer.timer_proc_ = proc;
    timer.interval_ = millisecond;
    m_timers.emplace(timer_id, timer);
}

void TimerQueue::DoAllTimer()
{
    std::map<int, TimerData> timer_temp;
    {
#ifdef _WIN32
        LockGuard<CriticalSection> lock(m_mutex);
#else
        std::lock_guard<std::recursive_mutex> m_lock(m_mutex);
#endif
        if (m_timers.empty()) return;
        timer_temp = m_timers;
    }
    int cur_size = timer_temp.size();
    using namespace std::chrono;
    auto now = snqu::GetTickCount64();
    for (auto& timer_item : timer_temp)
    {
        if (timer_item.second.is_delete) continue;
        auto duration = now - timer_item.second.last_time_;
        if (duration >= timer_item.second.interval_)
        {
            timer_item.second.last_time_ = now;
            m_cur_timer_id = timer_item.second.timer_id_;
            timer_item.second.timer_proc_(timer_item.first);
        }
    }
        
    {
#ifdef _WIN32
        LockGuard<CriticalSection> lock(m_mutex);
#else
        std::lock_guard<std::recursive_mutex> m_lock(m_mutex);
#endif
        for (auto& iterm : m_timers)
        {
            if (0 == timer_temp.count(iterm.first))
            {// 临时timer列表中不存在就是新增的
                timer_temp.emplace(iterm);
            }
            else
            {// 更新过
                if (iterm.second.interval_ != timer_temp[iterm.first].interval_)
                {// 更新过
                    timer_temp[iterm.first] = iterm.second;
                }
                if (0 == iterm.second.last_time_
                    && now != timer_temp[iterm.first].last_time_)
                {// notify过
                    timer_temp[iterm.first].last_time_ = 0;
                }
                if (iterm.second.last_time_ > now)
                {// update过
                    timer_temp[iterm.first].last_time_ = iterm.second.last_time_;
                }
				if (iterm.second.is_delete)
				{// 需要删除
					timer_temp[iterm.first].is_delete = true;
				}
            }
        }

        // 删除不用的timer
        auto iter = timer_temp.begin();
        while (iter != timer_temp.end())
        {
            if (iter->second.is_delete)
                timer_temp.erase(iter++);
            else
                iter++;
        }

		m_timers.swap(timer_temp);
    }
}

void TimerQueue::UpdateTimer(int timer_id)
{
#ifdef _WIN32
    LockGuard<CriticalSection> lock(m_mutex);
#else
    std::lock_guard<std::recursive_mutex> m_lock(m_mutex);
#endif
    auto iter = m_timers.find(timer_id);
    if (iter != m_timers.end())
        iter->second.last_time_ = snqu::GetTickCount64();
}

void TimerQueue::NotifyTimer(int timer_id)
{
#ifdef _WIN32
    LockGuard<CriticalSection> lock(m_mutex);
#else
    std::lock_guard<std::recursive_mutex> m_lock(m_mutex);
#endif
    auto iter = m_timers.find(timer_id);
    if (iter != m_timers.end())
        iter->second.last_time_ = 0;
}

void TimerQueue::KillTimer(int timer_id)
{
#ifdef _WIN32
    LockGuard<CriticalSection> lock(m_mutex);
#else
    std::lock_guard<std::recursive_mutex> m_lock(m_mutex);
#endif
    auto iter = m_timers.find(timer_id);
    if (iter != m_timers.end())
        iter->second.is_delete = true;
}

void TimerQueue::Clear()
{
#ifdef _WIN32
    LockGuard<CriticalSection> lock(m_mutex);
#else
    std::lock_guard<std::recursive_mutex> m_lock(m_mutex);
#endif
    m_timers.clear();
}

void TimerQueue::KillAllTimer()
{
#ifdef _WIN32
    LockGuard<CriticalSection> lock(m_mutex);
#else
    std::lock_guard<std::mutex> m_lock(m_mutex);
#endif
    for(auto& it : m_timers)
    {
        it.second.is_delete = true;
    }
}

}}
