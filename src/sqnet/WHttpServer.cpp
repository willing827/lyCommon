#include "sqnet/WHttpServer.h"
#include <sqwin/sqwin.h>
#include <sqnet/sqnetapi.h>
#include <sqlog/sqlog.h>
#include <sqstd/sqstringhelper.h>
#include <http.h>
#include <Objbase.h>
#include <codec/sqcodec.h>
#include <atomic>

#pragma comment(lib, "httpapi.lib")
#pragma comment(lib, "Ws2_32.lib")

namespace snqu{

// 初始化HTTP响应体宏
#define INITIALIZE_HTTP_RESPONSE( resp, status, reason )    \
    do                                                      \
    {                                                       \
        RtlZeroMemory( (resp), sizeof(*(resp)) );           \
        (resp)->StatusCode = (status);                      \
        (resp)->pReason = (reason);                         \
        (resp)->ReasonLength = (USHORT) strlen(reason);     \
    } while (FALSE)

#define ADD_KNOWN_HEADER(Response, HeaderId, RawValue)               \
    do                                                               \
    {                                                                \
        (Response).Headers.KnownHeaders[(HeaderId)].pRawValue =      \
                                                          (RawValue);\
        (Response).Headers.KnownHeaders[(HeaderId)].RawValueLength = \
            (USHORT) strlen(RawValue);                               \
    } while(FALSE)

#define MAX_ULONG_STR ((ULONG) sizeof("4294967295"))

typedef std::map<std::string, URICB>  UriDispatch;

struct WHttpServer::impl
{
	DWORD			m_last_err;
	UriDispatch		m_url_dealer;
	HANDLE          m_hReqQueue = NULL;	// Req Queue

	unsigned short  m_port;
	std::string     m_address;
	bool			m_is_https = false;
	atomic_bool		m_is_stop = true;

	std::vector<HTTP_UNKNOWN_HEADER> m_send_headers;
	std::vector<BYTE> m_recv_buff;
	unsigned int m_recv_buff_len;
	std::thread		m_thread;

	impl()
	{
		m_send_headers.resize(10);
		// 分配一个2 KB缓冲区。 这个大小应该适用于大多数请求。 如果需要，
		// 可以增加缓冲区大小。HTTP_REQUEST结构也需要空间。
		m_recv_buff_len = sizeof(HTTP_REQUEST) + 2048;
		m_recv_buff.resize(m_recv_buff_len);
	}

	std::wstring make_uri_addr(const std::string& uri)
	{
		return codec::S2W(fmt::Format("{0}://{1}:{2}{3}", m_is_https ? "https" : "http", m_address, m_port, uri));
	}

	std::string uri_parse(const wchar_t* abs_uri)
	{
		// /test?param=1 要做处理
		auto tmp_uri = codec::W2S(abs_uri);
		auto param_pos = tmp_uri.find('?');
		if (param_pos != std::string::npos)
		{
			tmp_uri = tmp_uri.substr(0, param_pos);
		}
		return tmp_uri;
	}

	URICB find_uri_dealer(const wchar_t* abs_uri)
	{
		auto tmp_uri = uri_parse(abs_uri);
		auto item = m_url_dealer.find(tmp_uri);
		if (item != m_url_dealer.end())
		{
			return item->second;
		}
		return nullptr;
	}

