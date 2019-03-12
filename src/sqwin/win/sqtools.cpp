#include <sqwin/sqwin.h>
#include <sqwin/win/sqtools.h>
#include <tlhelp32.h>
#include <userenv.h>
#include <Psapi.h>
#include <string>
#include <vmp/sqvmsdk.h>
#include <IPTypes.h>
#include <Shlwapi.h>
#include <ShellAPI.h>


#pragma comment(lib,"Userenv.lib")
#pragma comment(lib,"kernel32.lib")
#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"Psapi.lib")
#pragma comment(lib,"Shell32.lib")


using namespace std;
#define MAX_COMMAND_LINE_BUFFER 2048
namespace snqu {
int32 Double2Int(double value)
{
	if (value > 0)
	{
		return (int32)(value+0.5);
	}
	else
	{
		return (int32)(value-0.5);
	}
}


unsigned int __myseed;
void sqsrand(unsigned int s)
{
	__myseed = s;
}


unsigned int sqrand()
{
	return (__myseed = (137*__myseed + 11) % 0x7FFF);
}

int SQRandIntervalNum(int index, int min, int max)
{
	if (min == max)
	{
		return min;
	}

	uint32 seedbase = 0;

#ifdef _WIN32
	LARGE_INTEGER seed;
	QueryPerformanceFrequency(&seed);
	QueryPerformanceCounter(&seed);
	seedbase = (uint32)seed.QuadPart;
#else
	seedbase = time(NULL);
#endif

	//sqsrand((unsigned int)seedbase + index);
	//int rd =sqrand() + 3;
	srand(seedbase + index);
	int rd = rand();
	return rd % (max - min + 1) + min;
}

BOOL WINAPI SNIsWow64()
{
	BOOL bIsWow64 = FALSE;
	typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

	LPFN_ISWOW64PROCESS fnIsWow64Process;
	fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
		GetModuleHandle(TEXT("kernel32")),"IsWow64Process");

	if (NULL != fnIsWow64Process)
	{
		if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
		{
			// handle error
		}
	}
	return bIsWow64;
}

DWORD 
WINAPI 
GetPathFileSize(
	IN char *path
	)
{
	DWORD  dwSize = 0;
	char *pszPath = NULL;
	char   szPath[MAX_PATH] = {0};
	char *ptr = NULL;

	if (path[0] == '\\')
	{
		if ((ptr = strrchr(path, '\\')) != NULL)
		{
			GetSystemDirectoryA(szPath, sizeof(szPath));
			lstrcatA(szPath, ptr);
			pszPath = szPath;
		}
	}
	else
	{
		pszPath = path;
	}

	HANDLE hFile = CreateFileA(pszPath, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		return 0;
	}

	dwSize = GetFileSize(hFile, NULL);
	CloseHandle(hFile);
	return dwSize;
}


boolean 
WINAPI
ReadPathFileBuffer(
	IN char *path, 
	OUT PBYTE Buffer,
	IN ULONG Size
	)
{
	HANDLE hFile = NULL;
	DWORD  NumberOfBytesRead = 0;
	char *pszPath = NULL;
	char   szPath[MAX_PATH] = {0};
	char *ptr = NULL;

	if (path[0] == '\\')
	{
		if ((ptr = strrchr(path, '\\')) != NULL)
		{
			GetSystemDirectoryA(szPath, sizeof(szPath));
			lstrcatA(szPath, ptr);
			pszPath = szPath;
		}
	}
	else
	{
		pszPath = path;
	}


	hFile = CreateFileA(pszPath, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		return FALSE;
	}

	if (!ReadFile(hFile, Buffer, Size, &NumberOfBytesRead, NULL))
	{
		CloseHandle(hFile);
		return FALSE;
	}

	CloseHandle(hFile);
	return TRUE;
}


boolean 
WINAPI 
WritePathFileBuffer(
	IN char *path, 
	IN PBYTE Buffer,
	IN ULONG Size,
	IN DWORD dwCreationFlag,
	IN boolean fAppend
	)
{
	HANDLE hFile = NULL;
	char   *ptr = NULL;
	char   *pszPath = NULL;
	char   szPath[MAX_PATH] = {0};
	DWORD  NumberOfBytesWrite = 0;
	DWORD  dwSize = 0;

	if (path[0] == '\\')
	{
		if ((ptr = strrchr(path, '\\')) != NULL)
		{
			GetSystemDirectoryA(szPath, sizeof(szPath));
			lstrcatA(szPath, ptr);
			pszPath = szPath;
		}
	}
	else
	{
		pszPath = path;
	}

	hFile = 
		CreateFileA(pszPath, GENERIC_READ | GENERIC_WRITE, 
		FILE_SHARE_READ | FILE_SHARE_WRITE, 
		NULL,
		dwCreationFlag, 
		0, 
		NULL
		);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		return FALSE;
	}

	dwSize = GetFileSize(hFile, NULL);
	if (dwCreationFlag == OPEN_EXISTING && fAppend)
	{
		SetFilePointer(hFile, dwSize, NULL, FILE_BEGIN);
	}

	if (!WriteFile(hFile, Buffer, Size, &NumberOfBytesWrite, NULL))
	{
		CloseHandle(hFile);
		return FALSE;
	}

	CloseHandle(hFile);
	return TRUE;
}


VOID 
MapEncryKey(
	LPBYTE lpDeCodeBuffer, 
	LPBYTE lpKeyBuffer, 
	INT nSize
	)
{
	_VMProtectBegin(__FUNCTION__);
	BYTE KeyMap[0x100] = {0}; 
	INT  i = 0;
	BYTE cbTmp = 0;
	BYTE cbIdx = 0;

	for (i = 0;i < 0x100; i++)
	{  
		lpDeCodeBuffer[i] = i;
		KeyMap[i] = lpKeyBuffer[i % nSize];            
	}


	for (i = 0; i < 0x100; i++)
	{
		cbIdx = lpDeCodeBuffer[i] + cbIdx + KeyMap[i];
		cbTmp = lpDeCodeBuffer[i];
		lpDeCodeBuffer[i] = lpDeCodeBuffer[cbIdx];
		lpDeCodeBuffer[cbIdx] = cbTmp; 

	}
	_VMProtectEnd();
}


VOID 
WINAPI 
DeCodeBuffer(
	LPBYTE lpDeCodeBuffer,
	LPBYTE lpKeyBuffer,
	INT nSize,
	LPBYTE lpNewBuffer
	)
{
	_VMProtectBegin(__FUNCTION__);
	BYTE    cnIdx = 0; 
	BYTE    n = 0; 
	BYTE    m = 0; 
	BYTE    nTmp = 0;
	INT     i  = 0;


	for (i = 0 ; i< nSize; i++)
	{
		cnIdx = i + 1;   
		n = lpDeCodeBuffer[cnIdx];
		m += n;

		lpDeCodeBuffer[cnIdx] = lpDeCodeBuffer[m];
		lpDeCodeBuffer[m] = n;
		nTmp = n + lpDeCodeBuffer[cnIdx];
		lpNewBuffer[i] = lpDeCodeBuffer[nTmp] ^ lpKeyBuffer[i];
	}

	RtlZeroMemory(lpDeCodeBuffer,0x100);
	_VMProtectEnd();
}


VOID 
SNQUSimpleCryptData(
	__in LPBYTE lpHashKey,
	__in INT nKeySize,
	__inout LPBYTE lpDataBuffer,
	__in INT nDataSize
	)
{
	_VMProtectBegin(__FUNCTION__);
	BYTE   KeyBuffer[0x100] = {0};
	LPBYTE lpTmpBuffer = NULL;

	lpTmpBuffer = (LPBYTE)LocalAlloc(LPTR,nDataSize);
	if (NULL == lpTmpBuffer)
	{
		return;
	}

	if (nKeySize > 0x100) nKeySize = 0x100;
	MapEncryKey(KeyBuffer,lpHashKey, nKeySize);
	DeCodeBuffer(KeyBuffer, lpDataBuffer,nDataSize,lpTmpBuffer);
	memcpy(lpDataBuffer,lpTmpBuffer,nDataSize);

	LocalFree((HLOCAL)lpTmpBuffer);
	_VMProtectEnd();
}


