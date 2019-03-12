/*-------------------------------------------------------------------------*/
/*  sqsysfun.h																 */
/*                                                                           */
/*  History                                                                  */
/*      04/29/2015															 */
/*                                                                           */
/*  Author                                                                   */
/*      Guolei                                                               */
/*                                                                           */
/*  Some utility functions                                                   */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*-------------------------------------------------------------------------*/
#include <sqstd/sqtypes.h>
#include "sqstd/sqsysfun.h"
#include <sqstd/sqformat.h>
#include <string>
#include <sqwin/win/sqtools.h>
#include <sqstd/sqstringhelper.h>
#include <TlHelp32.h>
#include <codec/sqcodec.h>
#include <Psapi.h>
using namespace std;


#ifdef _WIN32
#include <Shlwapi.h>
#pragma comment(lib,"shlwapi.lib")
#endif // _WIN32

namespace snqu {

std::string get_execute_file()
{
	char apppath[MAX_PATH] = {0};
	std::string ret = "";
	GetModuleFileNameA(NULL, (LPSTR)apppath, sizeof(apppath) - 1);
	ret = apppath;
	return ret;
}
std::string get_file_data(const std::string& file)
{
    DWORD dwFileSize = 0;
    LPBYTE lpBuffer = NULL;
    BOOL bRel = FALSE;
    DWORD dwHashBuffer = 0;
    std::string file_string = "";

    dwFileSize = GetPathFileSize((char *)file.c_str());
    if (dwFileSize <= 0)
    {
        return file_string;
    }

    lpBuffer = (LPBYTE)LocalAlloc(LPTR, dwFileSize);
    if (!lpBuffer)
    {
        return file_string;
    }

    if (ReadPathFileBuffer((char *)file.c_str(), lpBuffer, dwFileSize))
    {
        return file_string;
    }

    file_string.assign((char*)lpBuffer);

    if (lpBuffer != NULL) 
    {
        LocalFree((HLOCAL)lpBuffer);
    }

    return file_string;
}


std::string generate_rand_token(std::string source)
{
	std::string data = source;
	int value = SQRandIntervalNum(0, 10, 100);
	data += to_string(value);
	return codec::MD5(data);
}

std::string generate_rand_string(bool ul_mix, int length)
{
	std::string retstring("");
	char szRandString[MAX_PATH] = {0};
	for (int i = 0; i < length; i++)
	{
		char ch = 'a';
		if (ul_mix)
		{
			int is_upper = SQRandIntervalNum(i, 0, 2);
			if (is_upper)
				ch = SQRandIntervalNum(i, 'A', 'Z');
			else
				ch = SQRandIntervalNum(i, 'a', 'z');
		}
		else
			ch = SQRandIntervalNum(i, 'a', 'z');
		
		szRandString[i] = ch;
	}

	retstring = szRandString;
	return retstring;
}

void generate_rand_numbers(int maxsize, int count, std::vector<int>& output)
{
	if (maxsize <= 0 || count <= 0)
		return;

	if (maxsize <= count)
	{
		for (int i = 0; i < maxsize; i++)
			output.push_back(i);

		return;
	}

	LARGE_INTEGER seed;
	QueryPerformanceFrequency(&seed);
	QueryPerformanceCounter(&seed);

	int *sequence = new int[maxsize];
	if (sequence != nullptr)
	{
		srand((unsigned int)seed.QuadPart);
		for (int i = 0; i < maxsize; i++)
			sequence[i] = i;

		int end = maxsize - 1;
		for (int i = 0; i < maxsize; i++, end--)
		{
			int k = rand() % (end + 1);
			output.push_back(sequence[k]);
			if (output.size() == count)
				break;

			sequence[k] = sequence[end];
		}
	}

	SAFE_DELETE_ARRAY(sequence);
}

bool release_file(const std::string& path, const std::string& buffer, int32 length, bool create_always)
{
	bool rel = false;
	DWORD create_flag = 0;
	
	if (create_always)
		create_flag = CREATE_ALWAYS;
	else
		create_flag = OPEN_EXISTING;

	if (WritePathFileBuffer((char *)path.c_str(), (uint8 *)buffer.c_str(), 
		length, create_flag, FALSE))
	{
		rel = true;
	}

	return rel;
}

bool run_process_internal(const std::string& app_name, const std::string& cmd_line, 
						  bool sw_show, uint32& pid, HANDLE& process, 
						  const std::string& desktop = "")
{
	BOOL rel = FALSE;
	PROCESS_INFORMATION pi = {0};
	int nShow = sw_show ? SW_SHOW : SW_HIDE;

	if (app_name.empty() && cmd_line.empty())
		return false;

	if (cmd_line.empty())
		rel = WinExecProcessExA(NULL, (char *)app_name.c_str(), nShow, 
								(LPSTR)desktop.c_str(), &pi);
	else
	{
		std::string full_cmdline = app_name;
		full_cmdline.append(" ");
		full_cmdline.append(cmd_line);
		rel = WinExecProcessExA(NULL, (char *)full_cmdline.c_str(), nShow, 
								(LPSTR)desktop.c_str(), &pi);
	}

	if (rel)
	{
		process = pi.hProcess;
		pid = pi.dwProcessId;
	}

	return rel ? true : false;
}

bool run_process(const std::string& app_name, const std::string& cmd_line, 
				 bool sw_show, uint32& pid, HANDLE& process)
{
	return run_process_internal(app_name, 
								cmd_line, 
								sw_show, 
								pid, 
								process);
}

// 在指定的桌面启动进程
bool run_process_as_desktop(const std::string& app_name, const std::string& cmd_line, 
							bool sw_show, uint32& pid, HANDLE& process,
							const std::string& desktop)
{
	return run_process_internal(app_name, 
								cmd_line, 
								sw_show, 
								pid, 
								process,
								desktop);
}

std::string load_file(const std::string& path)
{
	char *buf = nullptr;
	ulong32 size = 0;
	std::string retdata("");

	if (LoadFileToBuffer((char *)path.c_str(), (LPVOID*)&buf, (ulong32*)&size))
	{
		if (buf != nullptr)
		{
			retdata.resize(size, '0');
			memcpy((char *)retdata.data(), buf, size);

			LocalFree((HLOCAL)(buf));
		}
	}

	return retdata;
}

bool save_file(const std::string& path, const std::string& data)
{
	bool rel = false;
	if (WritePathFileBuffer((char *)path.c_str(),
						    (uint8*)data.c_str(), 
							data.size(), 
							CREATE_ALWAYS, FALSE))
	{
		rel = true;
	}

	return rel;
}

bool killprocess(const std::string& process)
{
	bool rel = false;
	uint32 pid = GetProcessIdByNameA((char *)process.c_str());
    if (pid > 0)
    {
        EnablePrivilege(TRUE, "SeDebugPrivilege");
        HANDLE hp = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (hp != NULL)
        {
            if (TerminateProcess(hp, 0))
                rel = true;
        }
    }
    else
        rel = true;

	return rel;
}

std::string get_process_full_path(uint32_t pid)
{
	std::string appPath("");
	char fullPath[MAX_PATH] = {0};
	if (GetProcessFullPath(pid, fullPath, sizeof(fullPath)-1))
	{
		appPath = fullPath;
	}

	return appPath;
}

bool find_process(uint32_t pid)
{
	PROCESSENTRY32  stProcList = {0}; 
	DWORD			dwPid = 0;
	HANDLE			hSnp = NULL;
	bool rel = false;

	hSnp = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (NULL != hSnp)
	{
		stProcList.dwSize = sizeof(stProcList);
		if (Process32First(hSnp, &stProcList))
		{
			do {
				if (stProcList.th32ProcessID == pid)
				{
					rel = true;
					break;
				}
			} while (Process32Next(hSnp, &stProcList));
		}

		CloseHandle(hSnp);
	}

	return rel;
}


};
