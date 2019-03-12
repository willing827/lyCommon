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

    /* 判断一个消息是否存在的，少用 */
    bool exist_msg(int message);

    /* 添加一个消息ID的回调，线程安全 */
    void add_msg_cb(int message, umsg_call_back_t);

    /* 移除一个消息ID的回调，线程安全 */
    void del_msg_cb(int message);

    /* 异步调用，内部会启动线程 */
    int start(const std::string& window_name = "");

    /* 同步调用，会阻塞当前线程 注意和start只能使用一种，此方法一般为阻塞在主线程使用 */
    void loop(const std::string& window_name = "");

    /* 停止模块，同步调用, 返回false一般是主线程loop被卡死了 */
	bool stop();

    /* 获取窗口句柄，也可以根据window_name参数查找 */
    HWND get_hwnd();

    /* 监视程序全屏事件 全屏状态变化后会触发回调 此函数要在loop和start之前调用才有效果
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



