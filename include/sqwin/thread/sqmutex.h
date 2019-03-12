/*---------------------------------------------------------------------------*/
/*  sqmutex.h                                                                */
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

#ifndef SNQU_MUTEX_H
#define SNQU_MUTEX_H
#include <sqwin/sqwin.h>


namespace snqu{

    class Mutex
    {
    public:
        Mutex();
        Mutex(const char* name);
        explicit Mutex(HANDLE h);
        ~Mutex();

        // 创建一个锁
        bool Create(const char* name);
        // 打开
        bool Open(unsigned long access, const char *name);
        // 等待
        bool Lock(unsigned long wait_time = INFINITE);
        // 释放
        void Unlock();

        void Destroy();

    private:
        HANDLE m_mutex;
        Mutex(const Mutex&) {};
        Mutex& operator=(const Mutex& a) {};
    };
}

#endif // SNQU_MUTEX_H