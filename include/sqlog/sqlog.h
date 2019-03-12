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
#ifndef DISABLE_FORMAT //toolset < 140����֧�ֱ��ģ�棬�붨������
        template<typename... Args>
        inline void OutPutLog(const char* module_name, int level, const char* file, long line, const char* src, Args... args)
        {
#ifndef _DEBUG
            if (level == kDebug) return;
#endif
            std::string log_str;
            log_str.reserve(3072);// Ĭ�ϳ���̫С
            
            LogHeaderBuild(level, file, line, log_str);
            fmt::detail::format(log_str, src, args...);
            log_str += "\r\n";
            OutPutLogStr(module_name, level, log_str);
        }
#endif

#ifndef DISABLE_FORMAT //toolset < 140����֧�ֱ��ģ�棬�붨������
		template<typename... Args>
		inline void DebugLog(const char* module_name, int level, const char* file, long line, const char* src, Args... args)
		{
			std::string log_str;
			log_str.reserve(3072);// Ĭ�ϳ���̫С

			LogHeaderBuild(level, file, line, log_str);
			fmt::detail::format(log_str, src, args...);
			log_str += "\r\n";
			DebugLogStr(module_name, level, log_str);
		}
#endif
        void OutPutLogEx(const char* module_name, int level, const char* file, long line, const char* fmt, ...);
		void DebugLogEx(const char* module_name, int level, const char* file, long line, const char* fmt, ...);
	}

	// ��ʼ����־ ���ģ�鹲��ʱ����һ����ʼ������Ч
	bool InitLogger(const LogInitParam&);

	// ж����־
	void UninitLogger();

    // ������־�ļ�
    bool AddLogFile(const char* file_name);

    // ������־ģ��
    bool AddLogModule(const char* module_name, const char* file_name);

	// ��ȡģ�鵱ǰ����־�ļ�
	std::string GetCurLogFile(const char* module_name);

    /*��־���ڲ��������߳�*/
	// �����־Ŀ¼�ռ�ռ�ã��붨ʱ�������������
	void CheckLogPath();

    // �����־����ʱ�䣬�붨ʱ�������������
    void CheckLogDate();

// �����־ �ӿ����Դ�����
#ifndef DISABLE_FORMAT 
// {0}{1}{2}��ʽ��־
#define SQLOG(level, fmt_str, ...) snqu::logdetail::OutPutLog(ktLoggerId, level, __FILE__, __LINE__, fmt_str, ##__VA_ARGS__)
// {0}{1}{2}��ʽ��־ ��������Դ��ں�dbgview
#define DBGLOGE(level, fmt_str, ...) snqu::logdetail::DebugLog(ktLoggerId, level, __FILE__, __LINE__, fmt_str, ##__VA_ARGS__)
#endif
// printf��ʽ��־
#define SNLOG(level, fmt_str, ...) snqu::logdetail::OutPutLogEx(ktLoggerId, level, __FILE__, __LINE__, fmt_str, ##__VA_ARGS__)
// printf��ʽ��־����������Դ��ں�dbgview
#define DBGLOG(level, fmt_str, ...) snqu::logdetail::DebugLogEx(ktLoggerId, level, __FILE__, __LINE__, fmt_str, ##__VA_ARGS__)
}

#endif // SNQU_LOG_H