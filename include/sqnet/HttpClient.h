#ifndef __HTTP_CLIENT_H__  
#define __HTTP_CLIENT_H__  
/************************************************************************/
/*  note: compile with CURL_STATICLIB  definition to use static lib     */
/************************************************************************/
  
#include <string>  
#include <functional>
#include <map>
#include <mutex>
#include <thread>
#include <atomic>
#include <list>
#include <curl/curl.h>
#include <atomic>
#include <vector>
#include <sqstd/sqfilerw.h>
#include "sqhttp.h"

#ifndef CURL_STATICLIB
#pragma comment(lib,"libcurl.lib")
#else
#pragma comment(lib,"libcurl_mt.lib")
#pragma comment(lib,"libcrypto_mt.lib")
#pragma comment(lib,"libssl_mt.lib")
#pragma comment(lib,"Crypt32.lib")
#pragma comment(lib,"legacy_stdio_definitions.lib")
#if _MSC_VER>=1900
#include "stdio.h" 
_ACRTIMP_ALT FILE* __cdecl __acrt_iob_func(unsigned);
#ifdef __cplusplus 
extern "C"
#endif 
inline FILE* __cdecl __iob_func(unsigned i) {
    return __acrt_iob_func(i);
}
#endif
#endif



struct BatchDownloadParam : DownloadParam
{
    friend class CHttpClient;
    std::atomic_int m_pieces   = 4;         // 分成多少片下载

    BatchDownloadParam(const std::string& strUrl, const std::string& strFileName, ProgressCallback cb, int pieces = 4)
        : DownloadParam(strUrl, strFileName, cb, DEFAULT_TIMEOUT) // 分片下载时，超时时间只对获取文件长度有效
    {
        m_call_back = cb;
        m_filename = strFileName;
        m_pieces = pieces;
    }

protected:
    std::shared_ptr<snqu::FileRW>  file_ = nullptr;
};

class CHttpClient  
{  
public:  
    CHttpClient(void);  
    ~CHttpClient(void);  
    // 初始化函数 使用前必须调用一次
	static CHttpClient& instance(); 

    //全局变量初始化，放在主线程中  
    static int globalInit();

    //全局资源释放，放在主线程中  
    static void globalUninit();
  
public:  
	/** 
    * 此请求为同步的，异步请用线程调用
    * @return 返回是否Post成功  0为成功 其它为CURL_CODE
    */  
    bool Request(HttpRqParam &http_rq);
  
    /**
    * @brief DownLoad请求 下载文件到内存中或文件中，适合下小文件
      同步接口
    */
    bool DownLoad(DownloadParam &dl_rq);

    /**
    * @brief BatchDownLoad请求 分片下载文件到内存中或文件中，适合下大文件
      连续5秒下载低于50bytes/s就停止下载报错
    */
    void BatchDownLoad(BatchDownloadParam &dl_rq);

    /**
    * ftp upload
    */
    bool FtpUpload(const std::string& strUrl,
        int port,
        const std::string& user_name,
        const std::string& password,
        const std::string& file_path);

    /**
    * ftp download
    */
    void SftpGet();

public:
    void SetDebug(bool bDebug);  

private:
    struct impl;
    std::shared_ptr<impl> m_impl;
};  

#endif // __HTTP_CLIENT_H__
