/*---------------------------------------------------------------------------*/
/*  sqlog.h                                                                  */
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
#ifndef SNQU_LOG_H
#define SNQU_LOG_H

#include <string>
#include <sqstd/sqsingleton.h>
#include <stdarg.h>
#include <sqstd/sqformat.h>
#include <sqlog/sqlogdef.h>


namespace snqu {

	namespace logdetail {
		void LogHeaderBuild(int level, const char* file, long line, std::string& ss);
        void OutPutLogStr(const char* module_name, int level, std::string& log_str);
		void DebugLogStr(const char* module_name, int level, std::string& log_str);
#ifndef DISABLE_FORMAT //toolset < 140都不支持变参模版，请定义宏禁用
        template<typename... Args>
        inline void OutPutLog(const char* module_name, int level, const char* file, long line, const char* src, Args... args)
        {
#ifndef _DEBUG
            if (level == kDebug) return;
#endif
            std::string log_str;
            log_str.reserve(3072);// 默认长度太小
            
            LogHeaderBuild(level, file, line, log_str);
            fmt::detail::format(log_str, src, args...);
            log_str += "\r\n";
            OutPutLogStr(module_name, level, log_str);
        }
#endif

#ifndef DISABLE_FORMAT //toolset < 140都不支持变参模版，请定义宏禁用
		template<typename... Args>
		inline void DebugLog(const char* module_name, int level, const char* file, long line, const char* src, Args... args)
		{
			std::string log_str;
			log_str.reserve(3072);// 默认长度太小

			LogHeaderBuild(level, file, line, log_str);
			fmt::detail::format(log_str, src, args...);
			log_str += "\r\n";
			DebugLogStr(module_name, level, log_str);
		}
#endif
        void OutPutLogEx(const char* module_name, int level, const char* file, long line, const char* fmt, ...);
		void DebugLogEx(const char* module_name, int level, const char* file, long line, const char* fmt, ...);
	}

	// 初始化日志 多个模块共用时，第一个初始化的有效
	bool InitLogger(const LogInitParam&);

	// 卸载日志
	void UninitLogger();

    // 增加日志文件
    bool AddLogFile(const char* file_name);

    // 增加日志模块
    bool AddLogModule(const char* module_name, const char* file_name);

	// 获取模块当前的日志文件
	std::string GetCurLogFile(const char* module_name);

    /*日志库内部不启动线程*/
	// 检查日志目录空间占用，请定时检测来进行清理
	void CheckLogPath();

    // 检查日志创建时间，请定时检测来进行清理
    void CheckLogDate();

// 输出日志 接口内自带换行
#ifndef DISABLE_FORMAT 
// {0}{1}{2}格式日志
#define SQLOG(level, fmt_str, ...) snqu::logdetail::OutPutLog(ktLoggerId, level, __FILE__, __LINE__, fmt_str, ##__VA_ARGS__)
// {0}{1}{2}格式日志 输出到调试窗口和dbgview
#define DBGLOGE(level, fmt_str, ...) snqu::logdetail::DebugLog(ktLoggerId, level, __FILE__, __LINE__, fmt_str, ##__VA_ARGS__)
#endif
// printf格式日志
#define SNLOG(level, fmt_str, ...) snqu::logdetail::OutPutLogEx(ktLoggerId, level, __FILE__, __LINE__, fmt_str, ##__VA_ARGS__)
// printf格式日志仅输出到调试窗口和dbgview
#define DBGLOG(level, fmt_str, ...) snqu::logdetail::DebugLogEx(ktLoggerId, level, __FILE__, __LINE__, fmt_str, ##__VA_ARGS__)
}

#endif // SNQU_LOG_H