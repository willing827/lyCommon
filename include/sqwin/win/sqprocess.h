#pragma once
#include <sqwin/sqwin.h>

namespace snqu {

	//根据进程名获取进程ID
	int ProcessID(const std::string& process_name);

	//根据进程名判断进程是否存在
	inline bool ProcessExist(const std::string& process_name)
	{
		return (0 != ProcessID(process_name));
	}

	//跟进进程名进程ID获取进程PE文件的完整文件路径
	std::string ProcessPath(int pid);
	inline std::string ProcessPath(const std::string& process_name)
	{
		return ProcessPath(ProcessID(process_name));
	}

	//通过系统接口杀掉进程
	bool ProcessKill(const std::string& process_name);

	// 在指定的桌面启动进程 (WinExecProcessEx)
	bool ProcessExec(const string& app_name, const string& cmd_line = "", bool sw_show = false, 
				   uint32* pid = NULL, HANDLE* process = NULL, const string& desktop = "");

	// 启动进程
	bool ProcessShell(const string& app_name, const string& cmd_line = "", bool sw_show = false);

	enum TargetType
	{
		WOW_86,
		WOW_64,
		WOW_ERROR
	};
	//通过解析exe文件(magic数)判断进程是x64还是x86
	TargetType ProcessGetWow(ULONG32 ulProcessID);
}
