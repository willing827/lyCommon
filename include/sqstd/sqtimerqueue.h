/*-------------------------------------------------------------------------*/
/*  sqtimerqueue.h															 */
/*                                                                           */
/*  History                                                                  */
/*      20150902:create  													 */
/*                                                                           */
/*  Author                                                                   */
/*      fenghao                                                              */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*-------------------------------------------------------------------------*/
#ifndef SQTIMER_QUEUE_H
#define SQTIMER_QUEUE_H

#ifdef _WIN32
#include <sqwin/thread/sqlock.h>
#include <atomic>
#else
#include <mutex>
#endif
#include <map>

namespace snqu {namespace common{

struct TimerData;

class TimerQueue
{
public:
    TimerQueue();
    ~TimerQueue();

    //此处注意，已经存在的timer会被修改数据
    void SetTimer(int timer_id, unsigned int millisecond, std::function<void(int timer_id)> proc);
    //延迟执行timer
    void UpdateTimer(int timer_id);
    //立刻执行timer
    void NotifyTimer(int timer_id);
    //关闭timer
    void KillTimer(int timer_id);

    void DoAllTimer();
    void KillAllTimer();

protected:
    void Clear();

private:
#ifdef _WIN32
    mutable CriticalSection m_mutex;
#else
    mutable std::recursive_mutex m_mutex;
#endif
    std::map<int, TimerData> m_timers;

    std::atomic_int m_cur_timer_id; // 当前正在执行的timer_id
};

}}
#endif //SQTIMER_QUEUE_H