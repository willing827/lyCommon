#include <sqwin/thread/sqmutex.h>
#include <Sddl.h>

#pragma comment(lib, "Advapi32.lib")

using namespace std;

namespace snqu{

    Mutex::Mutex()
        : m_mutex(NULL)
    {}

    Mutex::Mutex(HANDLE h)
        : m_mutex(h)
    {}

    Mutex::Mutex(const char* name)
    {
        Create(name);
    }

    Mutex::~Mutex()
    {
        Destroy();
    }

    bool Mutex::Create(const char* name)
    {
        SECURITY_ATTRIBUTES sa; //允许管理员访问
        sa.nLength=sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle=FALSE;
        LPCSTR szSD= "D:P(D;OICI;GA;;;BG)(A;OICI;GA;;;SY)(A;OICI;GA;;;BA)(A;OICI;GRGWGX;;;IU)";
        ConvertStringSecurityDescriptorToSecurityDescriptorA(szSD,SDDL_REVISION_1,&(
            sa.lpSecurityDescriptor),NULL);

        m_mutex = ::CreateMutexA(&sa, FALSE, name);
        if (NULL == m_mutex)
        {
            return false;
        }

        if (GetLastError() == ERROR_ALREADY_EXISTS)		
        {
            Unlock();
            return false;
        }

        return true;
    }

    void Mutex::Destroy()
    {
        if (NULL != m_mutex)
        {
            ::CloseHandle(m_mutex);
            m_mutex = NULL;
        }
    }

    void Mutex::Unlock()
    {
        if (NULL != m_mutex)
        {
            ::ReleaseMutex(m_mutex);
            m_mutex = NULL;
        }
    }

	bool Mutex::Open(unsigned long access, const char *name)
	{
		if (NULL == m_mutex)
		{
			m_mutex = OpenMutexA(access, FALSE, name);
			if (NULL == m_mutex)
			{
				return false;
			}
		}

        return true;
	}

    bool Mutex::Lock(unsigned long wait_time)
    {
        if (NULL == m_mutex)
        {
            return false;
        }

        return (WAIT_OBJECT_0 == WaitForSingleObject(m_mutex, wait_time));
    }

}