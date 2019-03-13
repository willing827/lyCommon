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
	// ��ʼ������ ʹ��ǰ�������һ��
	static WHttpServer& instance();

	// �ص�����Ŀǰ��ʵ���ǵ��̣߳��ص���������Ҫ���̣߳���Ӱ������
	void RegistCB(const std::string& uri, URICB func);

	/* 
		�ڲ��������߳�
		@param1 listen_port ����ָ���˿�
		@param2 is_local	true ֻ����������
		@param3 cert_finger_print ����https����ʱʹ�ã���Ҫ�ȵ���֤�鵽ϵͳ����֤���ָ��ֵ���� WSSLCert.h�ṩ����֤��ķ���
	*/
	bool Start(unsigned short listen_port, bool is_local = true, const std::string& cert_finger_print = "");
	void Stop();

private:
	struct impl;
	std::shared_ptr<impl> m_impl;
	// ������������̣���ʱ���̴߳���
	void Run();
};

}