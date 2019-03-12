#pragma once
#include <stdio.h>
#include <string>

using namespace std;
#include <tchar.h>

#include "security.h"
#include "BugTrap.h"

#include <DbgHelp.h>

extern "C"
{
	typedef LPTOP_LEVEL_EXCEPTION_FILTER (APIENTRY *PFNBT_InstallSehFilter) (void);

	typedef PCTSTR (APIENTRY* PFNBT_GetAppName)();

	typedef void (APIENTRY* PFNBT_SetAppName)(PCWSTR pszAppName);

	typedef void (APIENTRY* PFNBT_SetFlags)(DWORD dwFlags);

	typedef void (APIENTRY* PFNBT_SetActivityType)(BUGTRAP_ACTIVITY eActivityType);

	typedef void (APIENTRY* PFNBT_SetReportFilePath)(LPCWSTR pszReportFilePath);

	typedef void (APIENTRY* PFNBT_SetSupportServer)(LPCWSTR pszSupportHost, SHORT nSupportPort);

	typedef void (APIENTRY* PFNBT_AddLogFile)(LPCWSTR pszLogFile);

	typedef void (APIENTRY* PFNBT_SetDumpType)(DWORD dwDumpType);
}

__inline HMODULE LoadBugTrapDll()
{
	WCHAR szMyPath[MAX_PATH];
	GetModuleFileNameW(NULL,szMyPath,MAX_PATH);
	PathRemoveFileSpecW(szMyPath);
	PathAppendW(szMyPath,L"BugTrap.dll");

#if 0
	if(!CheckMySelf(szMyPath))
	{
		return FALSE;
	}
#endif

	HMODULE hModule=LoadLibraryExW(szMyPath,NULL,LOAD_WITH_ALTERED_SEARCH_PATH);
	if(!hModule)
		return NULL;

	return hModule;
}

__inline VOID SetBugtrapAppName(LPCWSTR lpAppName)
{
	HMODULE hModule=LoadBugTrapDll();
	if (!hModule)
		return;

	PFNBT_SetAppName pfnBT_SetAppName=(PFNBT_SetAppName)GetProcAddress(hModule,MAKEINTRESOURCEA(2));
	PFNBT_GetAppName pfnBT_GetAppName = (PFNBT_GetAppName)GetProcAddress(hModule,MAKEINTRESOURCEA(1));
	
	if (pfnBT_SetAppName != NULL)
	{
		pfnBT_SetAppName(lpAppName);
		OutputDebugStringW(lpAppName);
	}

	if (pfnBT_GetAppName != NULL)
	{
		OutputDebugStringW((LPCWSTR)pfnBT_GetAppName());
	}
}

__inline BOOL SetupExceptionHandler(LPCWSTR pszLogFileName,LPCWSTR lpAppName, LPCWSTR lpPostUrl,
									BUGTRAP_ACTIVITY eActivityType=BTA_SHOWUI)
{
	HMODULE hModule=LoadBugTrapDll();
	if(!hModule)
		return FALSE;

	//	BT_InstallSehFilter @120 NONAME
	// BT_SetFlags @41 NONAME
	// BT_SetActivityType @51 NONAME
	// BT_SetReportFilePath @55 NONAME
	//	BT_SetSupportServer @34 NONAME
	//	BT_SetAppName @2 NONAME
	PFNBT_InstallSehFilter pfnBT_InstallSehFilter=(PFNBT_InstallSehFilter)GetProcAddress(hModule,MAKEINTRESOURCEA(120));
	PFNBT_SetFlags pfnBT_SetFlags =(PFNBT_SetFlags)GetProcAddress(hModule,MAKEINTRESOURCEA(41));
	PFNBT_SetActivityType pfnBT_SetActivityType=(PFNBT_SetActivityType)GetProcAddress(hModule,MAKEINTRESOURCEA(51));
	PFNBT_SetReportFilePath pfnBT_SetReportFilePath=(PFNBT_SetReportFilePath)GetProcAddress(hModule,MAKEINTRESOURCEA(55));
	PFNBT_SetSupportServer pfnBT_SetSupportServer=(PFNBT_SetSupportServer)GetProcAddress(hModule,MAKEINTRESOURCEA(34));
	PFNBT_SetAppName pfnBT_SetAppName=(PFNBT_SetAppName)GetProcAddress(hModule,MAKEINTRESOURCEA(2));
	PFNBT_GetAppName pfnBT_GetAppName = (PFNBT_GetAppName)GetProcAddress(hModule,MAKEINTRESOURCEA(1));

	//BT_SetDumpType @43 NONAME
	PFNBT_SetDumpType pfnBT_SetDumpType=(PFNBT_SetDumpType)GetProcAddress(hModule,MAKEINTRESOURCEA(43));

	// Setup exception handler

	if(!pfnBT_InstallSehFilter || !pfnBT_SetFlags || !pfnBT_SetActivityType || !pfnBT_SetSupportServer || !pfnBT_SetAppName 
		|| !pfnBT_SetDumpType)
	{
		return FALSE;
	}

	pfnBT_SetFlags(BTF_DETAILEDMODE | BTF_ATTACHREPORT);
	pfnBT_SetActivityType(eActivityType);
	pfnBT_SetAppName(lpAppName);
	pfnBT_SetDumpType(MiniDumpWithDataSegs);

	//展示了UI就没必要用下面的接口
	//pfnBT_SetReportFilePath(pszLogFileName);

	pfnBT_SetSupportServer(lpPostUrl, 9999);
	pfnBT_InstallSehFilter();

	return TRUE;
}