	HttpReqPtr request_to_req(PHTTP_REQUEST pRequest)
	{
		auto req = std::make_shared<HttpReq>();
		req->m_req_uri = codec::W2S(pRequest->CookedUrl.pFullUrl);

		if (pRequest->CookedUrl.pQueryString)
		{// 解析URL里的参数 URLencode怎么判断
			req->m_req_ext = codec::W2S(pRequest->CookedUrl.pQueryString);
			str::trim(req->m_req_ext, "?");
			auto params = str::split(req->m_req_ext, "&");
			std::vector<std::string> param_one;
			for (auto& item : params)
			{
				param_one.clear();
				param_one = str::split(item, "=");
				if (param_one.size() > 1)
					req->m_params.insert(std::make_pair(param_one[0], param_one[1]));
			}
		}
		// 解析私有header
		PHTTP_UNKNOWN_HEADER phd = pRequest->Headers.pUnknownHeaders;
		for (int i = 0; i < pRequest->Headers.UnknownHeaderCount; i++, phd++)
		{
			req->m_headers.insert(std::make_pair(phd->pName, phd->pRawValue));
		}
		//公共的header不处理
// 		for (int i = 0; i < HttpHeaderRequestMaximum; i++)
// 		{
// 			pRequest->Headers.KnownHeaders[i].pRawValue;
// 			if (0 == pRequest->Headers.KnownHeaders[i].RawValueLength)
// 				break;
// 		}
		
		if (pRequest->EntityChunkCount > 0 && pRequest->pEntityChunks)
		{// post data
			req->m_post_data.assign((char*)pRequest->pEntityChunks->FromMemory.pBuffer, pRequest->pEntityChunks->FromMemory.BufferLength);
		}
		
		return req;
	}

	// send a file back
	void send_file_resp(IN HANDLE hReqQueue, IN PHTTP_REQUEST pRequest)
	{
		HTTP_RESPONSE   response;
		DWORD           result;
		DWORD           bytesSent;
		ULONG           BytesRead;

		TCHAR           szTempName[MAX_PATH + 1];
		CHAR            szContentLength[MAX_ULONG_STR];
		HANDLE          hTempFile;
		HTTP_DATA_CHUNK dataChunk;
		ULONG           TotalBytesRead = 0;

		BytesRead = 0;
		hTempFile = INVALID_HANDLE_VALUE;

		do
		{
			//  当HttpReceiveHttpRequest 的flag 未设置HTTP_RECEIVE_REQUEST_FLAG_COPY_BODY时
			//  如果返回的pRequest->Flags & HTTP_REQUEST_FLAG_MORE_ENTITY_BODY_EXISTS
			//  可以用HttpReceiveRequestEntityBody接受剩余数据

			// Initialize the HTTP response structure.
			INITIALIZE_HTTP_RESPONSE(&response, 200, "OK");

			if (result != NO_ERROR)
			{
				wprintf(L"HttpSendHttpResponse failed with %lu \n", result);
				break;
			}

			// Send entity body from a file handle.
			hTempFile = CreateFile(szTempName,
				GENERIC_READ | GENERIC_WRITE,
				0,                  // Do not share.
				NULL,               // No security descriptor.
				CREATE_ALWAYS,      // Overrwrite existing.
				FILE_ATTRIBUTE_NORMAL,    // Normal file.
				NULL);
			if (!hTempFile) break;
			//TotalBytesRead = GetFileLength();

			// sent using chunked transfer encoding, by  
			// passimg "Transfer-Encoding: Chunked".
			// NOTE: Because the TotalBytesread in a ULONG
			//       are accumulated, this will not work
			//       for entity bodies larger than 4 GB. 
			//       For support of large entity bodies,
			//       use a ULONGLONG.
			sprintf_s(szContentLength, MAX_ULONG_STR, "%lu", TotalBytesRead);
			ADD_KNOWN_HEADER(response, HttpHeaderContentLength, szContentLength);

			result = HttpSendHttpResponse(hReqQueue,           // ReqQueueHandle
				pRequest->RequestId, // Request ID
				HTTP_SEND_RESPONSE_FLAG_MORE_DATA,
				&response,       // HTTP response
				NULL,            // pReserved1
				&bytesSent,      // bytes sent-optional
				NULL,            // pReserved2
				0,               // Reserved3
				NULL,            // LPOVERLAPPED
				NULL);            // pReserved4

			if (result != NO_ERROR)
			{
				wprintf(L"HttpSendHttpResponse failed %lu\n", result);
				break;
			}

			dataChunk.DataChunkType = HttpDataChunkFromFileHandle;
			dataChunk.FromFileHandle.ByteRange.StartingOffset.QuadPart = 0;
			dataChunk.FromFileHandle.ByteRange.Length.QuadPart = HTTP_BYTE_RANGE_TO_EOF;
			dataChunk.FromFileHandle.FileHandle = hTempFile;

			result = HttpSendResponseEntityBody(hReqQueue, pRequest->RequestId,
				0,           // This is the last send.
				1,           // Entity Chunk Count.
				&dataChunk, NULL, NULL, 0, NULL, NULL);

			if (result != NO_ERROR)
			{
				wprintf(L"HttpSendResponseEntityBody failed %lu\n", result);
				break;
			}
		} while (0);

		if (hTempFile) CloseHandle(hTempFile);
	}

