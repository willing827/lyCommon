/*--------------------------------------------------------------------------*/
/*  sqshrmem.h																*/
/*	A shared memory class                                                   */
/*  History                                                                 */
/*      07/06/2015															*/
/*                                                                          */
/*  Copyright (C) 2015 by SNQU network technology Inc.                      */
/*  All rights reserved                                                     */
/*--------------------------------------------------------------------------*/
#ifndef __SQSHRMEM_H__
#define __SQSHRMEM_H__

#include <sqstd/sqinc.h>
#include <Sddl.h>
#include <string>


//////////////////////////////////////////////////////////////////////////
#define SQ_SHARED_FILE_NAME		NULL
#define SQ_SHARED_MEMORY_NAME	"_sq_fee_mem"
#define SQ_SHARED_MEMORY_SIZE	1024


namespace snqu { namespace common { 
//////////////////////////////////////////////////////////////////////////

class SQSharedMem
{
public:
    SQSharedMem(){};
    ~SQSharedMem(){};

    virtual void destory() = 0;
    virtual void* get_buffer() = 0;
    virtual uint32 get_size() = 0;
    virtual bool create(const char *filename, const char *mapname, uint32 size)
    {
        return false;
    }
    virtual bool open(uint32 dwAccess, const char *szMapName)
    {
        return false;
    }
};


// SQSharedMemSvr
class SQSharedMemSvr : public SQSharedMem
{
public:
	SQSharedMemSvr();
	SQSharedMemSvr(const char *filename, const char *mapname, uint32 size);
	virtual ~SQSharedMemSvr();

public:
	bool create(const char *filename, const char *mapname, uint32 size);
	void* get_buffer();
	uint32 get_size();
    void destory();

private:
	void init();

protected:
	HANDLE	m_hFile;
	HANDLE	m_hFileMap;
	void*	m_lpFileMapBuffer;

	char	*m_pFileName;
	char	*m_pMapName;
	uint32	m_dwSize;
	int		m_iCreateFlag;
};


//////////////////////////////////////////////////////////////////////////
// SQSharedMemUser
class SQSharedMemUser : public SQSharedMem
{
public:
	SQSharedMemUser();
	virtual ~SQSharedMemUser();
	SQSharedMemUser(uint32 dwAccess, const char *lpMapName);
    void destory();

protected:
	HANDLE	m_hFileMap;
	void*	m_lpFileMapBuffer;
	std::string	m_pMapName;
	int		m_iOpenFlag;

private:
	void init();

public:
	bool open(uint32 dwAccess, const char *szMapName);
	void* get_buffer();
	uint32 get_size();
	BOOL flush(size_t dwNumberOfBytesToFlush=0);
};


//////////////////////////////////////////////////////////////////////////
inline SQSharedMemSvr::SQSharedMemSvr()
{
	init();
}

inline SQSharedMemSvr::SQSharedMemSvr(const char *filename, const char *mapname, uint32 size)
{
	init();
	create(filename, mapname, size);
}

inline SQSharedMemSvr::~SQSharedMemSvr()
{
	destory();
}

inline void SQSharedMemSvr::init()
{
	m_hFile = NULL;
	m_hFileMap = NULL;
	m_lpFileMapBuffer = NULL;

	m_pFileName = NULL;
	m_pMapName = NULL;
	m_dwSize = 0;

	m_iCreateFlag = 0;
}

inline void SQSharedMemSvr::destory()
{
	if (m_lpFileMapBuffer)
	{
		UnmapViewOfFile(m_lpFileMapBuffer);
		m_lpFileMapBuffer = NULL;
	}

	if (m_hFileMap)
	{
		CloseHandle(m_hFileMap);
		m_hFileMap = NULL;
	}

	if (m_hFile && m_hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFile);
		m_hFile = NULL;
	}

	if (m_pFileName)
	{
		free(m_pFileName);
		m_pFileName = NULL;
	}

	if (m_pMapName)
	{
		free(m_pMapName);
		m_pMapName = NULL;
	}