boolean 
SimpleCryptFile(
	__in LPSTR lpFile
	)
{
	_VMProtectBegin(__FUNCTION__);
	BYTE byKey[] = { 
		0x12, 0x3f, 0x14, 0x15, 0xec, 0x2e, 0x3c, 0x3d, 
		0x2e, 0xc3, 0x04, 0x3f, 0xcc, 0x90, 0x86, 0xe3
	};

	boolean rel = FALSE;
	DWORD dwSize = 0;
	LPBYTE lpData = NULL;

	dwSize = GetPathFileSize(lpFile);
	if (dwSize <= 0)
	{
		return rel;
	}

	lpData = (LPBYTE)LocalAlloc(LMEM_FIXED, dwSize);
	if (lpData != NULL)
	{
		if (ReadPathFileBuffer(lpFile, lpData, dwSize))
		{
			SNQUSimpleCryptData(byKey, sizeof(byKey), lpData, dwSize);
			if (WritePathFileBuffer(lpFile, lpData, dwSize, OPEN_ALWAYS, FALSE))
			{
				rel = TRUE;
			}
		}
		LocalFree((HLOCAL)lpData);
	}

	return rel;
	_VMProtectEnd();
}

CHAR
WINAPI
HexChar2Char(
	__in CHAR ch
	)
{
	if ((ch>='0')&&(ch<='9'))
		return   ch-0x30;  
	else   if((ch>='A')&&(ch<='F'))  
		return   ch-'A'+10;  
	else   if((ch>='a')&&(ch<='f'))  
		return   ch-'a'+10;
	else   return   (-1);
}


VOID
WINAPI
DecryptString(
	__in char *src, 
	__in int len, 
	__out char *out, 
	__in BOOL hexastext
	)
{
	int   i = 0, j = 0;
	BYTE  byascii_src = 0;
	BYTE  hbyte = 0, lbyte = 0;
	char* byte_src = NULL;
	char  cch = 0;
	BYTE  xorkey = 0x01;
	
	if (hexastext) {
		byte_src = (char *)malloc(len);
		for (i = 0, j = 0; i < len; i++) {
			cch = HexChar2Char(src[i]);
			if ((i % 2) == 0)
				hbyte = (BYTE)cch << 4;
			else {
				lbyte = (BYTE)cch;
				byte_src[j++] = (char )(hbyte | lbyte);
			}
		}
		src = byte_src;
		len = j;
	}
	
	for (i = 0; i < len; i++)  {
		byascii_src = (BYTE)src[i];
		// 11110000
		hbyte = (byascii_src & 0xf0) >> 4;
		// 00001111
		lbyte = (byascii_src & 0x0f) << 4;
		byascii_src = lbyte | hbyte;
		
		if (0x00 == xorkey) xorkey++;
		out[i] = (char)(byascii_src ^ (xorkey++));
	}
	
	if (hexastext && byte_src != NULL) free(byte_src);
}


VOID 
WINAPI
SimpleEncryptData(
	__out LPBYTE out, 
	__in LPBYTE in, 
	__in int len
	)
{
	BYTE  byascii_src = 0;
	BYTE  hbyte = 0, lbyte = 0;
	BYTE  xorkey = 0x01;
	int i = 0;

	if (len <= 0)
		return;

	BYTE *tmpbuf = (BYTE *)LocalAlloc(LPTR, len);
	if (!tmpbuf)
	{
		return;
	}

	// 交换奇偶字节顺序
	while (i < len)
	{
		if ((i+1) < len)
		{
			tmpbuf[i] = in[i+1];
			tmpbuf[i+1] = in[i];
		}
		else
		{
			tmpbuf[i] = in[i];
		}

		i += 2;
	}

	// 交换高低位
	for (i = 0; i < len; i++)  
	{
		byascii_src = (BYTE)tmpbuf[i];
		
		// 11110000
		hbyte = (byascii_src & 0xf0) >> 4;
		
		// 00001111
		lbyte = (byascii_src & 0x0f) << 4;
		byascii_src = lbyte | hbyte;

		if (0x00 == xorkey) xorkey++;
		out[i] = (char)(byascii_src ^ (xorkey++));
	}

	LocalFree((HLOCAL)tmpbuf);
}


LPSTR 
SplitStrings(
	__in LPSTR lpszSrc,
	__in LPSTR lpszBTag,
	__in LPSTR lpszETag,
	__out LPSTR lpszValue,
	__in UINT nValueSize
	)
{
	LPSTR lPtr = NULL;
	LPSTR lPtr1 = NULL;
	LPSTR lpszTag = NULL;
	INT nSize = 0;

	if (!lpszBTag && lpszETag != NULL)
	{
		if ((lPtr = strstr(lpszSrc, lpszETag)) != NULL)
		{
			nSize = min((int)(lPtr - lpszSrc), (int)nValueSize);
			strncpy(lpszValue, lpszSrc, nSize);
		}
	}
	else if (lpszBTag != NULL && !lpszETag)
	{
		if ((lPtr = strstr(lpszSrc, lpszBTag)) != NULL)
		{
			lPtr += strlen(lpszBTag);
			nSize = min(strlen(lPtr), (int)nValueSize);
			strncpy(lpszValue, lPtr, nSize);
		}
	}
	else if (lpszBTag != NULL && lpszETag != NULL)
	{
		if ((lPtr = strstr(lpszSrc, lpszBTag)) != NULL)
		{
			lPtr += strlen(lpszBTag);
			if ((lPtr1 = strstr(lPtr, lpszETag)) != NULL)
			{
				nSize = min((int)(lPtr1 - lPtr), (int)nValueSize);
				strncpy(lpszValue, lPtr, nSize);
				lPtr = lPtr1 + strlen(lpszETag);
			}
		}
	}
	else if (!lpszBTag && !lpszETag)
	{
		nSize = min((int)(strlen(lpszSrc)), (int)nValueSize);
		strncpy(lpszValue, lpszSrc, nSize);
		lPtr = lpszSrc + strlen(lpszSrc);
	}

	return lPtr;
}


boolean
WINAPI
LoadFileToBuffer(
	__in LPSTR lpszFilePath,
	__inout LPVOID *lppBuff,
	__in DWORD *pdwSize
	)
{
	LPBYTE lpBuff = NULL;
	DWORD dwSize = 0;
	boolean bRel = FALSE;

	if (!lpszFilePath)
	{
		return FALSE;
	}

	dwSize = GetPathFileSize(lpszFilePath);
	if (dwSize <= 0)
	{
		return FALSE;
	}

	lpBuff = (LPBYTE)LocalAlloc(LPTR, dwSize);
	if (!lpBuff)
	{
		return FALSE;
	}

	if (ReadPathFileBuffer(lpszFilePath, lpBuff, dwSize))
	{
		*lppBuff = lpBuff;
		*pdwSize = dwSize;
		bRel = TRUE;
	}

	return bRel;
}


