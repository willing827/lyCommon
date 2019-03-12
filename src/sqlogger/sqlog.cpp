#include <sqlog/sqlog.h>
#include <functional>
#include <sqstd/sqthreadhelper.h>
#include <memory>
#include "../sqminiloger/sqminiloger.h"

namespace snqu {

#define sglmylog SQSingleton<snqu::MiniLogger>::instance()

	namespace logdetail {

        std::atomic_int g_is_log_init = 0; // 0 是初始状态 大于0是初始化成功 -1是已经释放 

		inline bool IsInit()
		{
			return g_is_log_init > 0;
		}

		void LogHeaderBuild(int level, const char* file, long line, std::string& ss)
		{
			char chlevel = 'U';
			switch (level)
			{
			case kDebug:
				chlevel = 'D'; break;
			case kTrace:
				chlevel = 'T'; break;
			case kInfo:
				chlevel = 'I'; break;
			case kError:
				chlevel = 'E'; break;
			case kFatal:
				chlevel = 'F'; break;
            case kWarning:
                chlevel = 'W'; break;
			default:
				break;
			}

			SYSTEMTIME  time;//时间结构声明，这个结构是系统的，
            GetLocalTime(&time);
			/*1+4+1+12+1*/
            ss.resize(24);
            auto thd_id = GetCurThdID();
            if (thd_id > 99999) thd_id = 0;
                    
			sprintf_s(&ss[0], 25, "%c%02d%02d %02d:%02d:%02d.%03d %05d", chlevel, time.wMonth, time.wDay, time.wHour,
				time.wMinute, time.wSecond, time.wMilliseconds, thd_id);

            ss += " ";
			ss += sqfilename(file);
			ss += ":";
			ss += std::to_string(line);
			ss += "] ";
		}

        void OutPutLogStr(const char* module_name, int level, std::string& log_str)
        {
			if (!logdetail::IsInit()) return;
            sglmylog.OutPutStr(module_name, level, log_str);
        }

		void DebugLogStr(const char* module_name, int level, std::string& log_str)
		{
			snqu::MiniLogger::Printf(module_name, level, log_str.c_str(), true, true);
		}

        void OutPutLogEx(const char* module_name, int level, const char* file, long line, const char* fmt, ...)
        {
#ifndef _DEBUG
            if (level == kDebug) return;
#endif
			if (!logdetail::IsInit()) return;

			if (!sglmylog.IsLogLevel(level))
			{
				return;
			}

			int final_n, n = ((int)strlen(fmt)) * 2;
            std::unique_ptr<char[]> formatted;
            va_list ap;
            while (1) 
            {
                formatted.reset(new char[n]);
                va_start(ap, fmt);
                final_n = vsnprintf(&formatted[0], n, fmt, ap);
                va_end(ap);
                if (final_n < 0 || final_n >= n)
                    n += abs(final_n - n + 1);
                else
                    break;
            }
            if (final_n < n)
                formatted[final_n] = '\0';
            std::string log_str;
            log_str.reserve(n);
            logdetail::LogHeaderBuild(level, file, line, log_str);
            log_str.append(formatted.get()).append("\r\n");
			OutPutLogStr(module_name, level, log_str);
        }

		void DebugLogEx(const char* module_name, int level, const char* file, long line, const char* fmt, ...)
		{
			int final_n, n = ((int)strlen(fmt)) * 2;
			std::unique_ptr<char[]> formatted;
			va_list ap;
			while (1)
			{
				formatted.reset(new char[n]);
				va_start(ap, fmt);
				final_n = vsnprintf(&formatted[0], n, fmt, ap);
				va_end(ap);
				if (final_n < 0 || final_n >= n)
					n += abs(final_n - n + 1);
				else
					break;
			}
			if (final_n < n)
				formatted[final_n] = '\0';
			std::string log_str;
			log_str.reserve(n);
			logdetail::LogHeaderBuild(level, file, line, log_str);
			log_str.append(formatted.get()).append("\r\n");
			DebugLogStr(module_name, level, log_str);
		}
	}

bool InitLogger(const LogInitParam& log_param)
{
    if (0 == logdetail::g_is_log_init)
    {
        if (sglmylog.Start(log_param))
        {//第一次初始化 设置参数
            logdetail::g_is_log_init++;
        }
    }
    else if (logdetail::g_is_log_init > 0)
    {// 之后的初始化只是计数
        logdetail::g_is_log_init++;
    }
    else
    {
        return false;
    }
	return logdetail::IsInit();
}

void UninitLogger()
{
	if (logdetail::IsInit())
	{
        logdetail::g_is_log_init--;

        if (0 == logdetail::g_is_log_init)
        {// 所有模块都释放后 清除日志
            sglmylog.Stop();
            logdetail::g_is_log_init = -1;
            SQSingleton<snqu::MiniLogger>::Release();
        }
	}
}

bool AddLogFile(const char* file_name)
{
	if (!logdetail::IsInit())
		return false;

    if (nullptr == file_name)
        return false;
    return sglmylog.AddFile(file_name);
}

bool AddLogModule(const char* module_name, const char* file_name)
{
	if (!logdetail::IsInit())
		return false;

    if (nullptr == module_name || nullptr == file_name)
        return false;
    return sglmylog.AddModule(module_name, file_name);
}

std::string GetCurLogFile(const char* module_name)
{
	if (NULL == module_name || !logdetail::IsInit())
		return "";

	return sglmylog.GetCurLogFileName(module_name);
}

void CheckLogPath()
{
	if (!logdetail::IsInit())
		return;
    sglmylog.CheckLogSpace();
}

void CheckLogDate()
{
	if (!logdetail::IsInit())
		return;

    sglmylog.CheckLogOutDate();
}

}