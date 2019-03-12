#include <sqwin/win/sqprocess.h>
#include <sqwin/win/sqos.h>
#include <Psapi.h>

namespace snqu {

	int ProcessID(const std::string& process_name)
	{
		return GetProcessIdByNameA((LPSTR)process_name.c_str());
	}

	std::string ProcessPath(int pid)
	{
		std::string ret;
		if (0 == pid) return ret;
			
		if (os::IsWinXP())
		{
			// 获取进程完整路径
			CHAR szImagePath[MAX_PATH] = { 0 };
			HANDLE hProcess = NULL;

			hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
			if (!hProcess)
				return ret;

			if (!GetProcessImageFileNameA(hProcess, szImagePath, MAX_PATH))
			{
				CloseHandle(hProcess);
				return ret;
			}

			if (!path::dospath_to_ntpath(szImagePath, ret))
			{
				CloseHandle(hProcess);
				return ret;
			}

			CloseHandle(hProcess);
		}
		else
		{
			// 获取进程完整路径
			char szImagePath[MAX_PATH] = { 0 };
			HANDLE hProcess = NULL;

			hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
			if (!hProcess)
				return ret;

			//vista后的系统可用此函数
			DWORD size = MAX_PATH;
			if (!QueryFullProcessImageNameA(hProcess, NULL, szImagePath, &size))
			{
				CloseHandle(hProcess);
				return ret;
			}
			ret = szImagePath;
			CloseHandle(hProcess);
		}
		return ret;
	}

	bool ProcessKill(const std::string& process_name)
	{
		bool rel = false;
		uint32 pid = GetProcessIdByNameA((char *)process_name.c_str());
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

	bool ProcessExec(const string& app_name, const string& cmd_line, bool sw_show,
		uint32* pid, HANDLE* process, const string& desktop)
	{
		BOOL rel = FALSE;
		PROCESS_INFORMATION pi = { 0 };
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
			if (process)
				*process = pi.hProcess;
			if (pid)
				*pid = pi.dwProcessId;
		}

		return rel ? true : false;
	}

	bool ProcessShell(const string& app_name, const string& cmd_line, bool sw_show)
	{
		SHELLEXECUTEINFOA ShellInfo;

		memset(&ShellInfo, 0, sizeof(ShellInfo));
		ShellInfo.cbSize = sizeof(ShellInfo);
		ShellInfo.hwnd = NULL;
		ShellInfo.lpVerb = "open";
		ShellInfo.lpParameters = cmd_line.c_str();
		ShellInfo.lpDirectory = snqu::path::get_parent_path(app_name).c_str();
		ShellInfo.lpFile = app_name.c_str();
		ShellInfo.nShow = sw_show ? SW_SHOWNORMAL : SW_HIDE;
		ShellInfo.fMask = SEE_MASK_NOCLOSEPROCESS;

		return TRUE == ShellExecuteExA(&ShellInfo);
	}

	TargetType ProcessGetWow(ULONG32  ulProcessID)
	{
		HANDLE  ProcessHandle = INVALID_HANDLE_VALUE;
		ProcessHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, ulProcessID);

		if (ProcessHandle == NULL)
		{
			return WOW_ERROR;
		}
		//获得Exe模块基地址
		ULONG64 ulModuleBaseAddress = NULL;
		HMODULE ModulesHandle[1024] = { 0 };
		DWORD dwReturn = 0;
		if (EnumProcessModules(ProcessHandle, ModulesHandle, sizeof(ModulesHandle), &dwReturn))
		{
			ulModuleBaseAddress = (ULONG64)ModulesHandle[0];
		}

		if (ulModuleBaseAddress == NULL)
		{
			CloseHandle(ProcessHandle);
			return WOW_ERROR;
		}

		IMAGE_DOS_HEADER   DosHeader = { 0 };
		//读取Dos头
		if (ReadProcessMemory(ProcessHandle, (PVOID)ulModuleBaseAddress, &DosHeader, sizeof(IMAGE_DOS_HEADER), NULL) == FALSE)
		{
			CloseHandle(ProcessHandle);
			return WOW_ERROR;
		}

		WORD  wMagic = 0;
		//模块加载基地址+Dos头部e_lfanew成员(PE头相对于文件的偏移 4字节)+标准PE头+4字节
		if (ReadProcessMemory(ProcessHandle, (PVOID)(ulModuleBaseAddress + DosHeader.e_lfanew + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER)), &wMagic, sizeof(WORD), NULL) == FALSE)
		{
			CloseHandle(ProcessHandle);
			return WOW_ERROR;
		}

		CloseHandle(ProcessHandle);
		if (wMagic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)//x64
		{
			return WOW_64;
		}
		else if (wMagic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)//x86
		{
			return WOW_86;
		}

		return WOW_ERROR;
	}
}