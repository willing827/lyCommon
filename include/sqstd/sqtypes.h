/*--------------------------------------------------------------------------*/
/*  sqtypes.h                                                               */
/*  Cross platform related defines                                          */
/*                                                                          */
/*  History                                                                 */
/*      08/02/2013     														*/
/*                                                                          */
/*      GUOLEI																*/
/*																			*/
/*  Copyright (C) 2013 by SNQU network technology Inc.                      */
/*  All rights reserved                                                     */
/*--------------------------------------------------------------------------*/
#if !defined(__SQTYPES_H__)
#define __SQTYPES_H__

/**
/* basic include
*/

#if defined (WIN32)
	#include <ws2tcpip.h>
	#include <IPHlpApi.h>
	#include <Shlwapi.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <fcntl.h>
	#include <string>
    #include <assert.h>
	#include <stdint.h>
 
	using namespace std;

#elif defined(__MWERKS__) 
	#define MACOS
	#define DLLEXPORT
	#define DLLIMPORT

#elif defined(LINUX) || defined(UNIX)
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <fcntl.h>
	#include <unistd.h>
	#include <errno.h>
	#include <limits.h>
	#include <signal.h>
    #include <sys/poll.h>
    #include <sys/types.h>
	#include <sys/stat.h>
    #include <pthread.h>
	#include <netdb.h>
	#include <arpa/inet.h>

	#define DLLEXPORT
	#define DLLIMPORT
#endif

#include <functional>
#include <memory>
namespace snqu {
/**
/* Primitive data types
*/


#if defined (WIN32)
	/*
#if !defined(LONGLONG)
typedef __int64 LONGLONG;
#endif 
//*/
/*
#if !defined(ULONGLONG)
typedef unsigned __int64 ULONGLONG;
#endif
//*/
#if !defined(int64)
typedef LONGLONG int64;
#endif

#if !defined(uint64)
typedef ULONGLONG uint64;
#endif

#elif defined(LINUX) || defined(UNIX)

#if !defined(LONGLONG)
typedef long long LONGLONG;
#endif 

#if !defined(ULONGLONG)
typedef unsigned long long ULONGLONG;
#endif

#if !defined(int64)
typedef long long int64;
#endif

#if !defined(uint64)
typedef unsigned long long uint64;
#endif

#endif // WIN32


#if !defined(int8)
typedef char int8;
#endif

#if !defined(uint8)
typedef unsigned char uint8;
#endif

#if !defined(int16)
typedef short int16;
#endif

#if !defined(uint16)
typedef unsigned short uint16;
#endif

#if !defined(int32)
typedef int int32;
#endif

#if !defined(uint32)
typedef unsigned int uint32;
#endif

#if !defined(ulong32)
typedef unsigned long ulong32;
#endif

#if !defined(float32)
typedef float float32;
#endif

#if !defined(float64)
typedef double float64;
#endif

#if !defined(boolean)
typedef uint8 boolean;
#endif

#if !defined(wchar)
typedef unsigned short wchar;
#endif

#if !defined(TRUE)
#define TRUE ((boolean)1)
#endif

#if !defined(FALSE)
#define FALSE ((boolean)0)
#endif

#if !defined(NULL)
#define NULL ((int32)0)
#endif


#if !defined (ip_type)
#define ip_type uint32 
#endif

#if !defined(JFPacket_Type)
#define JFPacket_Type uint32
#endif 


#define MAX_ALLOCATED_BUF_SIZE	0x00ffffff


#ifndef LMASSERT
#define LMASSERT(f) assert(f)
#define LMCHECKMEMORY  LMASSERT(_CrtCheckMemory())
#endif //LMASSERT

#define SAFE_DELETE(ptr) if (ptr != NULL) { delete ptr; ptr = NULL; }
#define SAFE_DELETE_ARRAY(ptr) if (ptr != NULL) { delete [] ptr; ptr = NULL; }


// Used to step into the debugger
#define  DebuggerBreak()  __asm { int 3 }

#define TOUPPER(c) ((((c) >= 'a') && ((c) <= 'z')) ? (c)+'A'-'a' : (c))
#define TONIBBLE(c) ((((c) >= 'A')&&((c) <= 'F')) ? (((c)-'A')+10) : ((c)-'0'))
#define BYTES_TO_KBPS(n) (float)(((((float)n)*8.0f)/1024.0f))

#define isSJIS(a,b) ((a >= 0x81 && a <= 0x9f || a >= 0xe0 && a<=0xfc) && (b >= 0x40 && b <= 0x7e || b >= 0x80 && b<=0xfc))
#define isEUC(a) (a >= 0xa1 && a <= 0xfe)
#define isASCII(a) (a <= 0x7f) 
#define isPLAINASCII(a) (((a >= '0') && (a <= '9')) || ((a >= 'a') && (a <= 'z')) || ((a >= 'A') && (a <= 'Z')))
#define isUTF8(a,b) ((a & 0xc0) == 0xc0 && (b & 0x80) == 0x80 )
#define isESCAPE(a,b) ((a == '&') && (b == '#'))
#define isHTMLSPECIAL(a) ((a == '&') || (a == '\"') || (a == '\'') || (a == '<') || (a == '>'))


#define SWAP16(v) ( ((v&0xff)<<8) | ((v&0xff00)>>8) )
#define SWAP24(v) (((v&0xff)<<16) | ((v&0xff00)) | ((v&0xff0000)>>16) )
#define SWAP32(v) (((v&0xff)<<24) | ((v&0xff00)<<8) | ((v&0xff0000)>>8) | ((v&0xff000000)>>24))
#define SWAP64(v) ((SWAP24(v)|((uint64)SWAP24(v+4)<< 32)))

#ifndef LOBYTE
#define LOBYTE(w) ((uint8)(((uint32)(w)) & 0xff))
#endif 

#if _BIG_ENDIAN
#define CHECK_ENDIAN16(v) v=SWAP16(v)
#define CHECK_ENDIAN24(v) v=SWAP24(v)
#define CHECK_ENDIAN32(v) v=SWAP32(v)
#define CHECK_ENDIAN64(v) v=SWAP64(v)
#else//!_BIG_ENDIAN
#define CHECK_ENDIAN16
#define CHECK_ENDIAN24
#define CHECK_ENDIAN32
#define CHECK_ENDIAN64
#endif //_BIG_ENDIAN

}

/**
/* basic ansi functions
*/
#if defined (WIN32)
	typedef SOCKET	socket_handle;
	#define SQ_INVALID_SOCKET	INVALID_SOCKET
	#define __socket_error_code WSAGetLastError
	#define __get_last_error	(int32)GetLastError
	#define get_tick_count		GetTickCount
	#define zero_memory(p, s)	memset(p, 0, s)
#elif defined(__MWERKS__)

#elif defined(LINUX) || defined(UNIX)
	#define stricmp strcasecmp
	typedef int	socket_handle;
	#define LM_INVALID_SOCKET	-1
	#define __socket_error_code 
	#define __get_last_error
	#define get_tick_count		
	#define zero_memory(p, s)	memset(p, 0, s)
#endif


#define __critical_code_start do {
#define __critical_code_end   } while (0);

// #define __try_code_start	\
// 	try {
// 
// #define __try_code_end(e)	\
// 	}						\
// 	catch (...)				\
// 	{						\
// 		e;					\
// 	}
	#define __try_code_start
	#define __try_code_end(e)
#endif //__SQTYPES_H__
