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

#ifndef SNQU_CRITICAL_SECTION_H
#define SNQU_CRITICAL_SECTION_H
#include <sqwin/sqwin.h>

namespace snqu{

    class CriticalSection
    {
    public:
        CriticalSection();
        ~CriticalSection();

        void Lock();
        void Unlock();

    private:
        CRITICAL_SECTION m_lock;
        CriticalSection(const CriticalSection&) {};
        CriticalSection& operator=(const CriticalSection& a) {};

        void Create();
        void Destory();
    };
}

#endif // SNQU_CRITICAL_SECTION_H