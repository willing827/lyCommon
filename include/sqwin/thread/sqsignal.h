/*-------------------------------------------------------------------------- - */
/*  sqsignal.h                                                                */
/*                                                                           */
/*  History                                                                  */
/*      05/28/2015                                                           */
/*                                                                           */
/*  Author                                                                   */
/*       Guo Lei                                                             */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*---------------------------------------------------------------------------*/
#ifndef SNQU_SIGNAL_H
#define SNQU_SIGNAL_H
#include <sqwin/sqwin.h>


namespace snqu{

    class Signal
    {
    public:
        Signal();
        ~Signal();

        // 创建一个锁
        bool Create(const char* name = "");
        bool Open(const char* name);
        int  Lock(int second = INFINITE);
        void Unlock();
        void Destory();

    private:
        HANDLE m_signal;
        Signal(const Signal&){};
        Signal& operator=(const Signal& a){};
    };
}

#endif // SNQU_SIGNAL_H