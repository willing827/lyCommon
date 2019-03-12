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
    // ��ʼ������ ʹ��ǰ�������һ��
    static WHttpClient& instance();

    /**
    * ������Ϊͬ���ģ��첽�����̵߳���
    * @return �����Ƿ�ɹ� 
    */
    bool Request(HttpRqParam &http_rq);

    /**
    * @brief DownLoad���� �����ļ����ڴ��л��ļ��У��ʺ���С�ļ�
    ͬ���ӿ�
    */
    bool DownLoad(DownloadParam &dl_rq);

private:
    struct impl;
    std::shared_ptr<impl> m_impl;
};