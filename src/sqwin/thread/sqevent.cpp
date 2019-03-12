#include <sqwin/thread/sqevent.h>


namespace snqu{

    Event::Event()
        : m_handle(NULL)
        , m_is_open(false)
    {
    }

    Event::Event(HANDLE h)
        : m_handle(h)
    {}

    Event::~Event()
    {
        if (!m_is_open)
            Destory();
    }

    bool Event::Create()
    {
        return Create(NULL, false, false, NULL);
    }

    bool Event::Create(LPSECURITY_ATTRIBUTES event_attributes, bool manual_reset, bool initial_state, const char* name)
    {
        if (NULL == m_handle)
        {
            m_handle = CreateEventA(event_attributes, manual_reset ? TRUE : FALSE, initial_state ? TRUE : FALSE, name);
            if (NULL == m_handle)
            {
                return false;
            }
        }

        return true;
    }

	bool Event::Open(unsigned long access, const char *name)
	{
		if (NULL == m_handle)
		{
			m_handle = OpenEventA(access, FALSE, name);
			if (NULL == m_handle)
			{
				return false;
			}
            m_is_open = true;
		}

        return true;
	}

    bool Event::Wait(unsigned long wait_time)
    {
        if (NULL == m_handle)
        {
            return false;
        }
        DWORD ret = WaitForSingleObject(m_handle, wait_time);
        switch (ret)
        {
        case WAIT_FAILED:
            //printf("wait failed %d", GetLastError());
            break;
        case WAIT_ABANDONED:
            break;
        case WAIT_TIMEOUT:
            break;
        default:
            break;
        }
        return (WAIT_OBJECT_0 == ret);
    }

    bool Event::Set()
    {
        if (NULL == m_handle)
        {
            return false;
        }

        return FALSE != SetEvent(m_handle); 
    }

    bool Event::Reset()
    {
        if (NULL == m_handle)
        {
            return false;
        }

        return FALSE != ResetEvent(m_handle); 
    }

    bool Event::Destory()
    {
		bool rel = false;
        if (NULL != m_handle && !m_is_open)
        {
			rel = ::CloseHandle(m_handle) ? true : false;
			m_handle = NULL;
        }

        return rel;
    }
}