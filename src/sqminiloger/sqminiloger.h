/*---------------------------------------------------------------------------*/
/*  sqminiloger.h                                                            */
/*                                                                           */
/*  History                                                                  */
/*      05/27/2017  create                                                   */
/*                                                                           */
/*  Author                                                                   */
/*       feng hao                                                            */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*---------------------------------------------------------------------------*/
#ifndef SNQU_MINI_LOGER_H
#define SNQU_MINI_LOGER_H
#include <sqstd/sqsingleton.h>
#include <sqlog/sqlogdef.h>
#include <atomic>
#include <mutex>
#include <sqstd/sqsafemap.h>

namespace snqu {

struct LogChannel;

/*
	这是个不考虑性能的简单日志库，适合做些简单的工作
*/
class MiniLogger
{
 public:
	 friend SQSingleton<MiniLogger>;
	 ~MiniLogger();

	bool Start(const LogInitParam& param);
	void Stop();
	bool AddFile(const char* log_file);
    bool AddModule(const char* log_module, const char* log_file);
    void OutPutStr(const char* log_module, int level, std::string& log_str);
    static void Printf(const char* log_module, int level, const char* log_str, 
					   bool is_out_to_console, bool is_out_debug);
    void CheckLogSpace();
    void CheckLogOutDate();
	std::string GetCurLogFileName(const char* log_module);
	inline bool IsLogLevel(int level)
	{
		if (level >= m_log_param.log_level)
		{
			return true;
		}
		return false;
	}
	
private:
	MiniLogger();
	std::atomic_bool m_is_stop;
	std::string m_log_dir;
	LogInitParam m_log_param;
    static CriticalSection m_printf_mtx;
    
    typedef std::shared_ptr<LogChannel> LogChannelPtr;
    SafeMap<uint32_t, LogChannelPtr> m_log_channel;
    SafeMap<uint32_t, uint32_t> m_log_module;
	int m_flush_time;
};
		 
}
#endif