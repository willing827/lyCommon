/*---------------------------------------------------------------------------*/
/*  sqwinthread.h                                                        */
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

#ifndef __WINTHREAD_H__  
#define __WINTHREAD_H__  

#include <string>  
#include <process.h>  


namespace snqu {

    class WinRunnable
    {
    public:
        virtual ~WinRunnable() {};
        virtual void Run() = 0;
    };

    class WinThread : public WinRunnable
    {
    private:
        explicit WinThread(const WinThread & rhs);

    public:
        WinThread();
        WinThread(WinRunnable * pRunnable);
        WinThread(const char * ThreadName, WinRunnable * pRunnable = NULL);
        WinThread(std::string ThreadName, WinRunnable * pRunnable = NULL);
        ~WinThread(void);

        /**
        开始运行线程
        @arg bSuspend 开始运行时是否挂起
        **/
        bool Start(bool bSuspend = false);

        /**
        运行的线程函数，可以使用派生类重写此函数
        **/
        virtual void Run();

        /**
        当前执行此函数线程等待线程结束
        @arg timeout 等待超时时间，如果为负数，等待无限时长
		@ret 超时或者出错返回false
        **/
		bool Join(int timeout = -1);
        /**
        恢复挂起的线程
        **/
        void Resume();
        /**
        挂起线程
        **/
        void Suspend();
        /**
        终止线程的执行
        **/
        bool Terminate(unsigned long ExitCode);

        unsigned int GetThreadID();
        std::string GetThreadName();
        void SetThreadName(std::string ThreadName);
        void SetThreadName(const char * ThreadName);

    private:
        static unsigned int __stdcall StaticThreadFunc(void * arg);

    private:
        void* m_handle;
        WinRunnable* const m_pRunnable;
        unsigned int m_ThreadID;
        std::string m_ThreadName;
        volatile bool m_bRun;
    };

}

#endif