boolean 
WINAPI
GetHostAdapterDataByIP(
	__inout_ecount(nMaxMac) LPSTR lpszMAC,
	__in UINT nMaxMac,
	__inout_ecount(nMaxIp) LPSTR lpszIpAddr,
	__in UINT nMaxIp,
	__inout_ecount(nMaxMask) LPSTR lpszNetMask,
	__in UINT nMaxMask,
	__in LPCSTR lpHostIP
	)
{
	typedef ULONG 
		(WINAPI *PFNGetAdaptersInfo)(
		IN PIP_ADAPTER_INFO AdapterInfo, 
		IN OUT PULONG SizePointer
		);
	boolean bRel = FALSE;
	PIP_ADAPTER_INFO pAdapterInfo = NULL;
	PIP_ADAPTER_INFO pAdapter = NULL;
	PIP_ADAPTER_INFO pAdapterEntry = NULL;
	DWORD dwRetVal = 0;
	UINT  nMacLen = 0;
	DWORD ulOutBufLen = 0;
	HMODULE hIpHelp;
	PFNGetAdaptersInfo pfnGetAdaptersInfo;

	hIpHelp = LoadLibraryA("iphlpapi.dll");
	if (!hIpHelp)
	{
		lstrcpyA(lpszMAC, "none");
		return bRel;
	}

	pfnGetAdaptersInfo = (PFNGetAdaptersInfo)GetProcAddress(hIpHelp, "GetAdaptersInfo");
	if (!pfnGetAdaptersInfo)
	{
		lstrcpyA(lpszMAC, "none");
		return bRel;
	}

	pAdapterInfo = (IP_ADAPTER_INFO *) LocalAlloc(LPTR, sizeof(IP_ADAPTER_INFO));
	ulOutBufLen = sizeof(IP_ADAPTER_INFO);

	// Make an initial call to GetAdaptersInfo to get
	// the necessary size into the ulOutBufLen variable
	if (pfnGetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) 
	{
		LocalFree(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *)LocalAlloc(LPTR, ulOutBufLen);
	}

	if ((dwRetVal = pfnGetAdaptersInfo( pAdapterInfo, &ulOutBufLen)) == NO_ERROR) 
	{
		pAdapter = pAdapterInfo;
		pAdapterEntry = pAdapterInfo;

		while (pAdapter)
		{
			if (pAdapter->Type == MIB_IF_TYPE_LOOPBACK || 
				!_strcmpi(pAdapter->IpAddressList.IpAddress.String, "0.0.0.0") ||
				!_strcmpi(pAdapter->IpAddressList.IpMask.String, "0.0.0.0"))
			{
				pAdapter->Type = 0xffff;
				pAdapter = pAdapter->Next;
				continue;
			}

			//
			// 取MIB_IF_TYPE_ETHERNET 类型的网卡
			// 
			if (pAdapter->Type == MIB_IF_TYPE_ETHERNET || pAdapter->Type ==  IF_TYPE_IEEE80211)
			{
				if (lpHostIP != NULL && _strcmpi(lpHostIP, pAdapter->IpAddressList.IpAddress.String) == 0)
				{
					wsprintfA(
						lpszMAC + nMacLen, 
						nMacLen > 0 ? ",%02X-%02X-%02X-%02X-%02X-%02X" : "%02X-%02X-%02X-%02X-%02X-%02X",
						pAdapter->Address[0],
						pAdapter->Address[1],
						pAdapter->Address[2],
						pAdapter->Address[3],
						pAdapter->Address[4],
						pAdapter->Address[5]
					);

					strncpy_s(lpszIpAddr,nMaxIp, pAdapter->IpAddressList.IpAddress.String, strlen(pAdapter->IpAddressList.IpAddress.String));
					strncpy_s(lpszNetMask,nMaxMask,pAdapter->IpAddressList.IpMask.String, strlen(pAdapter->IpAddressList.IpMask.String));
					pAdapter->Type = 0xffff;
					bRel = TRUE;
					break;
				}

			}

			pAdapter = pAdapter->Next;
		}
	}
	else
	{
		strcpy_s(lpszMAC, nMaxMac , "none");
	}

	if (pAdapterInfo != NULL) 
	{
		LocalFree(pAdapterInfo);
	}
	return bRel;
}


boolean 
WINAPI
GetHostMACByIP(
	__inout_ecount(nMaxMac) LPSTR lpszMAC,
	__in UINT nMaxMac,
	__in LPCSTR lpHostIP
	)
{
	typedef ULONG 
		(WINAPI *PFNGetAdaptersInfo)(
		IN PIP_ADAPTER_INFO AdapterInfo, 
		IN OUT PULONG SizePointer
		);
	boolean bRel = FALSE;
	PIP_ADAPTER_INFO pAdapterInfo = NULL;
	PIP_ADAPTER_INFO pAdapter = NULL;
	PIP_ADAPTER_INFO pAdapterEntry = NULL;
	DWORD dwRetVal = 0;
	DWORD ulOutBufLen = 0;
	HMODULE hIpHelp;
	PFNGetAdaptersInfo pfnGetAdaptersInfo;

	hIpHelp = LoadLibraryA("iphlpapi.dll");
	if (!hIpHelp)
	{
		lstrcpyA(lpszMAC, "none");
		return bRel;
	}

	pfnGetAdaptersInfo = (PFNGetAdaptersInfo)GetProcAddress(hIpHelp, "GetAdaptersInfo");
	if (!pfnGetAdaptersInfo)
	{
		lstrcpyA(lpszMAC, "none");
		return bRel;
	}

	pAdapterInfo = (IP_ADAPTER_INFO *) LocalAlloc(LPTR, sizeof(IP_ADAPTER_INFO));
	ulOutBufLen = sizeof(IP_ADAPTER_INFO);

	// Make an initial call to GetAdaptersInfo to get
	// the necessary size into the ulOutBufLen variable
	if (pfnGetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) 
	{
		LocalFree(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *)LocalAlloc(LPTR, ulOutBufLen);
	}

	if ((dwRetVal = pfnGetAdaptersInfo( pAdapterInfo, &ulOutBufLen)) == NO_ERROR) 
	{
		pAdapter = pAdapterInfo;
		pAdapterEntry = pAdapterInfo;

		while (pAdapter)
		{
			if (pAdapter->Type == MIB_IF_TYPE_LOOPBACK || 
				!_strcmpi(pAdapter->IpAddressList.IpAddress.String, "0.0.0.0") ||
				!_strcmpi(pAdapter->IpAddressList.IpMask.String, "0.0.0.0"))
			{
				pAdapter->Type = 0xffff;
				pAdapter = pAdapter->Next;
				continue;
			}

			//
			// 取MIB_IF_TYPE_ETHERNET 类型的网卡
			// 
			if (pAdapter->Type == MIB_IF_TYPE_ETHERNET || pAdapter->Type ==  IF_TYPE_IEEE80211)
			{
				if (lpHostIP != NULL && _strcmpi(lpHostIP, pAdapter->IpAddressList.IpAddress.String) == 0)
				{
					wsprintfA(
						lpszMAC, "%02X-%02X-%02X-%02X-%02X-%02X",
						pAdapter->Address[0],
						pAdapter->Address[1],
						pAdapter->Address[2],
						pAdapter->Address[3],
						pAdapter->Address[4],
						pAdapter->Address[5]
					);

					pAdapter->Type = 0xffff;
					bRel = TRUE;
					break;
				}

			}

			pAdapter = pAdapter->Next;
		}
	}
	else
	{
		strcpy_s(lpszMAC, nMaxMac , "none");
	}

	if (pAdapterInfo != NULL) 
	{
		LocalFree(pAdapterInfo);
	}
	return bRel;
}


boolean 
WINAPI
GetIPMaskByMAC(
	__in LPCSTR lpMAC,
	__inout_ecount(nMaxMask) LPSTR lpszNetMask,
	__in UINT nMaxMask
	)
{
	typedef ULONG 
		(WINAPI *PFNGetAdaptersInfo)(
		IN PIP_ADAPTER_INFO AdapterInfo, 
		IN OUT PULONG SizePointer
		);
	boolean bRel = FALSE;
	PIP_ADAPTER_INFO pAdapterInfo = NULL;
	PIP_ADAPTER_INFO pAdapter = NULL;
	PIP_ADAPTER_INFO pAdapterEntry = NULL;
	DWORD dwRetVal = 0;
	DWORD ulOutBufLen = 0;
	HMODULE hIpHelp;
	PFNGetAdaptersInfo pfnGetAdaptersInfo;
	CHAR szMAC[128] = {0};

	if (!lpMAC)
	{
		return bRel;
	}

	hIpHelp = LoadLibraryA("iphlpapi.dll");
	if (!hIpHelp)
	{
		return bRel;
	}

	pfnGetAdaptersInfo = (PFNGetAdaptersInfo)GetProcAddress(hIpHelp, "GetAdaptersInfo");
	if (!pfnGetAdaptersInfo)
	{
		return bRel;
	}

	pAdapterInfo = (IP_ADAPTER_INFO *) LocalAlloc(LPTR, sizeof(IP_ADAPTER_INFO));
	ulOutBufLen = sizeof(IP_ADAPTER_INFO);

	// Make an initial call to GetAdaptersInfo to get
	// the necessary size into the ulOutBufLen variable
	if (pfnGetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) 
	{
		LocalFree(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *)LocalAlloc(LPTR, ulOutBufLen);
	}

	if ((dwRetVal = pfnGetAdaptersInfo( pAdapterInfo, &ulOutBufLen)) == NO_ERROR) 
	{
		pAdapter = pAdapterInfo;
		pAdapterEntry = pAdapterInfo;

		while (pAdapter)
		{
			if (pAdapter->Type == MIB_IF_TYPE_LOOPBACK || 
				!_strcmpi(pAdapter->IpAddressList.IpAddress.String, "0.0.0.0") ||
				!_strcmpi(pAdapter->IpAddressList.IpMask.String, "0.0.0.0"))
			{
				pAdapter->Type = 0xffff;
				pAdapter = pAdapter->Next;
				continue;
			}

			//
			// 取MIB_IF_TYPE_ETHERNET 类型的网卡
			// 
			if (pAdapter->Type == MIB_IF_TYPE_ETHERNET || pAdapter->Type ==  IF_TYPE_IEEE80211)
			{
				wsprintfA(
					szMAC, "%02X-%02X-%02X-%02X-%02X-%02X",
					pAdapter->Address[0],
					pAdapter->Address[1],
					pAdapter->Address[2],
					pAdapter->Address[3],
					pAdapter->Address[4],
					pAdapter->Address[5]
				);

				if (_strcmpi(szMAC, lpMAC) == 0)
				{
					strncpy_s(lpszNetMask,nMaxMask,pAdapter->IpAddressList.IpMask.String, strlen(pAdapter->IpAddressList.IpMask.String));
					pAdapter->Type = 0xffff;
					bRel = TRUE;
					break;
				}
			}

			pAdapter = pAdapter->Next;
		}
	}

	if (pAdapterInfo != NULL) 
	{
		LocalFree(pAdapterInfo);
	}
	return bRel;
}

