/*---------------------------------------------------------------------------*/
/*  sqfilesignature.h                                                        */
/*                                                                           */
/*  History                                                                  */
/*      05/31/2017  create                                                   */
/*                                                                           */
/*  Author                                                                   */
/*       feng hao                                                            */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*---------------------------------------------------------------------------*/

#ifndef SNQU_EVENT_H
#define SNQU_EVENT_H

#include <sqwin/sqwin.h>

namespace snqu{

class Event
{
public:
    Event();
    explicit Event(HANDLE h);
    ~Event();

    // 创建一个默认自动事件
    bool Create();

    // 创建一个事件
    bool Create(LPSECURITY_ATTRIBUTES event_attributes, bool manual_reset, bool initial_state, const char* name);

    // 等待事件
    bool Wait(unsigned long wait_time);

    // 设置事件
    bool Set();

    // 释放事件
    bool Reset();

	// 打开
	bool Open(unsigned long access, const char *name);

    // 删除事件
    bool Destory();

private:
    HANDLE m_handle;
    bool m_is_open;
    Event(const Event&) {};
    Event& operator=(const Event& a) {};
};
}

#endif // SNQU_EVENT_H
