// AntiDebugee.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include <Windows.h>

/*
 #这里的代码来自某个程序的反调试模块，看到比较简单就写成了C源码

 #具体能不能反这个自己测试了，我只是写出来而已

 #have a fun! CopyRight reserved.
*/

 
__inline int 
Qemu_1()
{
	int		dwRet		=	0;
	DWORD	dwSize		=	1024;
	HKEY	hKey		=	NULL;
	CHAR	lpBuf[1024]	=	{0};

	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
		"HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 0\\Scsi Bus 0\\Target Id 0\\Logical Unit Id 0",
		0,
		KEY_READ,
		&hKey))
	{
		dwRet	=	1;
	} 
	else
	{
		if (RegQueryValueExA(hKey,
			"Identifier",
			0,
			0,
			(LPBYTE)lpBuf,
			&dwSize))
		{
			dwRet	=	1;
		}
		else
		{
			for (unsigned int i = 0; i < strlen(lpBuf); i++)	{
				lpBuf[i]	=	toupper(lpBuf[i]);
			}
			dwRet	=	strstr(lpBuf, "QEMU") == 0;
		}
	}

	return dwRet;
}

__inline int
Qemu_2()
{
	HKEY	hKey		=	NULL;
	DWORD	dwSize		=	1024;
	CHAR	lpBuf[1024]	=	{0};
	int 	dwRet		=	0;

	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
		"HARDWARE\\Description\\System",
		0,
		KEY_READ,
		&hKey))
	{
		dwRet	=	1;
	}
	else
	{
		if (RegQueryValueExA(hKey,
			"SystemBiosVersion",
			0,
			0,
			(LPBYTE)lpBuf,
			&dwSize))
		{
			dwRet	=	1;
		}
		else
		{
			for (unsigned int i = 0; i < strlen(lpBuf); i++) {
				lpBuf[i]	=	toupper(lpBuf[i]);
			}
			dwRet	=	strstr(lpBuf, "QEMU") == 0;
		}
	}

	return dwRet;
}

__inline
int
CheckQemu()
{
	int		dwResult	=	0;

	if (Qemu_1()) {
		dwResult	=	Qemu_2() != 0;
	} else {
		dwResult	=	0;
	}

	return dwResult;
}

__inline int
CheckSandBox_1()
{
	POINT	pt_1,pt_2;
	int		dwRet;

	GetCursorPos(&pt_1);
	Sleep(1000);
	GetCursorPos(&pt_2);
	
	if (pt_1.x == pt_2.x) {
		dwRet	=	pt_1.y != pt_2.y;
	} else {
		dwRet	=	1;
	}

	return dwRet;
}

__inline int
checkKeyWord()
{
	char	lpUser[200]	=	{0};
	BOOL	bFlag		=	TRUE;
	DWORD	len			=	200;

	GetUserNameA(lpUser, &len);

	for (unsigned int i = 0; i < strlen(lpUser); i++) {
		lpUser[i] = toupper(lpUser[i]);
	}

	if (strstr(lpUser, "SANBOX")) {
		bFlag	=	0;
	} 
	if (strstr(lpUser, "VIRUS")) {
		bFlag	=	0;
	} 
	if (strstr(lpUser, "MALWARE")) {
		bFlag	=	0;
	}

	return bFlag;
}

__inline int 
checkSandPath()
{
	char lpPath[500]	=	{0};
	bool bFlag			=	true;

	GetModuleFileNameA(NULL, lpPath, 500);

	for (unsigned int i = 0; i < strlen(lpPath); i++) {
		lpPath[i]	=	toupper(lpPath[i]);
	}

	if (strstr(lpPath, "\\SAMPLE")) {
		bFlag	=	0;
	}
	if (strstr(lpPath, "\\VIRUS")) {
		bFlag	=	0;
	}
	if (strstr(lpPath, "SANDBOX")) {
		bFlag	=	0;
	}

	return bFlag;
}

__inline
int 
CheckSandBox()
{
	bool bFlag;

	if (CheckSandBox_1()) {
		if (checkKeyWord()) {
			if (checkSandPath()) {
				bFlag	=	1;
			}
		} else {
			bFlag	=	0;
		}
	} else {
		bFlag	=	0;
	}

	return bFlag;
}

__inline int
checkSandBoxieImageBase()
{
	int bFlag;

	if (GetModuleHandleA("sbiedll.dll")) {
		bFlag	=	0;
	} else {
		bFlag	=	1;
	}

	return bFlag;
}

__inline
int 
checkSandBoxie()
{
	if (checkSandBoxieImageBase()) {
		return 1;
	} else {
		return 0;
	}
}

__inline int
apiCheck()
{
	return IsDebuggerPresent() == 0;
}

__inline
int
checkDebugger()
{
	if (apiCheck()) {
		return 1;
	} else {
		return 0;
	}
}