#include <WtsApi32.h>

boolean 
WINAPI
SetPrivilege(
	HANDLE hToken,          // access token handle
	LPCTSTR lpszPrivilege,  // name of privilege to enable/disable
	boolean bEnablePrivilege   // to enable or disable privilege
	) 
{
	TOKEN_PRIVILEGES tp;
	LUID luid;

	if ( !LookupPrivilegeValue( 
		NULL,            // lookup privilege on local system
		lpszPrivilege,   // privilege to lookup 
		&luid ) )        // receives LUID of privilege
	{
		return FALSE; 
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if (bEnablePrivilege)
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	else
		tp.Privileges[0].Attributes = 0;

	// Enable the privilege or disable all privileges.

	if ( !AdjustTokenPrivileges(
		hToken, 
		FALSE, 
		&tp, 
		sizeof(TOKEN_PRIVILEGES), 
		(PTOKEN_PRIVILEGES) NULL, 
		(PDWORD) NULL) )
	{ 
		return FALSE; 
	} 

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
	{
		return FALSE;
	} 

	return TRUE;
}


boolean
WINAPI
EnableTcbPrivilege(
	IN boolean bEnable
	)
{ 
	HANDLE hToken;
	TOKEN_PRIVILEGES tp;
	LUID luid;

	if(!::OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
	{
 		return FALSE;
	}

	if(!::LookupPrivilegeValue(NULL, SE_TCB_NAME, &luid))
	{
 		::CloseHandle(hToken);
		return FALSE;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if(bEnable)
	{
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	}
	else
	{
		tp.Privileges[0].Attributes = 0;
	}

	if(!::AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL))
	{
 		::CloseHandle(hToken);
		return FALSE;
	}

	::CloseHandle(hToken);

	if(::GetLastError() == ERROR_NOT_ALL_ASSIGNED)
	{
 		return FALSE;
	}
	return TRUE;
}

boolean
WINAPI
EnableShutdownPrivilege(
	IN boolean bEnable
	)
{ 
	HANDLE hToken;
	TOKEN_PRIVILEGES tp;
	LUID luid;

	if(!::OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
	{
 		return FALSE;
	}

	if(!::LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &luid))
	{
 		::CloseHandle(hToken);
		return FALSE;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if(bEnable)
	{
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	}
	else
	{
		tp.Privileges[0].Attributes = 0;
	}

	if(!::AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL))
	{
 		::CloseHandle(hToken);
		return FALSE;
	}

	::CloseHandle(hToken);

	if(::GetLastError() == ERROR_NOT_ALL_ASSIGNED)
	{
 		return FALSE;
	}
	return TRUE;
}

boolean
WINAPI
EnablePrivilege(
	IN boolean bEnable,
	IN LPCSTR Privilege
	)
{ 
	HANDLE hToken;
	TOKEN_PRIVILEGES tp;
	LUID luid;

	if(!::OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
	{
 		return FALSE;
	}

	if(!::LookupPrivilegeValue(NULL, Privilege, &luid))
	{
 		::CloseHandle(hToken);
		return FALSE;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if(bEnable)
	{
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	}
	else
	{
		tp.Privileges[0].Attributes = 0;
	}

	if(!::AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL))
	{
 		::CloseHandle(hToken);
		return FALSE;
	}

	::CloseHandle(hToken);

	if(::GetLastError() == ERROR_NOT_ALL_ASSIGNED)
	{
 		return FALSE;
	}
	return TRUE;
}

BOOL  
WINAPI
AnsiStringToUnicodeString(
	__in LPSTR	lpMultiByteStr,
	__out LPWSTR lpWideCharStr, 
	__in INT cchWideChar 
	)
{
	BOOL bRel = FALSE;
	
	bRel = (BOOL)MultiByteToWideChar(
		CP_ACP,
		0,
		lpMultiByteStr,
		-1,
		lpWideCharStr,
		cchWideChar
		);
	
	
	return bRel;
}


BOOL
WINAPI
UnicodeStringToAnsiString(
	__in LPWSTR  lpWideCharStr, 
	__out LPSTR	lpMultiByteStr,
	__in INT cbMultiByte
	)
{
	BOOL bRel = FALSE;
	
	bRel = (BOOL)WideCharToMultiByte(
		CP_ACP,
		0,
		lpWideCharStr,
		-1,
		lpMultiByteStr,
		cbMultiByte,
		NULL,
		NULL
		);
	
	return bRel;
}

DWORD  
WINAPI  
GetProcessThreadIdByNameA(
	__in LPSTR lpProcName
	)
{
	THREADENTRY32 stThreadList = {0}; 
	DWORD dwThreadId = 0;
	HANDLE	hSnp = NULL;
	DWORD dwPID = 0;

	dwPID = GetProcessIdByNameA(lpProcName);
	if (!dwPID)
		return 0;

	hSnp = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 );
	if (NULL != hSnp )
	{
		stThreadList.dwSize = sizeof(stThreadList);
		if (Thread32First( hSnp, &stThreadList))
		{
			do 
			{
				if (dwPID == stThreadList.th32OwnerProcessID)
				{
					dwThreadId = stThreadList.th32ThreadID;
					break;
				}

			} while (Thread32Next(hSnp, &stThreadList));
		}

		CloseHandle( hSnp );
	}

	return dwThreadId;
}


DWORD  
WINAPI  
GetProcessThreadIdByNameW(
	__in LPWSTR lpProcName
	)
{
	THREADENTRY32 stThreadList = {0}; 
	DWORD dwThreadId = 0;
	HANDLE	hSnp = NULL;
	DWORD dwPID = 0;

	dwPID = GetProcessIdByNameW(lpProcName);
	if (!dwPID)
		return 0;

	hSnp = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 );
	if (NULL != hSnp )
	{
		stThreadList.dwSize = sizeof(stThreadList);
		if (Thread32First( hSnp, &stThreadList))
		{
			do 
			{
				if (dwPID == stThreadList.th32OwnerProcessID)
				{
					dwThreadId = stThreadList.th32ThreadID;
					break;
				}

			} while (Thread32Next(hSnp, &stThreadList));
		}

		CloseHandle( hSnp );
	}

	return dwThreadId;
}

DWORD
WINAPI
GetProcessIdByNameA(
	__in LPSTR lpProcName
	)
{
	PROCESSENTRY32  stProcList = {0}; 
	DWORD			dwPid = 0;
	HANDLE			hSnp = NULL;
	
	if (NULL == lpProcName)
	{
		return 0;
	}
	
	hSnp = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if (NULL != hSnp )
	{
		stProcList.dwSize = sizeof(stProcList);
		if (Process32First( hSnp, &stProcList))
		{
			do 
			{
				if (!lstrcmpiA(lpProcName,stProcList.szExeFile))
				{
					dwPid = stProcList.th32ProcessID;
					break;
				}
				
			} while (Process32Next(hSnp , &stProcList));
		}
		
		
		CloseHandle( hSnp );
	}
	
	return dwPid;
}


DWORD  
WINAPI  
GetProcessIdByNameW( 
	__in LPWSTR lpProcName 
	)
{
	PROCESSENTRY32W  stProcList = {0}; 
	DWORD			 dwPid = 0;
	HANDLE			hSnp = NULL;
	
	if (NULL == lpProcName)
	{
		return 0;
	}
	
	hSnp = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if (NULL != hSnp )
	{
		stProcList.dwSize = sizeof(stProcList);
		if (Process32FirstW( hSnp, &stProcList))
		{
			do 
			{
				if (!lstrcmpiW(lpProcName,stProcList.szExeFile))
				{
					dwPid = stProcList.th32ProcessID;
					break;
				}
				
			} while (Process32NextW(hSnp , &stProcList));
		}
		
		
		CloseHandle( hSnp );
	}
	
	return dwPid;
}


BOOL 
WINAPI
DosPathToNtPath(
	LPSTR pszDosPath, 
	LPSTR pszNtPath,
	UINT nSizeNtPath
	)
{
	CHAR  szDriveStr[500];
	CHAR  szDrive[3];
	CHAR  szDevName[100];
	INT  cchDevName;
	INT  i;

	//检查参数
	if(!pszDosPath || !pszNtPath )
		return FALSE;

	//获取本地磁盘字符串
	if (GetLogicalDriveStrings(sizeof(szDriveStr), szDriveStr))
	{
		for (i = 0; szDriveStr[i]; i += 4)
		{
			if (!_strcmpi(&(szDriveStr[i]), "A:\\") || !_strcmpi(&(szDriveStr[i]), "B:\\"))
				continue;

			szDrive[0] = szDriveStr[i];
			szDrive[1] = szDriveStr[i + 1];
			szDrive[2] = '\0';

			// 查询 Dos 设备名
			if (!QueryDosDevice(szDrive, szDevName, 100))
				return FALSE;

			cchDevName = strlen(szDevName);
			if (_strnicmp(pszDosPath, szDevName, cchDevName) == 0)//命中
			{
				strncpy(pszNtPath, szDrive, sizeof(szDrive));//复制驱动器
				strncat(pszNtPath, pszDosPath + cchDevName, nSizeNtPath);//复制路径
				return TRUE;
			}           
		}
	}

	strncpy(pszNtPath, pszDosPath, nSizeNtPath);
	return FALSE;
}


BOOL 
WINAPI
GetProcessFullPath(
	__in DWORD dwPID, 
	__inout LPSTR pszFullPath,
	__in UINT nSizePath
	)
{
	// 获取进程完整路径
	CHAR szImagePath[MAX_PATH] = {0};
	HANDLE hProcess = NULL;

	if(!pszFullPath)
		return FALSE;

	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwPID);
	if (!hProcess)
		return FALSE;

	if(!GetProcessImageFileNameA(hProcess, szImagePath, MAX_PATH))
	{
		CloseHandle(hProcess);
		return FALSE;
	}

	if (!DosPathToNtPath(szImagePath, pszFullPath, nSizePath))
	{
		CloseHandle(hProcess);
		return FALSE;
	}

	CloseHandle(hProcess);
	return TRUE;
}

