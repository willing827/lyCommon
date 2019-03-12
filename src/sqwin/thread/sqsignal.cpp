#include <sqwin/thread/sqsignal.h>
#include <Sddl.h>

namespace snqu{

    Signal::Signal()
    {
        m_signal = NULL;
    }

    Signal::~Signal()
    {
        Destory();
    }

    bool Signal::Create(const char* name)
    {
        SECURITY_ATTRIBUTES sa; //允许管理员访问
        sa.nLength=sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle=FALSE;
        LPCSTR szSD= "D:P(D;OICI;GA;;;BG)(A;OICI;GA;;;SY)(A;OICI;GA;;;BA)(A;OICI;GRGWGX;;;IU)";
        ConvertStringSecurityDescriptorToSecurityDescriptorA(szSD,SDDL_REVISION_1,&(
            sa.lpSecurityDescriptor),NULL);

        m_signal = ::CreateSemaphore(&sa, 1, 1, name);
        if (NULL == m_signal)
            return false;
        return true;
    }

    bool Signal::Open(const char* name)
    {
        m_signal = ::OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, name);
        if (NULL == m_signal)
            return false;
        return true;
    }

    int Signal::Lock(int second)
    {
        if (NULL != m_signal)
        {
            int ret = ::WaitForSingleObject(m_signal, INFINITE == second ? INFINITE : second*1000);
			return ret;
        }
        return -1;
    }

    void Signal::Unlock()
    {
        if (NULL != m_signal)
        {
            ::ReleaseSemaphore(m_signal, 1, NULL);
        }
    }

    void Signal::Destory()
    {
        if (NULL != m_signal)
        {
            Unlock();
            ::CloseHandle(m_signal);
            m_signal = NULL;
        }
    }

}