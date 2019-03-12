/*---------------------------------------------------------------------------*/
/*  sqtools.h                                                                */
/*                                                                           */
/*  History                                                                  */
/*      05/30/2015                                                           */
/*                                                                           */
/*  Author                                                                   */
/*      GUO LEI																 */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*---------------------------------------------------------------------------*/
#ifndef __SQTOOLS_H__
#define __SQTOOLS_H__

#include <sqstd/sqtypes.h>
#include <sqwin/sqwin.h>

namespace snqu {

int SQRandIntervalNum(int index, int min, int max);

VOID 
WINAPI
SimpleEncryptData(
	__out LPBYTE out, 
	__in LPBYTE in, 
	__in int len
	);

DWORD 
WINAPI 
GetPathFileSize(
	IN char *path
	);


boolean 
WINAPI
ReadPathFileBuffer(
	IN char *path, 
	OUT PBYTE Buffer,
	IN ULONG Size
	);


boolean 
WINAPI 
WritePathFileBuffer(
	IN char *path, 
	IN PBYTE Buffer,
	IN ULONG Size,
	IN DWORD dwCreationFlag,
	IN boolean fAppend
	);


VOID 
SNQUSimpleCryptData(
	__in LPBYTE lpHashKey,
	__in INT nKeySize,
	__inout LPBYTE lpDataBuffer,
	__in INT nDataSize
	);


boolean
WINAPI
LoadFileToBuffer(
	__in LPSTR lpszFilePath,
	__inout LPVOID *lppBuff,
	__in DWORD *pdwSize
	);



BOOL
WINAPI
WinExecProcessW(
	__in LPWSTR  lpCommandLine,
	__in INT	 nShow
	);

boolean
WINAPI
EnableShutdownPrivilege(
	IN boolean bEnable
	);


boolean
WINAPI
EnablePrivilege(
	IN boolean bEnable,
	IN LPCSTR Privilege
	);


BOOL
WINAPI
WinExecProcessExA(
	__in LPSTR  lpAppName,
	__in LPSTR  lpCommandLine,
	__in INT	nShow,
	__in LPSTR  lpDeskName,
	__out PPROCESS_INFORMATION pi
	);

BOOL
WINAPI
WinExecProcessExW(
	__in LPWSTR  lpAppName,
	__in LPWSTR  lpCommandLine,
	__in INT	 nShow,
	__in LPWSTR  lpDeskName,
	__out PPROCESS_INFORMATION pi
	);

boolean
WINAPI
EnablePrivilege(
	IN boolean bEnable,
	IN LPCSTR Privilege
	);


INT64
WINAPI
GetLocalTime64(
	);

DWORD
WINAPI
GetLocalTime32(
	);

BOOL
WINAPI
IsProcessExisted(
	__in LPSTR lpProcess
	);

DWORD
WINAPI
GetProcessIdByNameA(
	__in LPSTR lpProcName
	);

DWORD
WINAPI
GetProcessIdByNameW( 
	__in LPWSTR lpProcName 
	);


DWORD  
WINAPI  
GetProcessThreadIdByNameW(
	__in LPWSTR lpProcName
	);


DWORD  
WINAPI  
GetProcessThreadIdByNameA(
	__in LPSTR lpProcName
	);

BOOL 
WINAPI
GetProcessFullPath(
	__in DWORD dwPID, 
	__inout LPSTR pszFullPath,
	__in UINT nSizePath
	);

BOOL
WINAPI
LaunchDefBrowserWithParameterW(
	__in LPWSTR lpUrl
	);


BOOL
WINAPI
WinExecByCurrentExplorerTokenW(
	__in LPWSTR  lpCommandLine,
	__in INT	 nShow
	);

BOOL WINAPI  ResourecToFileW(
	HMODULE hModule,
	LPWSTR lpType,
	LPWSTR lpResName,
	LPWSTR lpFilePath
	);

BOOL WINAPI  ResourecToFileA(
	HMODULE hModule,
	LPSTR lpType,
	LPSTR lpResName,
	LPSTR lpFilePath
	);

BOOL
WINAPI
DisableWow64FsRedirection(
    PVOID* OldValue
	);

BOOL 
WINAPI
RevertWow64FsRedirection(
    PVOID OldValue
	);

BOOLEAN 
NtPathToDosPathA(
	CHAR *FullNtPath, 
	CHAR *FullDosPath
	);

BOOLEAN 
DosPathToNtPathA(
	CHAR *FullDosPath, 
	CHAR *FullNtPath
	);

BOOLEAN 
DosPathToNtPathW(
	WCHAR *FullDosPath, 
	WCHAR *FullNtPath
	);

BOOLEAN
NtPathToDosPathW(
	WCHAR *FullNtPath,
	WCHAR *FullDosPath
	);

}
#endif //__SQTOOLS_H__