__inline int 
checkVboxPartOne()
{
	char	lpBuffer[1024]	=	{0};
	HKEY	hKey;
	bool	bFlag			=	true;
	DWORD	len				=	1024;

	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
		"HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 0\\Scsi Bus 0\\Target Id 0\\Logical Unit Id 0",
		0,
		KEY_READ,
		&hKey))
	{
		bFlag	=	1;
	}

	if (RegQueryValueExA(hKey,
		"Identifier",
		0,
		0,
		(LPBYTE)lpBuffer,
		&len))
	{
		bFlag	=	1;
	}

	for (unsigned int i = 0; i < strlen(lpBuffer); i++)	{
		lpBuffer[i]	=	toupper(lpBuffer[i]);
	}
	if (strstr(lpBuffer, "VBOX")) {
		bFlag	=	0;
	} else {
		bFlag	=	1;
	}

	return bFlag;
}

__inline int 
checkVboxPartTwo()
{
	bool	bFlag			=	true;
	char	lpBuffer[1024]	=	{0};
	DWORD	dwLen			=	1024;
	HKEY	hKey;

	if (RegOpenKeyExA(
		HKEY_LOCAL_MACHINE,
		"HARDWARE\\Description\\System",
		0,
		KEY_READ,
		&hKey))
	{
		bFlag	=	1;
	}

	if (RegQueryValueExA(hKey,
		"SystemBiosVersion",
		0,
		0,
		LPBYTE(lpBuffer),
		&dwLen))
	{
		bFlag	=	1;
	}

	for (unsigned int i=0; i < strlen(lpBuffer); i++)
	{
		lpBuffer[i]	=	toupper(lpBuffer[i]);
	}

	if (strstr(lpBuffer, "VBOX")) {
		bFlag	=	0;
	} else {
		bFlag	=	1;
	}

	return bFlag;
}

__inline int
checkVboxPartThree()
{
	HKEY hKey;

	return RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Oracle\\VirtualBox Guest Additions", 0, KEY_READ,&hKey) != 0;
}

__inline int 
checkVboxPartFour()
{
	return GetFileAttributesA("C:\\WINDOWS\\system32\\drivers\\VBoxMouse.sys") == -1;
}

__inline
int 
checkVbox()
{
	bool bFlag	=	1;

	if (checkVboxPartOne()) {
		if (checkVboxPartTwo()) {
			if (checkVboxPartThree()) {
				if (checkVboxPartFour()) {
					bFlag	=	1;
				}
			} else {
				bFlag	=	0;
			}
		} else {
			bFlag	=	0;
		}
	} else {
		bFlag	=	0;
	}

	return bFlag;

}

__inline int 
checkVMarePartOne()
{
	bool	bFlag			=	true;
	char	lpBuffer[1024]	=	{0};
	DWORD	dwLen			=	1024;
	HKEY	hKey;

	if (RegOpenKeyExA(
		HKEY_LOCAL_MACHINE,
		"HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 0\\Scsi Bus 0\\Target Id 0\\Logical Unit Id 0",
		0,
		KEY_READ,
		&hKey))
	{
		bFlag	=	1;
	}

	if (RegQueryValueExA(hKey,
		"Identifier",
		0,
		0,
		LPBYTE(lpBuffer),
		&dwLen))
	{
		bFlag	=	1;
	}

	for (unsigned int i=0; i < strlen(lpBuffer); i++)
	{
		lpBuffer[i]	=	toupper(lpBuffer[i]);
	}

	if (strstr(lpBuffer, "VMWARE")) {
		bFlag	=	0;
	} else {
		bFlag	=	1;
	}

	return bFlag;
}

__inline int 
checkVMarePartTwo()
{
	HKEY hKey;
	return RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\VMware, Inc.\\VMware Tools", 0, KEY_READ, &hKey) != 0;
}

__inline int
checkVMarePartThree()
{
	return GetFileAttributesA("C:\\WINDOWS\\system32\\drivers\\vmmouse.sys") == -1;
}

__inline int 
checkVMarePartFour()
{
	return GetFileAttributesA("C:\\WINDOWS\\system32\\drivers\\vmhgfs.sys") == -1;
}


__inline
int 
checkVMare()
{
	bool bFlag	=	true;

	if (checkVMarePartOne()) {
		if (checkVMarePartTwo()) {
			if (checkVMarePartThree()) {
				if (checkVMarePartFour()) {
					bFlag	=	1;
				} else {
					bFlag	=	0;
				}
			} else {
				bFlag	=	0;
			}
		} else {
			bFlag	=	0;
		}
	} else {
		bFlag	=	0;
	}

	return bFlag;
}

__inline int 
checkWineApi()
{
	HMODULE hMod;
	PVOID	p;
	bool	bFlag;

	hMod	=	GetModuleHandleA("kernel32.dll");

	if (hMod) {
		p	=	GetProcAddress(hMod, "wine_get_unix_file_name");
		if (p)
			bFlag	=	0;
		else
			bFlag	=	1;
	} else {
		bFlag	=	1;
	}

	return bFlag;
}

__inline
int
checkWine()
{
	return checkWineApi() != 0;
}

