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
		kDebug,	// 只在debug版本输出
	};

#ifdef _WIN32
#define sqfilename(x) strrchr(x,'\\') ? strrchr(x,'\\')+1 : x
#else
#define sqfilename(x) strrchr(x,'/') ? strrchr(x,'/')+1 : x
#endif // _WIN32

	struct LogInitParam
	{
		std::string ext_name;		// 文件扩展名 .log .dat
		std::string path;			// 日志文件目录  logs
		bool is_out_put_dbg_str;	// 是否outputdebugstring windows.
		bool is_out_to_console;		// 输出到控制台
		size_t max_log_file_size;   // 单个文件的大小限制
		size_t max_log_path_size;   // 整个文件目录的的大小
		LOG_LEVEL_TYPE log_level;	// 日志级别（输出>=此日志级别的日志）
        size_t log_time_limit;      // 日志超时时间（单位小时）
		bool is_no_delay;			// 是否对齐写入，false为对齐写入能提高io性能，但漏日志几率更高
		
		std::function<std::string(const std::string& string)> encrypt_func; // 加密函数（按行加密）

		LogInitParam()
		{
			BaseInit();
		}

	private:
		void BaseInit()
		{
			ext_name = ".log";
			path = "logs"; // EXE所在目录\logs 可以指定完整目录D:/temps/logs
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
#define ktDebugSign "[snqu]"	// 调试日志过滤关键
}




#endif // SNQU_LOG_DEF_H