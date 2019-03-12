#pragma once
#include <curl/curl.h>
#include <string>
#include <vector>
#include <functional>
#include <atomic>
#include <sqstd/sqfilerw.h>
#include <list>
#include <map>

/*
* �궨�峬ʱʱ��
*/
#define DEFAULT_TIMEOUT          10  //�����ʱ��

/*
* ���ͷ���
*/
enum HttpMethods
{
    METHOD_GET = 0,           //get����  
    METHOD_POST = 1,          //post����  
    METHOD_PUT = 2,           //put����  
    METHOD_DELETE = 3,        //delete����  
};

enum DOWNLOAD_ERRORS
{
    ERR_BAD_FILE_LEN = 300,     // �ļ������쳣
    ERR_FILE_DEL,               // ���ļ�ɾ��ʧ��
    ERR_BAD_HD,                 // ����ľ��
};

struct FormPart
{
	bool is_file;
	bool is_buffer;
	std::string value;
	const std::string& data;
};

struct HttpRqParam
{
    friend class CHttpClient;

    int m_keep_alive_id;        //�������, �Ƿ񱣳ֳ�����, 0 ������ ����ֵ��id����curl����
    std::string m_url;          //�������, �����Url��ַ, ��:http://www.baidu.com 
    std::string m_request;      //�������, Ҫ���������
    std::string m_response;     //�������, ���ص����� 
    std::string m_ca_path;      //�������, ֤��Ŀ¼ ssl��֤ʹ��
    std::vector<std::string> m_custom_headers; //�������, �Զ���HTTPͷ
    int m_http_method;          //��û��post������ϣ��ʹ��post����ʱ���ô˲���,Ĭ��get
    int m_http_code;            //����ֵ��http״̬�룬��ɹ�Ϊ200 ʧ��404��
    int m_err_code;             //CURL�ӿڵĴ�����룬����ο�CURL��ͷ�ļ���CURLE_OK�ǳɹ�
    bool m_need_header;         //����������������Ҫ�������ݵ�HTTPͷ���������ô˲�����ʵ��
    int m_port;                 //�������˿�
    std::string m_redirectUrl;  //������������ض����url(��httpcode����302ʱ��ȡ)
    int m_time_out = DEFAULT_TIMEOUT;       // �������, ����ʱ��Ĭ��10��
	
	typedef std::list<FormPart> FormParts;
	std::map<std::string, FormParts> m_form_param;

    HttpRqParam(const std::string& strUrl,
        const std::string& strPost = "",
        int keep_alive_id = 0,
        HttpMethods http_method = METHOD_GET)
        : m_url(strUrl), m_request(strPost)
        , m_keep_alive_id(keep_alive_id)
        , m_http_method(http_method)
    {
        if (!strPost.empty())
            m_http_method = true;
        m_http_code = -1;
        m_err_code = -1;
        m_need_header = false;
        m_port = 0; // 0 ������
    }

    void Clear()
    {
        m_err_code = -1;
        m_http_code = -1;
        m_response = "";
    }

	/* ��ӱ����� �Զ����ݺ�׺�ж����� û�к�׺����Ϊ�������� 
	   ע:
		  ʹ�ô˽ӿ�ʱ��m_request����(���캯���е�strPost����)��Ч�ˣ�Ҫ��������Ҳ�ô˽ӿ����
	   ʾ��:
		req_param.AddFormData("feedback", "18908199721");
		req_param.AddFormData("contact", "helloworld");
		req_param.AddFormData("v6_id", "600022");
		req_param.AddFormData("files", "E:\\v6dj\\3.1\\product\\personal\\�½��ļ��� (7)\\3.2.0.7621.zip", true);
		req_param.AddFormData("files", "E:\\v6dj\\3.1\\product\\personal\\�½��ļ��� (8)\\3.2.0.7645.zip", true);
		req_param.AddFormData("buffer", "hello.txt", false, true, "����һ�������ĵ�");
	*/
	void AddFormData(const std::string& param_name, const std::string& param_value,
					 bool is_file = false, bool is_buffer = false, const std::string& param_data = "")
	{
		m_form_param[param_name].push_back({ is_file, is_buffer, param_value, param_data });
	}
protected:
    struct curl_slist *m_headers = NULL;
	struct curl_httppost *m_formdata = NULL;
	struct curl_httppost *m_formlast = NULL;
};

typedef std::function<bool(int total, int down)>  ProgressCallback;
typedef std::function<int(int error_code, const std::string&strResponse)> AsyncRqCallback;

struct DownloadParam : HttpRqParam
{
    friend class CHttpClient;
    std::string m_filename;                 // �������,������ļ�·��Ϊ�� ���ǲ�Ҫ�����ļ� ���Ϊ���浽m_strResponse�ڴ���
    ProgressCallback m_call_back = nullptr; // �������,���ؽ��Ȼص����� ����false���Դ������
    std::atomic_int m_file_len = 0;         // Ҫ���ص��ܳ��� �����ѹ����ʽ�������ѹ�������ȣ������յ�m_downloaded�Բ��ϣ�m_downloaded�ǽ�ѹ���
    std::atomic_int m_downloaded = 0;       // �����������صĳ���

    std::shared_ptr<snqu::FileRW>  get_file()
    {
        return file_;
    }

    DownloadParam(const std::string& strUrl, const std::string& strFileName)
        : HttpRqParam(strUrl)
    {
        m_filename = strFileName;
    }
    DownloadParam(const std::string& strUrl, const std::string& strFileName, ProgressCallback cb, int time_out)
        : HttpRqParam(strUrl)
    {
        m_call_back = cb;
        m_time_out = time_out;
        m_filename = strFileName;
    }

protected:
    std::shared_ptr<snqu::FileRW>  file_ = nullptr;
};


const std::string& GetContentType(const std::string& ext_name);