	init();
}

inline bool SQSharedMemSvr::create(const char *szFileName, const char *szMapName, uint32 dwSize)
{
	if (m_iCreateFlag)
		destory();

	if (szFileName)
		m_pFileName = _strdup(szFileName);

	if (szMapName)
		m_pMapName = _strdup(szMapName);
	else 
		m_pMapName = _strdup(SQ_SHARED_MEMORY_NAME);

	if (dwSize > 0)
		m_dwSize = dwSize;
	else 
		m_dwSize = SQ_SHARED_MEMORY_SIZE;

	if (m_pFileName)
	{
		// file
		m_hFile = CreateFileA(
			m_pFileName,
			GENERIC_READ|GENERIC_WRITE,
			FILE_SHARE_READ|FILE_SHARE_WRITE,
			NULL,
			OPEN_ALWAYS,//OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
			);
	}
	else
	{
		// system
		m_hFile = (HANDLE)INVALID_HANDLE_VALUE;
	}

	if (m_hFile)
	{
		//12.24 可知system创建的section 其他不能访问所以
		SECURITY_ATTRIBUTES sa; //允许管理员访问
		sa.nLength=sizeof(SECURITY_ATTRIBUTES);
		sa.bInheritHandle=FALSE;
		LPCSTR szSD= "D:P"
			"(D;OICI;GA;;;BG)"
			"(A;OICI;GA;;;SY)"
			"(A;OICI;GA;;;BA)"
			"(A;OICI;GRGWGX;;;IU)";


		ConvertStringSecurityDescriptorToSecurityDescriptorA(szSD,SDDL_REVISION_1,&(sa.lpSecurityDescriptor),NULL);

		m_hFileMap = CreateFileMappingA(
			m_hFile,
			&sa,
			PAGE_READWRITE,
			0,
			m_dwSize,
			m_pMapName
			);
 
		LocalFree(sa.lpSecurityDescriptor);
	}
    else
        return false;

	if (m_hFileMap)
	{
		m_lpFileMapBuffer = MapViewOfFile(
			m_hFileMap,
			FILE_MAP_ALL_ACCESS,//FILE_MAP_WRITE|FILE_MAP_READ,
			0, 0, m_dwSize);

		ZeroMemory(m_lpFileMapBuffer, m_dwSize);
	}
    else
        return false;

    if(NULL == m_lpFileMapBuffer)
        return false;

	m_iCreateFlag = 1;
   
    return true;
}

inline void* SQSharedMemSvr::get_buffer()
{
	return (m_lpFileMapBuffer)?(m_lpFileMapBuffer):(NULL);
}

inline uint32 SQSharedMemSvr::get_size()
{
	return m_dwSize;
}

//////////////////////////////////////////////////////////////////////////
// JFSharedMemUser
inline SQSharedMemUser::SQSharedMemUser()
{
	init();
}

inline SQSharedMemUser::~SQSharedMemUser()
{
	destory();
}

inline BOOL SQSharedMemUser::flush(size_t dwNumberOfBytesToFlush)
{
	if(!m_lpFileMapBuffer)
		return FALSE;
	return FlushViewOfFile(m_lpFileMapBuffer, dwNumberOfBytesToFlush);
}

inline SQSharedMemUser::SQSharedMemUser(uint32 dwAccess, const char *lpMapName)
{
	init();
	open(dwAccess, lpMapName);
}

inline bool SQSharedMemUser::open(uint32 dwAccess, const char *szMapName)
{
	if (m_iOpenFlag)
		destory();

	if (szMapName)
		m_pMapName = szMapName;
	else m_pMapName = SQ_SHARED_MEMORY_NAME;

	m_hFileMap = OpenFileMappingA(dwAccess, TRUE, m_pMapName.c_str());

	if (m_hFileMap)
	{
		m_lpFileMapBuffer = MapViewOfFile(m_hFileMap, dwAccess, 0, 0, 0);
	}
    else
    {
        int err = ::GetLastError();
        return false;
    }

    if(NULL == m_lpFileMapBuffer)
    {
        return false;
    }
    
	m_iOpenFlag = 1;
    return true;
}

inline void SQSharedMemUser::init()
{
	m_hFileMap = NULL;
	m_lpFileMapBuffer = NULL;

	m_pMapName;

	m_iOpenFlag = 0;
}

inline void SQSharedMemUser::destory()
{
	if (m_lpFileMapBuffer)
	{
		UnmapViewOfFile(m_lpFileMapBuffer);
		m_lpFileMapBuffer = NULL;
	}

	if (m_hFileMap)
	{
		CloseHandle(m_hFileMap);
		m_hFileMap = NULL;
	}

	if (!m_pMapName.empty())
	{
		m_pMapName = "";
	}

	init();
}

inline void* SQSharedMemUser::get_buffer()
{
	return (m_lpFileMapBuffer)?(m_lpFileMapBuffer):(NULL);
}

inline uint32 SQSharedMemUser::get_size()
{
	return 0;
}
}}
#endif //__SQSHRMEM_H__