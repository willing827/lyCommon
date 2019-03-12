#pragma once
#include "sqhttp.h"

/************************************************************************/
/*  winhttp http request module                                         */
/************************************************************************/

class WHttpClient
{
public:
    WHttpClient(void);
    ~WHttpClient(void);
    // 初始化函数 使用前必须调用一次
    static WHttpClient& instance();

    /**
    * 此请求为同步的，异步请用线程调用
    * @return 返回是否成功 
    */
    bool Request(HttpRqParam &http_rq);

    /**
    * @brief DownLoad请求 下载文件到内存中或文件中，适合下小文件
    同步接口
    */
    bool DownLoad(DownloadParam &dl_rq);

private:
    struct impl;
    std::shared_ptr<impl> m_impl;
};