BOOL 
WINAPI 
WinExecByCurrentExplorerTokenW(
	__in LPWSTR  lpCommandLine,
	__in INT		nShow
	)
{
	BOOL       bRel = FALSE;
	HANDLE     hToken = NULL;
	HANDLE     hProcess = NULL;
	DWORD      dwSessionId = 0;
	DWORD      dwProcessSid = 0;
	DWORD      dwPid  = 0;
	LPVOID     lpEnv = NULL;
	STARTUPINFOW				stStartup	= {0};
	PROCESS_INFORMATION			stProcInfo	= {0};
	DWORD		dwLastError = 0;
	

	dwSessionId = WTSGetActiveConsoleSessionId();
	dwPid = GetProcessIdByNameW(L"explorer.exe");

	if (0 != dwPid)
	{
		if ((-1 != dwSessionId) && ProcessIdToSessionId(dwPid,&dwProcessSid))
		{
			if (dwProcessSid == dwSessionId)
			{
				hProcess = OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,dwPid);
				if (NULL != hProcess)
				{
					if(!OpenProcessToken(hProcess,TOKEN_ALL_ACCESS,&hToken))
					{
						hToken = NULL;
					}
					CloseHandle(hProcess);
				}
			}
		}

	}

	if (NULL == hToken)
	{
		if(OpenProcessToken(GetCurrentProcess(),TOKEN_ALL_ACCESS,&hToken))
		{
			if(!SetTokenInformation(hToken,TokenSessionId,&dwSessionId,sizeof(DWORD)))
			{
				CloseHandle(hToken);
				return bRel;
			}
		}
		else
		{
			return bRel;
		}
	}

	stStartup.cb = sizeof(STARTUPINFOW);
	stStartup.dwFlags |= STARTF_USESHOWWINDOW;
	stStartup.wShowWindow = LOWORD(nShow);
	stStartup.lpDesktop = L"WinSta0\\Default";

	CreateEnvironmentBlock(&lpEnv, hToken,FALSE);

	bRel = CreateProcessAsUserW(
		hToken,
		NULL,
		lpCommandLine,
		NULL,
		NULL,
		FALSE,
		CREATE_UNICODE_ENVIRONMENT,
		lpEnv,
		NULL,
		&stStartup,
		&stProcInfo
		);
	
	dwLastError = GetLastError();
	
	if (bRel)
	{
		if (NULL != stProcInfo.hProcess )
		{
			CloseHandle(stProcInfo.hProcess);
		}

		if (NULL != stProcInfo.hThread)
		{
			CloseHandle(stProcInfo.hThread);
		}
	}

	if (NULL != lpEnv)
	{
		DestroyEnvironmentBlock(lpEnv);
	}

	CloseHandle(hToken);
	
	SetLastError(dwLastError);

	return bRel;

}


BOOL 
WINAPI 
WinExecByCurrentExplorerTokenA(
	__in LPSTR   lpCommandLine,
	__in INT		nShow
	)
{
	LPWSTR  lpwszCommandLine = NULL;
	BOOL	bRel = FALSE;
	DWORD   dwLastError = 0;

	lpwszCommandLine = (LPWSTR)LocalAlloc(
		LMEM_FIXED,
		MAX_COMMAND_LINE_BUFFER
		);

	if (NULL == lpwszCommandLine)
	{
		return bRel;
	}

	bRel = AnsiStringToUnicodeString(
		lpCommandLine,lpwszCommandLine,
		MAX_COMMAND_LINE_BUFFER / sizeof(WCHAR)
		);

	if (bRel)
	{
		bRel = WinExecByCurrentExplorerTokenW(
			lpwszCommandLine,
			nShow
			);

		dwLastError = GetLastError();
	}

	LocalFree((HLOCAL)lpwszCommandLine);
	SetLastError(dwLastError);

	return bRel;
}


BOOL
WINAPI
WinExecProcessW(
	__in LPWSTR  lpCommandLine,
	__in INT	 nShow
	)
{
	BOOL bRel = FALSE;
	STARTUPINFOW				stStartup	= {0};
	PROCESS_INFORMATION			stProcInfo	= {0};
	DWORD	dwLastError = 0;

	stStartup.cb = sizeof(STARTUPINFOW);
	stStartup.dwFlags |= STARTF_USESHOWWINDOW;
	stStartup.wShowWindow = LOWORD(nShow);

	bRel = CreateProcessW( 
		NULL,   // No module name (use command line)
		lpCommandLine,  // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&stStartup,     // Pointer to STARTUPINFO structure
		&stProcInfo 
		);    

	dwLastError = GetLastError();
	return bRel;
}

BOOL
WINAPI
WinExecProcessA(
	__in LPSTR   lpCommandLine,
	__in INT	 nShow
	)
{
	LPWSTR  lpwszCommandLine = NULL;
	BOOL	bRel = FALSE;
	DWORD   dwLastError = 0;

	lpwszCommandLine = (LPWSTR)LocalAlloc(
		LMEM_FIXED,
		MAX_COMMAND_LINE_BUFFER
		);

	if (NULL == lpwszCommandLine)
	{
		return bRel;
	}

	bRel = AnsiStringToUnicodeString(
		lpCommandLine,lpwszCommandLine,
		MAX_COMMAND_LINE_BUFFER / sizeof(WCHAR)
		);

	if (bRel)
	{
		bRel = WinExecProcessW(
			lpwszCommandLine,
			nShow
			);

		dwLastError = GetLastError();
	}

	LocalFree((HLOCAL)lpwszCommandLine);
	SetLastError(dwLastError);

	return bRel;
}


BOOL
WINAPI
WinExecProcessExW(
	__in LPWSTR  lpAppName,
	__in LPWSTR  lpCommandLine,
	__in INT	 nShow,
	__in LPWSTR  lpDeskName,
	__out PPROCESS_INFORMATION pi
	)
{
	BOOL bRel = FALSE;
	STARTUPINFOW stStartup	= {0};
	DWORD dwLastError = 0;

	stStartup.cb = sizeof(STARTUPINFOW);
	stStartup.dwFlags |= STARTF_USESHOWWINDOW;
	stStartup.wShowWindow = LOWORD(nShow);
	stStartup.lpDesktop = lpDeskName;

	bRel = CreateProcessW( 
		lpAppName,		// App Name
		lpCommandLine,  // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&stStartup,     // Pointer to STARTUPINFO structure
		pi 
		);    

	dwLastError = GetLastError();
	SetLastError(dwLastError);
	return bRel;
}

