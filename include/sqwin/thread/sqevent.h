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

    // ����һ��Ĭ���Զ��¼�
    bool Create();

    // ����һ���¼�
    bool Create(LPSECURITY_ATTRIBUTES event_attributes, bool manual_reset, bool initial_state, const char* name);

    // �ȴ��¼�
    bool Wait(unsigned long wait_time);

    // �����¼�
    bool Set();

    // �ͷ��¼�
    bool Reset();

	// ��
	bool Open(unsigned long access, const char *name);

    // ɾ���¼�
    bool Destory();

private:
    HANDLE m_handle;
    bool m_is_open;
    Event(const Event&) {};
    Event& operator=(const Event& a) {};
};
}

#endif // SNQU_EVENT_H