	int send_data_resp(PHTTP_REQUEST pRequest, unsigned short StatusCode, const std::string& pReason, const HttpRespPtr& resp)
	{
		HTTP_RESPONSE   response;
		HTTP_DATA_CHUNK dataChunk;
		DWORD           result;
		DWORD           bytesSent;
		ULONG FLAG = 0;

		// 初始化HTTP response结构体
		INITIALIZE_HTTP_RESPONSE(&response, StatusCode, pReason.c_str());
		// 添加known header.
		char szDT[128];
		struct tm *newtime;
		time_t ltime;
		time(&ltime);
		newtime = gmtime(&ltime);
		strftime(szDT, 128, "%a, %d %b %Y %H:%M:%S GMT", newtime);
		ADD_KNOWN_HEADER(response, HttpHeaderContentType, "application/json; charset=UTF-8");
		ADD_KNOWN_HEADER(response, HttpHeaderDate, szDT);
		ADD_KNOWN_HEADER(response, HttpHeaderKeepAlive, resp->m_is_keep_alive ? "Keep-Alive" : "close");
		ADD_KNOWN_HEADER(response, HttpHeaderAcceptRanges, "bytes");
		ADD_KNOWN_HEADER(response, HttpHeaderServer, "simpleWSvr");
		auto len_str = std::to_string(resp->m_resp_data.length());
		ADD_KNOWN_HEADER(response, HttpHeaderContentLength, len_str.c_str());
		resp->m_headers.emplace("Access-Control-Allow-Origin", "*");

		if (resp)
		{
			if (resp->m_headers.size() > 0)
			{
				response.Headers.UnknownHeaderCount = (USHORT)resp->m_headers.size();

				if (m_send_headers.size() < response.Headers.UnknownHeaderCount)
				{
					m_send_headers.resize(response.Headers.UnknownHeaderCount);
				}
				memset(&m_send_headers[0], 0, sizeof(HTTP_UNKNOWN_HEADER) * m_send_headers.size());
				int i = 0;
				for (auto& item : resp->m_headers)
				{
					// 添加一个unknown header.
					m_send_headers[i].pName = item.first.c_str();
					m_send_headers[i].NameLength = (USHORT)item.first.length();
					m_send_headers[i].pRawValue = item.second.c_str();;
					m_send_headers[i].RawValueLength = (USHORT)item.second.length();
					i++;
				}
				response.Headers.pUnknownHeaders = &m_send_headers[0];
			}

			if (!resp->m_resp_data.empty())
			{
				// 添加一个entity chunk.
				dataChunk.DataChunkType = HttpDataChunkFromMemory;
				dataChunk.FromMemory.pBuffer = (void*)resp->m_resp_data.c_str();
				dataChunk.FromMemory.BufferLength = resp->m_resp_data.length();

				response.EntityChunkCount = 1;
				response.pEntityChunks = &dataChunk;
			}
		}

		// 因为entity body在一个调用中发送，所以不需要指定Content-Length。
		result = HttpSendHttpResponse(m_hReqQueue, // ReqQueueHandle
			pRequest->RequestId, // Request ID
			0,                   // Flags
			&response,           // HTTP response
			NULL,                // pReserved1
			&bytesSent,          // bytes sent  (OPTIONAL)
			NULL,                // pReserved2  (must be NULL)
			0,                   // Reserved3   (must be 0)
			NULL,                // LPOVERLAPPED(OPTIONAL)
			NULL);               // pReserved4  (must be NULL)


		if (result != NO_ERROR)
		{
			SNLOG(kError, "HttpSendHttpResponse failed with %lu \n", m_last_err);
		}

		return result;
	}

