// Header for sample that sub-includes original header from src/ folder
#ifndef EASYLOGGING_FOR_SAMPLES_H
#define EASYLOGGING_FOR_SAMPLES_H

// note �� ʹ�þ�̬��ע��Ԥ���� _LIB_LOGGER ��

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
 * brief :  ͷ�ļ��ϴ󣬽���Ӵ�pch�ڴ棬�Ƽ����� /Zm130
 * 1 - �򵥵��� snqu::sqlogger::setupLogger ��������, ��ĳ��cpp�ļ��е��� INITIALIZE_EASYLOGGINGPP һ��
 * 3 - �ļ��ع�ʱ���Զ����ݾ����ݣ�������ļ���š��ļ���Ų���Ϊ1
 * 4 - ���C���ÿһ�����ݳ��Ȳ�Ҫ����8K
 * 5 - ģ�鹲��ͬһʵ�У��ȵ��� INITIALIZE_NULL_EASYLOGGINGPP��Ȼ�����el::Helpers::setStorage(snqu::getLoggers()) ��������
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
	 * brief : ��־�ļ���ʽ����
	 * @fBaseName �ļ�����ǰ׺
	 * @outToConsole �Ƿ����������̨
	 * @diffLevelFile ��ͬ�ȼ���־�Ƿ���Ҫ���뵽��ͬ�ļ���
	 * @fileSizes ��λΪMB��Ĭ��Ϊ10MB���Ƽ�ֵ��
	 * @fnCB ���ݼ��ܽӿ�
	**/
	void setupLogger(
		const char* fBaseName,
		bool outToConsole = true,
		bool diffLevelFile = false, 
		int fileSizes = 0,
		bool encrypt = true);

	void stopLogger();

	// �����Ƿ����Log���ݵ�������(debugview)
	void setLogtoDebuger(bool enable = true);
	
	// ������־����ȼ�, >= level�Ĳ����
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