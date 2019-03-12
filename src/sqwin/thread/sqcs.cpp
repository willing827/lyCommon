#include <sqwin/thread/sqcs.h>
#include <Sddl.h>

namespace snqu {

    CriticalSection::CriticalSection(void)
    {
        Create();
    }

    CriticalSection::~CriticalSection()
    {
        Destory();
    }

    void CriticalSection::Lock()
    {
        ::EnterCriticalSection(&m_lock);
    }

    void CriticalSection::Unlock()
    {
        ::LeaveCriticalSection(&m_lock);
    }

    void CriticalSection::Create()
    {
        ::InitializeCriticalSection(&m_lock);
    }

    void CriticalSection::Destory()
    {
        ::DeleteCriticalSection(&m_lock);
    }

}