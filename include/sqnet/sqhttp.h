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
* 宏定义超时时间
*/
#define DEFAULT_TIMEOUT          10  //请求最长时间

/*
* 发送方法
*/
enum HttpMethods
{
    METHOD_GET = 0,           //get方法  
    METHOD_POST = 1,          //post方法  
    METHOD_PUT = 2,           //put方法  
    METHOD_DELETE = 3,        //delete方法  
};

enum DOWNLOAD_ERRORS
{
    ERR_BAD_FILE_LEN = 300,     // 文件长度异常
    ERR_FILE_DEL,               // 旧文件删除失败
    ERR_BAD_HD,                 // 错误的句柄
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

    int m_keep_alive_id;        //输入参数, 是否保持长连接, 0 不保持 其它值按id保存curl对象
    std::string m_url;          //输入参数, 请求的Url地址, 如:http://www.baidu.com 
    std::string m_request;      //输入参数, 要传输的数据
    std::string m_response;     //输出参数, 返回的内容 
    std::string m_ca_path;      //输入参数, 证书目录 ssl认证使用
    std::vector<std::string> m_custom_headers; //输入参数, 自定义HTTP头
    int m_http_method;          //当没有post数据又希望使用post方法时设置此参数,默认get
    int m_http_code;            //返回值，http状态码，如成功为200 失败404等
    int m_err_code;             //CURL接口的错误的码，具体参考CURL的头文件如CURLE_OK是成功
    bool m_need_header;         //各别加密情况可能需要返回数据的HTTP头，可以设置此参数来实现
    int m_port;                 //服务器端口
    std::string m_redirectUrl;  //输出参数返回重定向的url(当httpcode返回302时获取)
    int m_time_out = DEFAULT_TIMEOUT;       // 输入参数, 请求超时，默认10秒
	
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
        m_port = 0; // 0 不设置
    }

    void Clear()
    {
        m_err_code = -1;
        m_http_code = -1;
        m_response = "";
    }

	/* 添加表单数据 自动根据后缀判断类型 没有后缀的视为参数处理 
	   注:
		  使用此接口时，m_request数据(构造函数中的strPost数据)无效了，要传参数，也用此接口添加
	   示例:
		req_param.AddFormData("feedback", "18908199721");
		req_param.AddFormData("contact", "helloworld");
		req_param.AddFormData("v6_id", "600022");
		req_param.AddFormData("files", "E:\\v6dj\\3.1\\product\\personal\\新建文件夹 (7)\\3.2.0.7621.zip", true);
		req_param.AddFormData("files", "E:\\v6dj\\3.1\\product\\personal\\新建文件夹 (8)\\3.2.0.7645.zip", true);
		req_param.AddFormData("buffer", "hello.txt", false, true, "我是一个测试文档");
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
    std::string m_filename;                 // 输入参数,保存的文件路径为空 就是不要保存文件 会变为保存到m_strResponse内存中
    ProgressCallback m_call_back = nullptr; // 输入参数,下载进度回调函数 返回false可以打断下载
    std::atomic_int m_file_len = 0;         // 要下载的总长度 如果是压缩格式，这个是压缩包长度，与最终的m_downloaded对不上，m_downloaded是解压后的
    std::atomic_int m_downloaded = 0;       // 最终下载下载的长度

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