#include <sqwin/sqwin.h>
#include <sqwin/win/sqwinsock.h>

namespace snqu{ namespace net{

WinSockInit::WinSockInit()
    : m_is_loaded(false)
    , err(0)
{
}
  
WinSockInit::~WinSockInit()  
{  
    uninit();
}  

WinSockInit& WinSockInit::instance()
{
    static WinSockInit g_winsock;
    return g_winsock;
}
  
bool WinSockInit::init(unsigned short version)
{  
	if (m_is_loaded)
		return true;

    m_is_loaded = false;  
  
    WSADATA data;  
    int res = ::WSAStartup(version, &data);  
    if (res == 0)  
    {  
        m_is_loaded = true;  
        m_use_version = data.wVersion;  
    }  
    else  
    {  
        err = GetLastError();
        return false;
    }

    return true;
} 

bool WinSockInit::init(BYTE major, BYTE minor)
{
	auto version = MAKEWORD(major, minor);
	return init(version);
}

void WinSockInit::uninit()
{
    if (m_is_loaded)  
    {  
        ::WSACleanup();
        m_is_loaded = false;
    }  
}

}}