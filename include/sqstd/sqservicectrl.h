/*++

Copyright (c) 2013/01/22

Module Name:

	servicectl.h

Abstract:
	服务控制相关
--*/

#ifndef __SERVICECTL_H__
#define __SERVICECTL_H__


#include <sqstd/sqinc.h>

namespace snqu {
class WinServiceControl
{
public:
	WinServiceControl();
	virtual ~WinServiceControl();

private:
	SC_HANDLE m_hService;	
	SC_HANDLE m_hSCManager;

private:
	SC_HANDLE	GetSCMHandle();
	BOOL		CloseSCMHandle();
	SC_HANDLE   GetServiceHandle();

public:
	BOOL		Close();
	BOOL		Delete();
	BOOL		Open(LPSTR  lpServiceName, DWORD  dwDesiredAccess);

	BOOL		Create(
		LPSTR lpServiceName ,
		DWORD  dwServiceType ,
		DWORD  dwStartType,
		LPSTR lpBinaryPathName,
		LPSTR lpLoadOrderGroup,
		DWORD  dwDesiredAccess  = SERVICE_ALL_ACCESS,
		LPSTR lpDisplayName	= NULL
		);


	BOOL		Start(
		DWORD  dwNumServiceArgs,   
		LPCSTR *lpServiceArgVectors 
		);


	BOOL Stop();

	BOOL		Control(
		DWORD dwControl,    
		LPSERVICE_STATUS lpServiceStatus 
		);


	BOOL		ChangeConfig(
		DWORD dwServiceType,       
		DWORD dwStartType,        
		DWORD dwErrorControl,      
		LPSTR lpBinaryPathName, 
		LPSTR lpLoadOrderGroup,  
		LPDWORD lpdwTagId,       
		LPSTR lpDependencies,   
		LPSTR lpServiceStartName, 
		LPSTR lpPassword,   
		LPSTR lpDisplayName  = NULL
		);

	BOOL QueryConfig(LPQUERY_SERVICE_CONFIG *lpServiceConfig);

	BOOL QueryStatus(LPSERVICE_STATUS  lpServiceStatus);
};

BOOL 
WINAPI
QueryServiceBinaryPath(
	__in LPSTR lpServiceName,
	__out LPSTR lpPath,
	__in UINT nSize
	);

BOOL
WINAPI
StartWinService(
	__in LPSTR lpServiceName
	);

BOOL
WINAPI
StopWinService(
	__in LPSTR lpServiceName
	);

BOOL
WINAPI
CreateAndStartServiceW(
	__in LPWSTR lpwszServiceName,
	__in LPWSTR lpwszBinPath,
	__in LPWSTR lpwszServiceDesc,
	__in BOOL bNeedStart
	);


BOOL
WINAPI
CreateAndStartServiceA(
	__in LPSTR lpszServiceName,
	__in LPSTR lpszBinPath,
	__in LPSTR lpszServiceDesc,
	__in DWORD ServiceType,
	__in DWORD StartType,
	__in BOOL bNeedStart
	);

BOOL
WINAPI
CreateAndStartDriverW(
	__in LPWSTR lpwszServiceName,
	__in LPWSTR lpwszBinPath
	);

BOOL
WINAPI
CreateAndStartDriverA(
	__in LPSTR lpszServiceName,
	__in LPSTR lpszBinPath
	);
}

#endif //__SERVICECTL_H__