BOOL
WINAPI
WinExecProcessExA(
	__in LPSTR  lpAppName,
	__in LPSTR  lpCommandLine,
	__in INT	 nShow,
	__in LPSTR  lpDeskName,
	__out PPROCESS_INFORMATION pi
	)
{
	BOOL bRel = FALSE;
	STARTUPINFOA stStartup	= {0};
	DWORD dwLastError = 0;

	stStartup.cb = sizeof(STARTUPINFOA);
	stStartup.dwFlags |= STARTF_USESHOWWINDOW;
	stStartup.wShowWindow = LOWORD(nShow);
	stStartup.lpDesktop = lpDeskName;

	bRel = CreateProcessA( 
		lpAppName,		// App Name
		lpCommandLine,  // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&stStartup,     // Pointer to STARTUPINFO structure
		pi 
		);

	dwLastError = GetLastError();
	SetLastError(dwLastError);
	return bRel;
}


BOOL
WINAPI
SetRegistryDwordValueW(
	__in HKEY hRootKey,
	__in_opt LPCWSTR lpSubKey,
	__in LPWSTR lpValueName,
	__in DWORD dwValue
	)
{
	HKEY hkey = NULL; 
	LSTATUS lRet = -1;
	BOOL bRet = FALSE;

	lRet = RegOpenKeyExW(hRootKey, 
				lpSubKey, 
				NULL, 
				KEY_ALL_ACCESS, 
				&hkey); 
	if (ERROR_SUCCESS == lRet)
	{
		lRet = RegSetValueExW(hkey, lpValueName, 0, REG_DWORD, (LPBYTE)&dwValue, 4);
		if (ERROR_SUCCESS == lRet)
		{
			bRet = TRUE;
		}

		RegCloseKey(hkey); 
	}
	
	return bRet;
}


BOOL
WINAPI
SetRegistryDwordValueA(
	__in HKEY hRootKey,
	__in_opt LPCSTR lpSubKey,
	__in LPSTR lpValueName,
	__in DWORD dwValue
	)
{
	HKEY hkey = NULL; 
	LSTATUS lRet = -1;
	BOOL bRet = FALSE;

	lRet = RegOpenKeyExA(hRootKey, 
				lpSubKey, 
				NULL, 
				KEY_ALL_ACCESS, 
				&hkey); 
	if (ERROR_SUCCESS == lRet)
	{
		lRet = RegSetValueExA(hkey, lpValueName, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));
		if (ERROR_SUCCESS == lRet)
		{
			bRet = TRUE;
		}

		RegCloseKey(hkey);
	}
	
	return bRet;
}


BOOL
WINAPI
CreateRegistryKeyA(
	__in HKEY hRootKey,
	__in_opt LPCSTR lpSubKey,
	__in LPSTR lpValueName	
	)
{
	HKEY hkey = NULL; 
	LSTATUS lRet = -1;
	BOOL bRet = FALSE;

	lRet = RegOpenKeyExA(hRootKey, 
		lpSubKey, 
		NULL, 
		KEY_ALL_ACCESS, 
		&hkey); 
	if (ERROR_FILE_NOT_FOUND == lRet)
	{
		HKEY hReKey = NULL;
		DWORD dwDispost = REG_CREATED_NEW_KEY;
		lRet = RegCreateKeyEx(hRootKey, lpSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
					NULL, &hReKey, &dwDispost);

		if (ERROR_SUCCESS == lRet)
		{
			RegCloseKey(hReKey);
			bRet = TRUE;
		}
	}
	else if (ERROR_SUCCESS == lRet)
	{
		RegCloseKey(hkey);
		bRet = TRUE;
	}

	return bRet;
}

DWORD
WINAPI
GetRegistryDwordValueA(
	__in HKEY hRootKey,
	__in_opt LPCSTR lpSubKey,
	__in LPSTR lpValueName
	)
{
	HKEY hkey = NULL; 
	LSTATUS lRet = -1;
	DWORD dwRet = 0;
	DWORD dwSize = sizeof(DWORD);

	lRet = RegOpenKeyExA(hRootKey, 
		lpSubKey, 
		NULL, 
		KEY_ALL_ACCESS, 
		&hkey); 
	if (ERROR_SUCCESS == lRet)
	{
		DWORD dwType = REG_DWORD;
		//lRet = RegGetValueA(hkey, lpSubKey, lpValueName, RRF_RT_DWORD, &dwType, &dwRet, &dwSize);
		SHGetValue(hRootKey, lpSubKey, lpValueName, &dwType, &dwRet, &dwSize);
		RegCloseKey(hkey);
	}

	return dwRet;
}

BOOL
WINAPI
SetRegistryStringValueA(
	__in HKEY hRootKey,
	__in_opt LPCSTR lpSubKey,
	__in LPSTR lpValueName,
	__in LPSTR lpszValue,
	__in UINT ncbLen
	)
{
	HKEY hkey = NULL; 
	LSTATUS lRet = -1;
	BOOL bRet = FALSE;
	DWORD dwFlags = KEY_ALL_ACCESS;

	if (SNIsWow64())
	{
		dwFlags |= KEY_WOW64_64KEY;
	}

	lRet = RegOpenKeyExA(hRootKey, 
				lpSubKey, 
				NULL, 
				dwFlags, 
				&hkey); 
	if (ERROR_SUCCESS == lRet)
	{
		lRet = RegSetValueExA(hkey, lpValueName, 0, REG_SZ, (LPBYTE)lpszValue, ncbLen);
		if (ERROR_SUCCESS == lRet)
		{
			bRet = TRUE;
		}

		RegCloseKey(hkey);
	}
	
	return bRet;
}


BOOL
WINAPI
SetRegistryStringValueW(
	__in HKEY hRootKey,
	__in_opt LPCWSTR lpSubKey,
	__in LPWSTR lpValueName,
	__in LPWSTR lpszValue,
	__in UINT ncbLen
	)
{
	HKEY hkey = NULL; 
	LSTATUS lRet = -1;
	BOOL bRet = FALSE;
	DWORD dwFlags = KEY_ALL_ACCESS;

	if (SNIsWow64())
	{
		dwFlags |= KEY_WOW64_64KEY;
	}

	lRet = RegOpenKeyExW(hRootKey, 
		lpSubKey, 
		NULL, 
		dwFlags, 
		&hkey); 
	if (ERROR_SUCCESS == lRet)
	{
		lRet = RegSetValueExW(hkey, lpValueName, 0, REG_SZ, (LPBYTE)lpszValue, ncbLen);
		if (ERROR_SUCCESS == lRet)
		{
			bRet = TRUE;
		}

		RegCloseKey(hkey);
	}

	return bRet;
}


BOOL
WINAPI
GetRegistryStringValueA(
	__in HKEY hRootKey,
	__in_opt LPCSTR lpSubKey,
	__in LPSTR lpValueName,
	__in LPSTR lpszValue,
	__inout LPDWORD lpcbDataLen
	)
{
	HKEY hkey = NULL; 
	LSTATUS lRet = -1;
	BOOL bRet = FALSE;
	DWORD dwType = 0;
	DWORD dwFlags = KEY_ALL_ACCESS;

	if (SNIsWow64())
	{
		dwFlags |= KEY_WOW64_64KEY;
	}

	lRet = RegOpenKeyExA(hRootKey, 
				lpSubKey, 
				NULL, 
				dwFlags, 
				&hkey); 
	if (ERROR_SUCCESS == lRet)
	{
		lRet = RegQueryValueExA(hkey, lpValueName, NULL, &dwType, (LPBYTE)lpszValue, lpcbDataLen);
		if (ERROR_SUCCESS == lRet)
		{
			bRet = TRUE;
		}

		RegCloseKey(hkey);
	}
	
	return bRet;
}


