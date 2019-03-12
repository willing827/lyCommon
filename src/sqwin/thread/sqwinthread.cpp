#include <sqwin/thread/sqwinthread.h>
#include <sqwin/sqwin.h>

namespace snqu {

    WinThread::WinThread(void) :
        m_pRunnable(NULL),
        m_bRun(false)
    {
    }

    WinThread::~WinThread(void)
    {
    }

    WinThread::WinThread(WinRunnable * pRunnable) :
        m_ThreadName(""),
        m_pRunnable(pRunnable),
        m_bRun(false)
    {
    }

    WinThread::WinThread(const char * ThreadName, WinRunnable * pRunnable) :
        m_ThreadName(ThreadName),
        m_pRunnable(pRunnable),
        m_bRun(false)
    {
    }

    WinThread::WinThread(std::string ThreadName, WinRunnable * pRunnable) :
        m_ThreadName(ThreadName),
        m_pRunnable(pRunnable),
        m_bRun(false)
    {
    }

    bool WinThread::Start(bool bSuspend)
    {
        if (m_bRun)
        {
            return true;
        }
        if (bSuspend)
        {
            m_handle = (HANDLE)_beginthreadex(NULL, 0, StaticThreadFunc, this, CREATE_SUSPENDED, &m_ThreadID);
        }
        else
        {
            m_handle = (HANDLE)_beginthreadex(NULL, 0, StaticThreadFunc, this, 0, &m_ThreadID);
        }
        m_bRun = (NULL != m_handle);
        return m_bRun;
    }

    void WinThread::Run()
    {
        if (!m_bRun)
        {
            return;
        }
        if (NULL != m_pRunnable)
        {
            m_pRunnable->Run();
        }
        m_bRun = false;
    }

    bool WinThread::Join(int timeout)
    {
        if (NULL == m_handle || !m_bRun)
        {
            return true;
        }
        if (timeout <= 0)
        {
            timeout = INFINITE;
        }
		return WAIT_OBJECT_0 == ::WaitForSingleObject(m_handle, timeout);
    }

    void WinThread::Resume()
    {
        if (NULL == m_handle || !m_bRun)
        {
            return;
        }
        ::ResumeThread(m_handle);
    }

    void WinThread::Suspend()
    {
        if (NULL == m_handle || !m_bRun)
        {
            return;
        }
        ::SuspendThread(m_handle);
    }

    bool WinThread::Terminate(unsigned long ExitCode)
    {
        if (NULL == m_handle || !m_bRun)
        {
            return true;
        }
        if (::TerminateThread(m_handle, ExitCode))
        {
            ::CloseHandle(m_handle);
            return true;
        }
        return false;
    }

    unsigned int WinThread::GetThreadID()
    {
        return m_ThreadID;
    }

    std::string WinThread::GetThreadName()
    {
        return m_ThreadName;
    }

    void WinThread::SetThreadName(std::string ThreadName)
    {
        m_ThreadName = ThreadName;
    }

    void WinThread::SetThreadName(const char * ThreadName)
    {
        if (NULL == ThreadName)
        {
            m_ThreadName = "";
        }
        else
        {
            m_ThreadName = ThreadName;
        }
    }

    unsigned int WinThread::StaticThreadFunc(void * arg)
    {
        WinThread * pThread = (WinThread *)arg;
        pThread->Run();
        return 0;
    }

}