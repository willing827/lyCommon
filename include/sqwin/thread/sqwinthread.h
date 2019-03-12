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
        ��ʼ�����߳�
        @arg bSuspend ��ʼ����ʱ�Ƿ����
        **/
        bool Start(bool bSuspend = false);

        /**
        ���е��̺߳���������ʹ����������д�˺���
        **/
        virtual void Run();

        /**
        ��ǰִ�д˺����̵߳ȴ��߳̽���
        @arg timeout �ȴ���ʱʱ�䣬���Ϊ�������ȴ�����ʱ��
		@ret ��ʱ���߳�����false
        **/
		bool Join(int timeout = -1);
        /**
        �ָ�������߳�
        **/
        void Resume();
        /**
        �����߳�
        **/
        void Suspend();
        /**
        ��ֹ�̵߳�ִ��
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
