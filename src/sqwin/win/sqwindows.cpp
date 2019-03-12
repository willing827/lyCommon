#include <sqwin/win/sqwindows.h>
#include <WinVer.h>
#include <time.h>
#include <objbase.h>
#include <io.h>
#include <TlHelp32.h>
#include <sqstd/sqtime.h>
#include <sqwin/sqwin.h>
#include <sqstd/sqfilerw.h>


#pragma comment(lib,"Version.lib")
#pragma comment(lib,"ws2_32.lib")

// 创建唯一标识
std::string create_guid()
{
    char buffer[64] = {0};
    GUID guid;

    if (CoCreateGuid(&guid))
    {
        fprintf(stderr, "create guid error\n");
        return std::string();
    }

    _snprintf(buffer, sizeof(buffer), 
        "%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X", 
        guid.Data1, guid.Data2, guid.Data3, 
        guid.Data4[0], guid.Data4[1], guid.Data4[2], 
        guid.Data4[3], guid.Data4[4], guid.Data4[5], 
        guid.Data4[6], guid.Data4[7]);

    return buffer;
}

std::string GetFileVersion(const std::string& path)
{
    DWORD dwSize = GetFileVersionInfoSizeA(path.c_str(), NULL);
    if(0 == dwSize) return "";
    char* pBlock = (char*)malloc(dwSize);
    BOOL b = GetFileVersionInfoA(path.c_str(), 0, dwSize, pBlock);

    std::string ret;
    if (b == TRUE)
    {
        UINT nlen1 = 0;

        VS_FIXEDFILEINFO *lpFfi; 
        if(VerQueryValueA(pBlock , TEXT("\\") , (LPVOID *)&lpFfi , &nlen1))
        {
            DWORD dwFileVersionMS = lpFfi->dwFileVersionMS; 
            DWORD dwFileVersionLS = lpFfi->dwFileVersionLS; 
            //printf( "Higher: %x\n" , dwFileVersionMS ); 
            //printf( "Lower: %x\n" , dwFileVersionLS ); 
            DWORD dwLeftMost = HIWORD(dwFileVersionMS); 
            DWORD dwSecondLeft = LOWORD(dwFileVersionMS); 
            DWORD dwSecondRight = HIWORD(dwFileVersionLS); 
            DWORD dwRightMost = LOWORD(dwFileVersionLS); 
            char tmp[256];
            sprintf(tmp, "%d.%d.%d.%d" , dwLeftMost, dwSecondLeft, 
                dwSecondRight, dwRightMost ); 
            ret = tmp;
        }
    }

    free(pBlock);
    return ret;
}

FileInfo GetFileInfo(const std::string& path)
{
	FileInfo ret_info;
	DWORD dwSize = GetFileVersionInfoSizeA(path.c_str(), NULL);
	if (0 == dwSize) return ret_info;
	char* pBlock = (char*)malloc(dwSize);
	BOOL b = GetFileVersionInfoA(path.c_str(), 0, dwSize, pBlock);

	if (b == TRUE)
	{
		struct LANGANDCODEPAGE {
			WORD wLanguage;
			WORD wCodePage;
		} *lpTranslate;

		UINT nlen1 = 0;
		if (!VerQueryValueA(pBlock, TEXT("\\VarFileInfo\\Translation"),
			(LPVOID*)&lpTranslate, &nlen1))
			return ret_info;

		char tmp[256] = {0};
		auto query_func = [&](const char* info_str)->std::string
		{
			sprintf(tmp, "\\StringFileInfo\\%04x%04x\\%s", 
						lpTranslate[0].wLanguage, lpTranslate[0].wCodePage, info_str);
			char *pValue = NULL;
			if (VerQueryValueA(pBlock, TEXT(tmp), (LPVOID*)&pValue, &nlen1))
			{
				return pValue;
			};

			return "";
		};

		ret_info.file_version_		= query_func("FileVersion");
		ret_info.product_version_	= query_func("ProductVersion");
		ret_info.product_name		= query_func("ProductName");
		ret_info.file_copy_right_	= query_func("LegalCopyright");
		ret_info.file_description_	= query_func("FileDescription");
	}

	free(pBlock);
	return ret_info;
}

bool UnPackRes(WORD wResID, const std::string& file_type, const std::string& path, HMODULE hd)
{
    DWORD res_size = 0;
    HRSRC hrsc = FindResourceA(hd, MAKEINTRESOURCE(wResID), file_type.c_str());
    if (NULL == hrsc)
        return false;

    HGLOBAL hgl = LoadResource(hd, hrsc);
    if (NULL == hrsc)
        return false;

    res_size = SizeofResource(hd, hrsc);
    if (0 == res_size)
        return false;

    return snqu::FileRW::OverWrite(path, (const char*)hgl, res_size);
}



namespace snqu{ namespace os{

time_t SysTmToCTime(const _SYSTEMTIME& st)
{
	struct tm gm = { st.wSecond, st.wMinute, st.wHour,
		st.wDay, st.wMonth - 1, st.wYear - 1900, st.wDayOfWeek, 0, 0 };
	return mktime(&gm);
}

std::string load_string(void *hInstance, unsigned int  resID)
{
	CHAR resStr[MAX_PATH] = { 0 };
	LoadStringA((HINSTANCE)hInstance, resID, resStr, sizeof(resStr));
	return std::string(resStr);
}

void for_each_process(std::function<bool(const std::string& proc_name, unsigned long pid, unsigned long ppid)> func)
{
	PROCESSENTRY32  stProcList = { 0 };
	HANDLE	hSnp = NULL;
	hSnp = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (NULL != hSnp)
	{
		stProcList.dwSize = sizeof(stProcList);
		if (Process32First(hSnp, &stProcList))
		{
			do {
				if (!func(stProcList.szExeFile, stProcList.th32ProcessID, stProcList.th32ParentProcessID))
					break;
			} while (Process32Next(hSnp, &stProcList));
		}
		CloseHandle(hSnp);
	}
}

HMODULE GetCurrModuleHandle()
{
    MEMORY_BASIC_INFORMATION info;
    ::VirtualQuery((LPCVOID)(&GetCurrModuleHandle), &info, sizeof(info));
    return (HMODULE)info.AllocationBase;
}

}}