	void run()
	{
		
	}
};

WHttpServer::WHttpServer()
	:m_impl(new impl)
{}

WHttpServer::~WHttpServer()
{
	m_impl.reset();
}

WHttpServer& WHttpServer::instance()
{
	static WHttpServer global;
	return global;
}

void WHttpServer::RegistCB(const std::string& uri, URICB func)
{
	m_impl->m_url_dealer.emplace(std::make_pair(uri, func));
}
bool WHttpServer::Start(unsigned short listen_port, bool is_local, const std::string& cert_finger_print)
{
	if (is_local)
		m_impl->m_address = "127.0.0.1";
	else
		m_impl->m_address = "0.0.0.0";
	m_impl->m_port = listen_port;
	m_impl->m_is_https = !cert_finger_print.empty();

	// 初始化HTTP Server APIs
	m_impl->m_last_err = HttpInitialize(HTTPAPI_VERSION_1, HTTP_INITIALIZE_SERVER | HTTP_INITIALIZE_CONFIG, NULL);

	if (m_impl->m_last_err != NO_ERROR)
	{
		SNLOG(kError, "HttpInitialize failed with %lu \n", m_impl->m_last_err);
		return false;
	}

	if (!m_impl->m_is_https)
	{// 支持https
		auto pHash = snqu::net::HexToBin(cert_finger_print);
		SOCKADDR_IN sa;
		HTTP_SERVICE_CONFIG_SSL_SET ssl_settings;
		memset(&sa, 0, sizeof(sa));
		sa.sin_addr.S_un.S_addr = inet_addr(m_impl->m_address.c_str());
		sa.sin_family = AF_INET;
		sa.sin_port = htons(m_impl->m_port);
		ssl_settings.KeyDesc.pIpPort = (SOCKADDR*)&sa;
		ssl_settings.ParamDesc.SslHashLength = pHash.length();
		ssl_settings.ParamDesc.pSslHash = (void*)pHash.c_str();
		CLSIDFromString(L"{2C565242-B238-11D3-442D-0008C779D776}", &ssl_settings.ParamDesc.AppId);
		ssl_settings.ParamDesc.pSslCertStoreName = 0; //(PWSTR)L"MY";
		ssl_settings.ParamDesc.DefaultCertCheckMode = 0x4;
		ssl_settings.ParamDesc.DefaultRevocationFreshnessTime = 3600;
		ssl_settings.ParamDesc.DefaultRevocationUrlRetrievalTimeout = 0;
		ssl_settings.ParamDesc.pDefaultSslCtlIdentifier = 0;
		ssl_settings.ParamDesc.pDefaultSslCtlStoreName = 0;
		ssl_settings.ParamDesc.DefaultFlags = HTTP_SERVICE_CONFIG_SSL_FLAG_NEGOTIATE_CLIENT_CERT;
		m_impl->m_last_err = HttpSetServiceConfiguration(NULL, HttpServiceConfigSSLCertInfo, &ssl_settings, sizeof(HTTP_SERVICE_CONFIG_SSL_SET), NULL);
		if (m_impl->m_last_err != NO_ERROR && m_impl->m_last_err != ERROR_ALREADY_EXISTS)
			return false;
	}

	// 创建请求队列句柄
	m_impl->m_last_err = HttpCreateHttpHandle(&m_impl->m_hReqQueue, 0);
	if (m_impl->m_last_err != NO_ERROR)
	{
		SNLOG(kError, "HttpCreateHttpHandle failed with %lu \n", m_impl->m_last_err);
		Stop();
		return false;
	}

	// 命令行参数指定要监听的URI。为每个URI调用HttpAddUrl。
	// URI是一个完全合格的URI，必须包含终止字符(/)
	for (auto& item : m_impl->m_url_dealer)
	{
		std::wstring uri_to_deal = m_impl->make_uri_addr(item.first);
		m_impl->m_last_err = HttpAddUrl(m_impl->m_hReqQueue, uri_to_deal.c_str(),NULL);

		if (m_impl->m_last_err != NO_ERROR)
		{
			SNLOG(kError, "HttpAddUrl failed with %lu \n", m_impl->m_last_err);
			Stop();
		}
	}

	if (m_impl->m_last_err == NO_ERROR)
	{
		m_impl->m_thread = std::thread(std::bind(&WHttpServer::Run, this));
		return true;
	}
	return false;
}

void WHttpServer::Stop()
{
	if (m_impl->m_is_stop) return;

	m_impl->m_is_stop = true;

	if (m_impl->m_hReqQueue)
	{
		// 对所有添加的URI调用HttpRemoveUrl.
		for (auto& item : m_impl->m_url_dealer)
		{
			std::wstring uri_to_deal = m_impl->make_uri_addr(item.first);
			HttpRemoveUrl(m_impl->m_hReqQueue, uri_to_deal.c_str());
		}

		HttpShutdownRequestQueue(m_impl->m_hReqQueue);
		// 关闭请求队列句柄.
		CloseHandle(m_impl->m_hReqQueue);
		m_impl->m_hReqQueue = NULL;
	}

	// 调用HttpTerminate.
	HttpTerminate(HTTP_INITIALIZE_SERVER, NULL);
}

void WHttpServer::Run()
{
	HTTP_REQUEST_ID    requestId;
	DWORD              bytesRead;
	PHTTP_REQUEST      pRequest = NULL;

	// 等待一个新请求. 标记为一个NULL请求ID
	HTTP_SET_NULL_ID(&requestId);
	m_impl->m_is_stop = false;

	while (!m_impl->m_is_stop)
	{
		pRequest = (PHTTP_REQUEST)&m_impl->m_recv_buff[0];
		ZeroMemory((void*)pRequest, m_impl->m_recv_buff_len);

		m_impl->m_last_err = HttpReceiveHttpRequest(m_impl->m_hReqQueue,// Req Queue
			requestId,          // Req ID
			HTTP_RECEIVE_REQUEST_FLAG_COPY_BODY,  // Flags
			pRequest,           // HTTP request buffer
			m_impl->m_recv_buff_len,// req buffer length
			&bytesRead,         // bytes received
			NULL);              // LPOVERLAPPED

		if (NO_ERROR == m_impl->m_last_err)
		{
			// 找到对应的回调函数
			auto handler = m_impl->find_uri_dealer(pRequest->CookedUrl.pAbsPath);
			if (handler && (pRequest->Verb == HttpVerbGET || pRequest->Verb == HttpVerbPOST))
			{
				auto req = m_impl->request_to_req(pRequest);
				auto resp = std::make_shared<HttpResp>();
				handler(req, resp);
				SNLOG(kTrace, "Got a request for %s \n", req->m_req_uri.c_str());
				m_impl->send_data_resp(pRequest, 200, "OK", resp);
			}
			else
			{
				SNLOG(kError, "Got a unknown request for %ws \n", codec::W2S(pRequest->CookedUrl.pFullUrl).c_str());
				m_impl->send_data_resp(pRequest, 503, "Not Implemented", NULL);
				break;
			}
			// 重置请求ID用于处理下一个请求.
			HTTP_SET_NULL_ID(&requestId);
		}
		else if (m_impl->m_last_err == ERROR_MORE_DATA)
		{
			// 输入缓冲区太小，无法容纳请求标头。增加缓冲区大小，再次调用API。
			// 再次调用API时，通过传递RequestID来处理失败的请求。
			// 该RequestID从旧缓冲区读取。
			requestId = pRequest->RequestId;

			// 释放旧的缓冲区并分配一个新的缓冲区。
			m_impl->m_recv_buff_len = bytesRead;
			m_impl->m_recv_buff.resize(m_impl->m_recv_buff_len);
		}
		else if (ERROR_CONNECTION_INVALID == m_impl->m_last_err && !HTTP_IS_NULL_ID(&requestId))
		{
			// 当尝试使用更多缓冲区来处理请求时,TCP连接被对方破坏
			// 继续下一个请求。
			HTTP_SET_NULL_ID(&requestId);
		}
		else
		{
			SNLOG(kError, "HttpReceiveHttpRequest failed with %lu \n", m_impl->m_last_err);
			m_impl->m_is_stop = true;
			break;
		}
	}
}

}