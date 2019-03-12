/*---------------------------------------------------------------------------*/
/*  sqlogdef.h                                                               */
/*                                                                           */
/*  History                                                                  */
/*      05/26/2017  create                                                   */
/*                                                                           */
/*  Author                                                                   */
/*       feng hao                                                            */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*---------------------------------------------------------------------------*/
#ifndef SNQU_LOG_DEF_H
#define SNQU_LOG_DEF_H

#include <string>
#include <functional>

namespace snqu {

	enum LOG_LEVEL_TYPE
	{
		kTrace = 0,
		kInfo,
		kWarning,
		kError,
		kFatal,
		kDebug,	// ֻ��debug�汾���
	};

#ifdef _WIN32
#define sqfilename(x) strrchr(x,'\\') ? strrchr(x,'\\')+1 : x
#else
#define sqfilename(x) strrchr(x,'/') ? strrchr(x,'/')+1 : x
#endif // _WIN32

	struct LogInitParam
	{
		std::string ext_name;		// �ļ���չ�� .log .dat
		std::string path;			// ��־�ļ�Ŀ¼  logs
		bool is_out_put_dbg_str;	// �Ƿ�outputdebugstring windows.
		bool is_out_to_console;		// ���������̨
		size_t max_log_file_size;   // �����ļ��Ĵ�С����
		size_t max_log_path_size;   // �����ļ�Ŀ¼�ĵĴ�С
		LOG_LEVEL_TYPE log_level;	// ��־�������>=����־�������־��
        size_t log_time_limit;      // ��־��ʱʱ�䣨��λСʱ��
		bool is_no_delay;			// �Ƿ����д�룬falseΪ����д�������io���ܣ���©��־���ʸ���
		
		std::function<std::string(const std::string& string)> encrypt_func; // ���ܺ��������м��ܣ�

		LogInitParam()
		{
			BaseInit();
		}

	private:
		void BaseInit()
		{
			ext_name = ".log";
			path = "logs"; // EXE����Ŀ¼\logs ����ָ������Ŀ¼D:/temps/logs
			is_out_to_console = false;
			is_out_put_dbg_str = false;
			max_log_file_size = 10 * 1024 * 1024;	// 10m
			max_log_path_size = 1024 * 1024 * 1024;	// 1G
			log_level = kTrace;
            log_time_limit = 7 * 24;
			is_no_delay = true;
		}
	};

	struct LogModule
	{
		LogModule(const std::string& log_module)
		{
			m_log_module = log_module;
		}

	protected:
		std::string m_log_module;
	};

#define ktLoggerId "default"
#define KLogModuleId "module"
#define ktDebugSign "[snqu]"	// ������־���˹ؼ�
}




#endif // SNQU_LOG_DEF_H