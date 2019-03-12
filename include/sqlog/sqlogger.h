// Header for sample that sub-includes original header from src/ folder
#ifndef EASYLOGGING_FOR_SAMPLES_H
#define EASYLOGGING_FOR_SAMPLES_H

// note ： 使用静态库注意预定义 _LIB_LOGGER 宏

#ifdef MYMATHS_EXPORTS
#define MYMATHDLL_EXPORT __declspec(dllexport) 
#else
#define MYMATHDLL_EXPORT __declspec(dllimport) 
#endif

#ifdef _LIB_LOGGER
#undef MYMATHDLL_EXPORT
#define MYMATHDLL_EXPORT

#undef GOOGLE_GLOG_DLL_DECL
#define GOOGLE_GLOG_DLL_DECL
#endif

#define HAS_GOOGLE_LOG
#define GLOG_NO_ABBREVIATED_SEVERITIES

#include <string>
#include <sqstd/sqsysfun.h>
#include <sqstd/singleton.h>
#include <sqlog/vlogging.h>

using namespace snqu;
using namespace std;
using namespace google;

/*
 * brief :  头文件较大，建议加大pch内存，推荐设置 /Zm130
 * 1 - 简单调用 snqu::sqlogger::setupLogger 方法即可, 在某个cpp文件中调用 INITIALIZE_EASYLOGGINGPP 一次
 * 3 - 文件回滚时候自动备份旧数据，并添加文件序号。文件序号步进为1
 * 4 - 针对C风格，每一条数据长度不要超过8K
 * 5 - 模块共享同一实列，先调用 INITIALIZE_NULL_EASYLOGGINGPP，然后调用el::Helpers::setStorage(snqu::getLoggers()) 方法即可
**/

namespace snqu {

static inline std::string getBuffers(const char* fmt, ...)
{
    int final_n, n = ((int)strlen(fmt)) * 2; /* Reserve two times as much as the length of the fmt_str */
    std::unique_ptr<char[]> formatted;
    va_list ap;
    while (1) {
        formatted.reset(new char[n]); /* Wrap the plain char array into the unique_ptr */
        strcpy_s(&formatted[0], n, fmt);
        va_start(ap, fmt);
        final_n = vsnprintf(&formatted[0], n, fmt, ap);
        va_end(ap);
        if (final_n < 0 || final_n >= n)
            n += abs(final_n - n + 1);
        else
            break;
    }
    return std::string(formatted.get());
}

class sqlogger;
class MYMATHDLL_EXPORT sqlogger : public Singleton<sqlogger>
{
public:
    typedef std::function<std::string(const std::string&, const std::string&)> CryptFuncT;
	/*
	 * brief : 日志文件格式配置
	 * @fBaseName 文件名称前缀
	 * @outToConsole 是否输出到控制台
	 * @diffLevelFile 不同等级日志是否需要输入到不同文件中
	 * @fileSizes 单位为MB，默认为10MB（推荐值）
	 * @fnCB 数据加密接口
	**/
	void setupLogger(
		const char* fBaseName,
		bool outToConsole = true,
		bool diffLevelFile = false, 
		int fileSizes = 0,
		bool encrypt = true);

	void stopLogger();

	// 设置是否输出Log数据到调试器(debugview)
	void setLogtoDebuger(bool enable = true);
	
	// 设置日志输出等级, >= level的才输出
	void setLogLevel(int level);

    void setEncryptFunc(CryptFuncT);

    void BindModuleLog(const char* module_name, const char* loggerId);
    const char* GetModuleLog(const char* module_name);
private:
	static std::string loggerEncrypt(const char* data, size_t length);
};

}

#define kDebug INFO
#define kTrace INFO
#define kInfo INFO
#define kWarning WARNING
#define kError ERROR
#define kFatal ERROR

#define SNLOG(level, fmt, ...) \
	LOG(ktLoggerId, level) << snqu::getBuffers(fmt, __VA_ARGS__)

#define SLOG(level) LOG(ktLoggerId, level)

#define CBLOG(level, fmt, ...) \
    if (NULL != sqlogger::instance().GetModuleLog(KLogModuleId)) \
        LOG(sqlogger::instance().GetModuleLog(KLogModuleId), level) << "[" << KLogModuleId << "] " << snqu::getBuffers(fmt, __VA_ARGS__);

#define SQLOGGER sqlogger::instance()
#define SQInitLogger sqlogger::instance().setupLogger
#define SQUninitLogger sqlogger::instance().stopLogger
#define SQConfLogToDebugger sqlogger::instance().setLogtoDebuger
#define SQConfLogLevel sqlogger::instance().setLogLevel
#define CBInitLogger(logid) sqlogger::instance().BindModuleLog(KLogModuleId, logid)

#ifdef _DEBUG
#pragma comment(lib, "tdlogd.lib")
#else
#pragma comment(lib, "tdlog.lib")
#endif //_DEBUG

#pragma warning(disable: 4005)
#endif // EASYLOGGING_FOR_SAMPLES_H