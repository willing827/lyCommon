/*---------------------------------------------------------------------------*/
/*  threadhelper.h                                                           */
/*                                                                           */
/*  History                                                                  */
/*      05/25/2017  create                                                   */
/*                                                                           */
/*  Author                                                                   */
/*       feng hao                                                            */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*---------------------------------------------------------------------------*/
#ifndef _THREAD_HELPER_H
#define _THREAD_HELPER_H

#ifdef _WIN32
#include <sqwin/sqwin.h>
#else
#include <pthread.h>
#endif

namespace snqu {

inline unsigned long GetCurThdID()
{
#ifdef WIN32
	return GetCurrentThreadId();
#else
	return thread_self();
#endif
}

}

#endif