BOOL
WINAPI
GetRegistryStringValueW(
	__in HKEY hRootKey,
	__in_opt LPCWSTR lpSubKey,
	__in LPWSTR lpValueName,
	__in LPWSTR lpszValue,
	__inout LPDWORD lpcbDataLen
	)
{
	HKEY hkey = NULL; 
	LSTATUS lRet = -1;
	BOOL bRet = FALSE;
	DWORD dwType = REG_SZ;
	DWORD dwFlags = KEY_ALL_ACCESS;

	if (SNIsWow64())
	{
		dwFlags |= KEY_WOW64_64KEY;
	}

	lRet = RegOpenKeyExW(hRootKey, 
				lpSubKey, 
				NULL, 
				dwFlags, 
				&hkey); 
	if (ERROR_SUCCESS == lRet)
	{
		lRet = RegQueryValueExW(hkey, lpValueName, NULL, &dwType, (LPBYTE)lpszValue, lpcbDataLen);
		if (ERROR_SUCCESS == lRet)
		{
			bRet = TRUE;
		}
		else if (ERROR_MORE_DATA == lRet)
		{

		}

		RegCloseKey(hkey);
	}
	
	return bRet;
}


BOOL 
WINAPI
DeleteRegistryValueA(
	__in HKEY hRootKey,
	__in_opt LPCSTR lpSubKey,
	__in LPSTR lpValueName
	)
{
	HKEY hkey = NULL; 
	LSTATUS lRet = -1;
	BOOL bRet = FALSE;

	lRet = RegOpenKeyExA(hRootKey, 
		lpSubKey, 
		NULL, 
		KEY_ALL_ACCESS, 
		&hkey); 
	if (ERROR_SUCCESS == lRet)
	{
		lRet = RegDeleteValueA(hkey, lpValueName);
		if (ERROR_SUCCESS == lRet)
		{
			bRet = TRUE;
		}

		RegCloseKey(hkey);
	}

	return bRet;
}


BOOL 
WINAPI
DeleteRegistryValueW(
	__in HKEY hRootKey,
	__in_opt LPCWSTR lpSubKey,
	__in LPWSTR lpValueName
	)
{
	HKEY hkey = NULL; 
	LSTATUS lRet = -1;
	BOOL bRet = FALSE;

	lRet = RegOpenKeyExW(hRootKey, 
		lpSubKey, 
		NULL, 
		KEY_ALL_ACCESS, 
		&hkey); 
	if (ERROR_SUCCESS == lRet)
	{
		lRet = RegDeleteValueW(hkey, lpValueName);
		if (ERROR_SUCCESS == lRet)
		{
			bRet = TRUE;
		}

		RegCloseKey(hkey);
	}

	return bRet;
}

DWORD
WINAPI
GetRegistryBinaryValueA(
	__in HKEY hRootKey,
	__in_opt LPCSTR lpSubKey,
	__in LPSTR lpValueName,
	__out LPVOID lpData,
	__in DWORD dwSize
	)
{
	HKEY hkey = NULL; 
	LSTATUS lRet = -1;
	DWORD dwRet = 0;

	lRet = RegOpenKeyExA(hRootKey, 
		lpSubKey, 
		NULL, 
		KEY_ALL_ACCESS, 
		&hkey); 
	if (ERROR_SUCCESS == lRet)
	{
		DWORD dwType = REG_BINARY;
		lRet = SHGetValue(hRootKey, lpSubKey, lpValueName, &dwType, lpData, &dwSize);
		RegCloseKey(hkey);
	}

	return dwRet;
}


DWORD
WINAPI
SetRegistryBinaryValueA(
	__in HKEY hRootKey,
	__in_opt LPCSTR lpSubKey,
	__in LPSTR lpValueName,
	__in LPVOID lpData,
	__in DWORD dwSize
	)
{
	HKEY hkey = NULL; 
	LSTATUS lRet = -1;
	DWORD dwRet = 0;

	lRet = RegOpenKeyExA(hRootKey, 
		lpSubKey, 
		NULL, 
		KEY_ALL_ACCESS, 
		&hkey); 
	if (ERROR_SUCCESS == lRet)
	{
		DWORD dwType = REG_BINARY;
		lRet = SHSetValue(hRootKey, lpSubKey, lpValueName, dwType, lpData, dwSize);
		RegCloseKey(hkey);
	}

	return dwRet;
}

INT64
WINAPI
GetLocalTime64(
	)
{
	SYSTEMTIME systm;
	GetLocalTime(&systm);
	return (INT64)(time(NULL) * 1000 + systm.wMilliseconds);
}


DWORD
WINAPI
GetLocalTime32(
	)
{
	return (DWORD)(time(NULL));
}


BOOL
WINAPI
IsProcessExisted(
	__in LPSTR lpProcess
	)
{
	return (GetProcessIdByNameA(lpProcess) != 0);
}

BOOL
WINAPI
LaunchDefBrowserWithParameterW(
	__in LPWSTR lpUrl
	)
{
	BOOL bRel = FALSE;
	HKEY hDefBrowserPos = NULL; 
	wstring wstrDefBrowserPath = L"iexplore.exe"; 

	WCHAR wszBuffer[MAX_PATH + 1] = {0}; 
	DWORD dwDataSize = sizeof(wszBuffer); 

	if (GetRegistryStringValueW(
		HKEY_CURRENT_USER, 
		L"Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\Ftp\\UserChoice\\", 
		L"Progid", 
		wszBuffer, 
		&dwDataSize))
	{ 
		wstring wstrDefBrowserPos = wszBuffer; 
		wstrDefBrowserPos += L"\\shell\\open\\command\\"; 
		dwDataSize = sizeof(wszBuffer); 

		if (GetRegistryStringValueW(
			HKEY_CLASSES_ROOT, 
			wstrDefBrowserPos.c_str(), 
			NULL,
			wszBuffer, 
			&dwDataSize))
		{ 
			// 解出exe 路径. 
			wstrDefBrowserPath = wszBuffer; 
			wstring::size_type leftQuotation = wstrDefBrowserPath.find(L'"'); 
			if (leftQuotation != wstring::npos) 
			{ 
				wstring::size_type rightQuotation = wstrDefBrowserPath.find(L'"', leftQuotation + 1); 
				if (rightQuotation != wstring::npos) 
				{ 
					wstrDefBrowserPath.assign( 
						wstrDefBrowserPath.begin() + leftQuotation + 1, 
						wstrDefBrowserPath.begin() + rightQuotation 
						); 
				} 
			} 
		} 
	} 

	if (!PathFileExistsW(wstrDefBrowserPath.c_str()))
		wstrDefBrowserPath = L"iexplore.exe";

	// NOTICE 
	// https://msdn.microsoft.com/en-us/library/windows/desktop/bb762153(v=vs.85).aspx
	// If the function succeeds, it returns a value greater than 32. 
	// If the function fails, it returns an error value that indicates the cause of the failure. 
	if (ShellExecuteW( 
		NULL, 
		L"open", 
		wstrDefBrowserPath.c_str(), 
		lpUrl, 
		NULL, 
		SW_SHOWNORMAL) > (HINSTANCE)SE_ERR_DLLNOTFOUND)
	{
		bRel = TRUE;
	}

	return bRel;
}

BOOL
WINAPI
LaunchDefBrowserWithParameterA(
	__in LPSTR lpUrl
	)
{
	BOOL bRel = FALSE;
	LPWSTR lpWurl = NULL;
	if (!lpUrl)
	{
		bRel = LaunchDefBrowserWithParameterW(NULL);
	}
	else
	{
		DWORD len = strlen(lpUrl);
		lpWurl = (LPWSTR)LocalAlloc(LPTR, len + 2);
		if (lpWurl != NULL)
		{
			AnsiStringToUnicodeString(lpUrl, lpWurl, len);
			bRel = LaunchDefBrowserWithParameterW(lpWurl);
			LocalFree(lpWurl);
		}
	}
	
	return bRel;
}

INT  WINAPI  LoadResourceAndLockA(
	HMODULE hModule,
	LPSTR   lpType,
	LPSTR   lpResName,
	LPVOID  *lpResBase
	)
{
	INT         nRel = 0;
	HRSRC		hRes = NULL;
	HGLOBAL		hGlobBase = NULL;
	DWORD		dwSize = 0;
	LPVOID		lpBase = NULL;

	if (NULL == lpResBase)
	{
		return nRel;
	}
	else
	{
		*lpResBase = 0;
	}

	hRes = FindResourceA(hModule,lpResName,lpType);
	if (NULL == hRes)
	{
		return nRel;
	}

	dwSize =  SizeofResource(hModule,hRes);
	if (0 == dwSize)
	{
		return nRel;
	}

	hGlobBase = LoadResource(hModule,hRes);

	if (NULL == hGlobBase)
	{
		return nRel;
	}

	lpBase = LockResource(hGlobBase);

	if (NULL != lpBase)
	{
		nRel = dwSize;
		*lpResBase = lpBase;
	}
	else
	{
		FreeResource(hGlobBase);
	}

	return nRel;

};