#ifdef Compatibility_Mode_Need_
__inline BOOL 
	IsCompatibilityModeDetection()
{
	TCHAR szDir[MAX_PATH];
	GetSystemDirectory(szDir,MAX_PATH);
	PathAppend(szDir,_T("NTDLL.DLL"));
	wstring szVer;
	if(!rt::os::get_file_version(szDir,szVer))
	{
		return FALSE;
	}

	OSVERSIONINFOEX OS={0};
	OS.dwOSVersionInfoSize=sizeof(OS);
	if(!GetVersionEx((OSVERSIONINFO*)&OS))
	{
		return FALSE;
	}

	WCHAR strOsVer[100];
	swprintf_s(strOsVer,_T("%d.%d"), 
		OS.dwMajorVersion,OS.dwMinorVersion);

	if(!memcmp(strOsVer,szVer.c_str(),sizeof(WCHAR)*wcslen(strOsVer)))
	{
		return FALSE;
	}

	//TCHAR
	//L"6.3.9600.16384"


	return TRUE;

}
#endif


typedef VOID
	(NTAPI *PFNDbgBreakPoint)(
	VOID 
	);

__inline BOOL IsSelfModified(HMODULE module_handle)
{
	BOOL is_debugger = IsDebuggerPresent();
	if(!is_debugger)
	{
		__try
		{
			HMODULE ntdll_module = LoadLibrary(_T("ntdll.dll"));
			if(!ntdll_module)
			{
				return TRUE;
			}

			PFNDbgBreakPoint pfnDbgBreakPoint=(PFNDbgBreakPoint)GetProcAddress(ntdll_module,"DbgBreakPoint");

			if(!pfnDbgBreakPoint)
				return TRUE;

			pfnDbgBreakPoint();
			is_debugger=TRUE;
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{

		}
	}


#ifndef _DEBUG
	WCHAR szMyPath[MAX_PATH];
	GetModuleFileNameW(module_handle,szMyPath,MAX_PATH);
	if(!CheckMySelf(szMyPath,!is_debugger))
	{
		return TRUE;
	}

#endif //_DEBUG

	return FALSE;
}

__inline BOOL InstallSEH(LPCWSTR lpAppName,LPCWSTR lpLogDirName, LPCWSTR lpPostUrl, BUGTRAP_ACTIVITY eActivityType=BTA_SHOWUI)
{
	WCHAR szMyPath[MAX_PATH];
	GetModuleFileNameW(NULL,szMyPath,MAX_PATH);
	PathRemoveFileSpecW(szMyPath);
	if (lpLogDirName != NULL)
		PathAppendW(szMyPath, lpLogDirName);
	return SetupExceptionHandler(NULL,lpAppName, lpPostUrl,eActivityType);
}


__inline BOOL AddSEHCustomLog(LPCWSTR szFileFullPath)
{
	HMODULE hModule=LoadBugTrapDll();
	if(!hModule)
		return FALSE;

	//BT_AddLogFile @57 NONAME
	PFNBT_AddLogFile pfnBT_AddLogFile=(PFNBT_AddLogFile)GetProcAddress(hModule,MAKEINTRESOURCEA(57));
	if(!pfnBT_AddLogFile)
	{
		return FALSE;
	}

	pfnBT_AddLogFile(szFileFullPath);

	return TRUE;
}


