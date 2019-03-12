#pragma once
#include <string>
#include <map>
#include <vector>
#include <functional>

/************************************************************************/
/*  winhttp http server module                                         */
/************************************************************************/


namespace snqu {

struct HttpReq
{
	bool m_is_keep_alive = false;
	bool m_is_post = false;
	std::string m_req_uri;
	std::map<std::string, std::string> m_headers;
	std::map<std::string, std::string> m_params;
	std::string m_post_data;
	std::string m_req_ext;
};

struct HttpResp
{
	int m_ret_code = 200;
	bool m_is_keep_alive = false;
	std::string m_resp_uri;
	std::string m_resp_content;
	std::map<std::string, std::string> m_headers;
	std::string m_resp_data;

	HttpResp()
	{
		m_resp_content = "application/json; charset=UTF-8";
	}

	void BuildHeader(std::string& resp_str);
	void BuildBody();

	std::string to_string()
	{
		std::string resp_str;
		BuildBody();
		BuildHeader(resp_str);
		resp_str += m_resp_data;
		return resp_str;
	}
};

typedef std::shared_ptr<HttpReq> HttpReqPtr;
typedef std::shared_ptr<HttpResp> HttpRespPtr;
typedef std::function<void(const HttpReqPtr req, HttpRespPtr& resp)> URICB;


class WHttpServer
{
public:
	WHttpServer(void);
	~WHttpServer(void);
	// 初始化函数 使用前必须调用一次
	static WHttpServer& instance();

	// 回调处理目前的实现是单线程，回调处理尽量不要卡线程，会影响性能
	void RegistCB(const std::string& uri, URICB func);

	// 内部创建有线程
	bool Start(unsigned short listen_port, bool is_local = true, const std::string& cert_finger_print = "");
	void Stop();

private:
	struct impl;
	std::shared_ptr<impl> m_impl;
	// 处理请求的流程，暂时单线程处理
	void Run();
};

}