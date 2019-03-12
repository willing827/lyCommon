/*---------------------------------------------------------------------------*/
/*  sqwindowmsg.h                                                            */
/*                                                                           */
/*  History                                                                  */
/*      05/26/2017   create                                                  */
/*                                                                           */
/*  Author                                                                   */
/*       feng hao                                                            */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*---------------------------------------------------------------------------*/
#ifndef _WINDOW_MSG_PROC_H
#define _WINDOW_MSG_PROC_H

#include <memory>
#include <string>
#include <functional>
#include <sqwin/sqwin.h>
#include <sqstd/sqtypes.h>


namespace snqu{

class WindowMsgProc
{
public:
    WindowMsgProc();
    ~WindowMsgProc();

    typedef std::function<int(int wparam, int lparam)> umsg_call_back_t;

    /* �ж�һ����Ϣ�Ƿ���ڵģ����� */
    bool exist_msg(int message);

    /* ���һ����ϢID�Ļص����̰߳�ȫ */
    void add_msg_cb(int message, umsg_call_back_t);

    /* �Ƴ�һ����ϢID�Ļص����̰߳�ȫ */
    void del_msg_cb(int message);

    /* �첽���ã��ڲ��������߳� */
    int start(const std::string& window_name = "");

    /* ͬ�����ã���������ǰ�߳� ע���startֻ��ʹ��һ�֣��˷���һ��Ϊ���������߳�ʹ�� */
    void loop(const std::string& window_name = "");

    /* ֹͣģ�飬ͬ������, ����falseһ�������߳�loop�������� */
	bool stop();

    /* ��ȡ���ھ����Ҳ���Ը���window_name�������� */
    HWND get_hwnd();

    /* ���ӳ���ȫ���¼� ȫ��״̬�仯��ᴥ���ص� �˺���Ҫ��loop��start֮ǰ���ò���Ч��
        TRUE ==lParam->FullScreen On  
        FALSE==lParam->FullScreen OFF
    */
    bool watch_full_screen(int message, umsg_call_back_t);

private:
    struct impl;
    std::unique_ptr<impl> m_impl;
};

}

#endif // _WINDOW_MSG_PROC_H