INT  WINAPI  LoadResourceAndLockW(
	HMODULE hModule,
	LPWSTR   lpType,
	LPWSTR   lpResName,
	LPVOID  *lpResBase
	)
{
	INT         nRel = 0;
	HRSRC		hRes = NULL;
	HGLOBAL		hGlobBase = NULL;
	DWORD		dwSize = 0;
	LPVOID		lpBase = NULL;

	if (NULL == lpResBase)
	{
		return nRel;
	}
	else
	{
		*lpResBase = 0;
	}

	hRes = FindResourceW(hModule,lpResName,lpType);
	if (NULL == hRes)
	{
		return nRel;
	}

	dwSize =  SizeofResource(hModule,hRes);
	if (0 == dwSize)
	{
		return nRel;
	}

	hGlobBase = LoadResource(hModule,hRes);

	if (NULL == hGlobBase)
	{
		return nRel;
	}

	lpBase = LockResource(hGlobBase);

	if (NULL != lpBase)
	{
		nRel = dwSize;
		*lpResBase = lpBase;
	}
	else
	{
		FreeResource(hGlobBase);
	}


	return nRel;

};


BOOL WINAPI  ResourecToFileA(
	HMODULE hModule,
	LPSTR lpType,
	LPSTR lpResName,
	LPSTR lpFilePath
	)
{
	BOOL		bRel = FALSE;
	INT			nResSize = 0;
	DWORD       dwCnt    = 0;
	LPVOID		lpBuffer = NULL;
	HANDLE      hFile   = NULL;


	nResSize = LoadResourceAndLockA(
		hModule,
		lpType,
		lpResName,
		&lpBuffer
		);

	if (0 == nResSize)
	{
		return bRel;
	}

	hFile =  CreateFileA(
		lpFilePath,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);

	if (INVALID_HANDLE_VALUE != hFile)
	{
		bRel = WriteFile(hFile,lpBuffer,nResSize,&dwCnt,NULL);
		CloseHandle(hFile);
	}

	if (lpBuffer)
	{
		FreeResource((HGLOBAL)lpBuffer);
	}

	return bRel;

}

BOOL WINAPI  ResourecToFileW(
	HMODULE hModule,
	LPWSTR lpType,
	LPWSTR lpResName,
	LPWSTR lpFilePath
	)
{
	BOOL		bRel = FALSE;
	INT			nResSize = 0;
	DWORD       dwCnt    = 0;
	LPVOID		lpBuffer = NULL;
	HANDLE      hFile   = NULL;


	nResSize = LoadResourceAndLockW(
		hModule,
		lpType,
		lpResName,
		&lpBuffer
		);

	if (0 == nResSize)
	{
		return bRel;
	}

	hFile =  CreateFileW(
		lpFilePath,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);

	if (INVALID_HANDLE_VALUE == hFile)
	{
		return bRel;
	}

	bRel = WriteFile(hFile,lpBuffer,nResSize,&dwCnt,NULL);
	CloseHandle(hFile);

	if (lpBuffer)
	{
		FreeResource((HGLOBAL)lpBuffer);
	}

	return bRel;
}

BOOL 
WINAPI 
DisableWow64FsRedirection(
    PVOID* OldValue
	)
{
#ifdef WIN64
	UNREFERENCED_PARAMETER(OldValue);
	return TRUE;
#else
	typedef BOOL (WINAPI * LPWOW64DISABLEWOW64FSREDIRECTION)(PVOID *);

	LPWOW64DISABLEWOW64FSREDIRECTION    fnWow64DisableWow64FsRedirection;
	HMODULE                             kernelMod;
	BOOL                                success = TRUE;

	kernelMod = GetModuleHandleW(L"kernel32");
	if (kernelMod)
	{
		fnWow64DisableWow64FsRedirection = (LPWOW64DISABLEWOW64FSREDIRECTION)GetProcAddress(kernelMod, "Wow64DisableWow64FsRedirection");
		if (fnWow64DisableWow64FsRedirection)
			success = fnWow64DisableWow64FsRedirection(OldValue);
	}

	return success;
#endif
}

BOOL 
WINAPI
RevertWow64FsRedirection(
    PVOID OldValue
	)
{
#ifdef WIN64
	UNREFERENCED_PARAMETER(OldValue);
	return TRUE;
#else
	typedef BOOL (WINAPI * LPWOW64REVERTWOW64FSREDIRECTION)(PVOID);

	LPWOW64REVERTWOW64FSREDIRECTION fnWow64RevertWow64FsRedirection;
	HMODULE                         kernelMod;
	BOOL                            success = TRUE;

	kernelMod = GetModuleHandleW(L"kernel32");
	if (kernelMod)
	{
		fnWow64RevertWow64FsRedirection = (LPWOW64REVERTWOW64FSREDIRECTION)GetProcAddress(kernelMod, "Wow64RevertWow64FsRedirection");
		if (fnWow64RevertWow64FsRedirection)
			success = fnWow64RevertWow64FsRedirection(OldValue);
	}

	return success;
#endif
}

BOOLEAN NtPathToDosPathW(WCHAR *FullNtPath, WCHAR *FullDosPath)
{
	WCHAR DosDevice[4] = { 0 };       //dos设备名最大长度为4  
	WCHAR NtPath[64] = { 0 };       //nt设备名最大长度为64  
	WCHAR *RetStr = NULL;
	size_t NtPathLen = 0;
	if (!FullNtPath || !FullDosPath)
	{
		return FALSE;
	}
	for (short i = 65; i < 26 + 65; i++)
	{
		DosDevice[0] = i;
		DosDevice[1] = L':';
		if (QueryDosDeviceW(DosDevice, NtPath, 64))
		{
			if (NtPath)
			{
				NtPathLen = wcslen(NtPath);
				if (!_wcsnicmp(NtPath, FullNtPath, NtPathLen))
				{
					wcscpy(FullDosPath, DosDevice);
					wcscat(FullDosPath, FullNtPath + NtPathLen);
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

BOOLEAN DosPathToNtPathW(WCHAR *FullDosPath, WCHAR *FullNtPath)
{
	WCHAR DosDevice[4] = { 0 };       //dos设备名最大长度为4  
	WCHAR NtPath[64] = { 0 };       //nt设备名最大长度为64  
	WCHAR *RetStr = NULL;
	size_t NtPathLen = 0;
	if (!FullNtPath || !FullDosPath)
	{
		return FALSE;
	}
	DosDevice[0] = FullDosPath[0];
	DosDevice[1] = L':';
	if (QueryDosDeviceW(DosDevice, NtPath, 64))
	{
		if (NtPath)
		{
			wcscpy(FullNtPath, NtPath);
			wcscat(FullNtPath, FullDosPath + 2);
			return TRUE;
		}
	}
	return FALSE;
}

BOOLEAN NtPathToDosPathA(CHAR *FullNtPath, CHAR *FullDosPath)
{
	CHAR DosDevice[4] = { 0 };       //dos设备名最大长度为4  
	CHAR NtPath[64] = { 0 };       //nt设备名最大长度为64  
	CHAR *RetStr = NULL;
	size_t NtPathLen = 0;
	if (!FullNtPath || !FullDosPath)
	{
		return FALSE;
	}
	for (short i = 65; i < 26 + 65; i++)
	{
		DosDevice[0] = (CHAR)i;
		DosDevice[1] = L':';
		if (QueryDosDeviceA(DosDevice, NtPath, 64))
		{
			if (NtPath)
			{
				NtPathLen = strlen(NtPath);
				if (!_strnicmp(NtPath, FullNtPath, NtPathLen))
				{
					strcpy(FullDosPath, DosDevice);
					strcat(FullDosPath, FullNtPath + NtPathLen);
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

BOOLEAN DosPathToNtPathA(CHAR *FullDosPath, CHAR *FullNtPath)
{
	CHAR DosDevice[4] = { 0 };       //dos设备名最大长度为4  
	CHAR NtPath[64] = { 0 };       //nt设备名最大长度为64  
	CHAR *RetStr = NULL;
	size_t NtPathLen = 0;
	if (!FullNtPath || !FullDosPath)
	{
		return FALSE;
	}
	DosDevice[0] = FullDosPath[0];
	DosDevice[1] = L':';
	if (QueryDosDeviceA(DosDevice, NtPath, 64))
	{
		if (NtPath)
		{
			strcpy(FullNtPath, NtPath);
			strcat(FullNtPath, FullDosPath + 2);
			return TRUE;
		}
	}
	return FALSE;
}
}