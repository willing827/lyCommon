/*---------------------------------------------------------------------------*/
/*  sqcs.h                                                                   */
/*                                                                           */
/*  History                                                                  */
/*      08/30/2017                                                           */
/*                                                                           */
/*  Author                                                                   */
/*       Feng Hao                                                            */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*---------------------------------------------------------------------------*/

#ifndef SNQU_LOCK_H
#define SNQU_LOCK_H
#include <sqwin/thread/sqcs.h>
#include <sqwin/thread/sqsignal.h>
#include <sqwin/thread/sqmutex.h>

namespace snqu{

    struct not_lock_t
    {	// indicates adopt lock
    };

    template<class _Mutex>
    class LockGuard
    {
    public:
        explicit LockGuard(_Mutex& mutex)
            : m_mutex(mutex)
        {
            m_mutex.Lock();
        }

        // ²»ÉÏËø
        LockGuard(_Mutex& mutex, not_lock_t)
            : m_mutex(mutex)
        {}

        ~LockGuard()
        {
            m_mutex.Unlock();
        };

    private:
        _Mutex& m_mutex;
        LockGuard(const LockGuard&) {};
        LockGuard& operator=(const LockGuard& a) {};
    };
}

#endif // SNQU_CRITICAL_SECTION_H