#include <sqwin/win/sqos.h>
#include <sqwin/sqwin.h>

namespace snqu { namespace os{

    bool IsWinXP()
    {
        OSVERSIONINFO osvi;
        bool bIsWindowsXPorLater = true;

        ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

        GetVersionEx(&osvi);
        if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion >= 1)
            bIsWindowsXPorLater = true;
        else if (osvi.dwMajorVersion >= 6 && osvi.dwMinorVersion >= 1)
            bIsWindowsXPorLater = false;
        return bIsWindowsXPorLater;
    }

    bool IsWow64()
    {
        typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
        LPFN_ISWOW64PROCESS fnIsWow64Process;
        BOOL bIsWow64 = FALSE;
        fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle("kernel32"), "IsWow64Process");
        if (NULL != fnIsWow64Process)
        {
            if (!fnIsWow64Process(GetCurrentProcess(), &bIsWow64))
                return false;
            else
                return true;
        }
        return bIsWow64 ? true : false;
    }

    bool SetTime(__int64 utc_time)
    {
        std::tm gm;
        ::localtime_s(&gm, &utc_time);
        SYSTEMTIME st = { WORD(1900 + gm.tm_year), WORD(1 + gm.tm_mon), WORD(gm.tm_wday), WORD(gm.tm_mday), WORD(gm.tm_hour), WORD(gm.tm_min), WORD(gm.tm_sec), 0 };
        if (TRUE != ::SetLocalTime(&st))
            return false;

        auto new_time = time(NULL);
        if (new_time > utc_time + 1 || new_time < utc_time - 1)
            return false;

        return true;
    }

    std::pair<std::string, Ms_Os_Types> get_os_versionex()
    {
        OSVERSIONINFOEX osvi;
        Ms_Os_Types vertype = MS_UNKNOWN;
        std::string verString("");

        ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

        BOOL bOsVersionInfoEx = FALSE;
        if (!(bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO *)&osvi)))
        {
            osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
            GetVersionEx((OSVERSIONINFO *)&osvi);
        }
        if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
        {
            verString = "Windows 2000";
            vertype = MS_WIN_2000;
        }
        else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1)
        {
            verString = "Windows XP";
            vertype = MS_WIN_XP;
        }
        else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2)
        {
            if (GetSystemMetrics(SM_SERVERR2) != 0)
            {
                verString = "Windows Server 2003 R2";
                vertype = MS_WIN_2003_R2;
            }
            else
            {
                verString = "Windows Server 2003";
                vertype = MS_WIN_2003;
            }
        }
        else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0)
        {
            if (osvi.wProductType == VER_NT_WORKSTATION)
            {
                verString = "Windows Vista";
                vertype = MS_WIN_VISTA;
            }
            else
            {
                verString = "Windows Server 2008";
                vertype = MS_WIN_2008;
            }
        }
        else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1)
        {
            if (osvi.wProductType == VER_NT_WORKSTATION)
            {
                verString = "Windows 7";
                vertype = MS_WIN_WIN7;
            }
            else
            {
                verString = "Windows Server 2008 R2";
                vertype = MS_WIN_2008_R2;
            }
        }
        else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 2)
        {
            if (osvi.wProductType == VER_NT_WORKSTATION)
            {
                verString = "Windows 8";
                vertype = MS_WIN_WIN8;
            }
            else
            {
                verString = "Windows Server 2012";
                vertype = MS_WIN_2012;
            }
                
        }
        else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 3)
        {
            if (osvi.wProductType == VER_NT_WORKSTATION)
            {
                verString = "Windows 8.1";
                vertype = MS_WIN_WIN8_1;
            }
            else
            {
                verString = "Windows Server 2012 R2";
                vertype = MS_WIN_2012_R2;
            }
        }
        else if (osvi.dwMajorVersion == 10 && osvi.dwMinorVersion == 0)
        {
            if (osvi.wProductType == VER_NT_WORKSTATION)
                verString = "Windows 10";
            else
            {
                verString = "Windows Server 2016";
                vertype = MS_WIN_2016;
            }
                
        }
        else
        {
            verString = "Microsoft Windows NT ";
        }

        if (osvi.dwMajorVersion >= 6)
        {
            if (IsWow64())
                verString += " x64";
        }

        return std::make_pair(verString, vertype);
    }

    std::string get_os_version()
    {
        return get_os_versionex().first;
    }

    Ms_Os_Types get_os_type()
    {
        return get_os_versionex().second;
    }

    std::string get_host_name()
    {
        std::string strname = "";
        char host_name[MAX_PATH] = { 0 };
        DWORD size = sizeof(host_name);
        GetComputerNameA(host_name, &size);
        strname = host_name;
        return strname;
    }
}}