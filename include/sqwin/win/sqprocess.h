#pragma once
#include <sqwin/sqwin.h>

namespace snqu {

	//���ݽ�������ȡ����ID
	int ProcessID(const std::string& process_name);

	//���ݽ������жϽ����Ƿ����
	inline bool ProcessExist(const std::string& process_name)
	{
		return (0 != ProcessID(process_name));
	}

	//��������������ID��ȡ����PE�ļ��������ļ�·��
	std::string ProcessPath(int pid);
	inline std::string ProcessPath(const std::string& process_name)
	{
		return ProcessPath(ProcessID(process_name));
	}

	//ͨ��ϵͳ�ӿ�ɱ������
	bool ProcessKill(const std::string& process_name);

	// ��ָ���������������� (WinExecProcessEx)
	bool ProcessExec(const string& app_name, const string& cmd_line = "", bool sw_show = false, 
				   uint32* pid = NULL, HANDLE* process = NULL, const string& desktop = "");

	// ��������
	bool ProcessShell(const string& app_name, const string& cmd_line = "", bool sw_show = false);

	enum TargetType
	{
		WOW_86,
		WOW_64,
		WOW_ERROR
	};
	//ͨ������exe�ļ�(magic��)�жϽ�����x64����x86
	TargetType ProcessGetWow(ULONG32 ulProcessID);
}
