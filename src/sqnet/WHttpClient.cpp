#include <sqnet/WHttpClient.h>
#include <sqwin/sqwin.h>
#include <codec/sqcodec.h>
#include <sqstd/sqstringhelper.h>
#include <winhttp.h>
#include <sqstd/sqfilerw.h>
#pragma comment(lib, "winhttp.lib")

typedef struct _URL_INFO
{
    WCHAR szScheme[512];
    WCHAR szHostName[512];
    WCHAR szUserName[512];
    WCHAR szPassword[512];
    WCHAR szUrlPath[512];
    WCHAR szExtraInfo[512];
}URL_INFO, *PURL_INFO;

struct WHttpClient::impl
{
	bool LoadAndSetCa(const std::string& file_path, const HINTERNET& hRequest, const std::wstring& pwd)
	{
		//
		TCHAR szPath[256] = { 0 };
		::GetModuleFileName(NULL, szPath, 255);
		(_tcsrchr(szPath, _T('\\')))[1] = 0;

		std::string file_data;
		if (!snqu::FileRW::Read(file_path, file_data))
		{
			return false;
		}

		// Convert a.pfx file image to a Certificate store
		CRYPT_DATA_BLOB PFX;
		PFX.pbData = (BYTE *)&file_data[0];
		PFX.cbData = file_data.size();

		HCERTSTORE pfxStore = ::PFXImportCertStore(&PFX, pwd.c_str(), 0);
		if (NULL == pfxStore)
		{
			_tprintf(_T("PFXImportCertStore error %d\n"), ::GetLastError());
			return false;
		}

		// Extract the certificate from the store and pass it to WinHttp
		PCCERT_CONTEXT pcontext = NULL, clientCertContext = NULL;
		while (pcontext = ::CertEnumCertificatesInStore(pfxStore, pcontext)) {
			clientCertContext = ::CertDuplicateCertificateContext(pcontext); // CertEnumCertificatesInStore frees its passed in pcontext !

			BOOL stat = ::WinHttpSetOption(hRequest, WINHTTP_OPTION_CLIENT_CERT_CONTEXT, (LPVOID)clientCertContext, sizeof(CERT_CONTEXT));
			if (FALSE == stat)
			{
				_tprintf(_T("WinHttpSetOption error %d\n"), ::GetLastError());
				CertCloseStore(pfxStore, 0);
				CertFreeCertificateContext(clientCertContext);
				return false;
			}
			else
			{
				break;//success
			}
		}

		CertCloseStore(pfxStore, 0);
		CertFreeCertificateContext(clientCertContext);
		return true;
	}
};


WHttpClient::WHttpClient(void)
    : m_impl(new impl)
{
}

WHttpClient::~WHttpClient(void)
{
	m_impl = nullptr;
}

WHttpClient& WHttpClient::instance()
{
    static WHttpClient _instance;
    return _instance;
}

bool WHttpClient::Request(HttpRqParam &http_rq)
{
    bool ret = false;
    URL_INFO* url_info = new URL_INFO();
    memset((char*)url_info, 0, sizeof(URL_INFO));

    URL_COMPONENTSW url = { 0 };
    url.dwStructSize = sizeof(url);
    url.lpszExtraInfo = url_info->szExtraInfo;
    url.lpszHostName = url_info->szHostName;
    url.lpszPassword = url_info->szPassword;
    url.lpszScheme = url_info->szScheme;
    url.lpszUrlPath = url_info->szUrlPath;
    url.lpszUserName = url_info->szUserName;

    url.dwExtraInfoLength =
        url.dwHostNameLength =
        url.dwPasswordLength =
        url.dwSchemeLength =
        url.dwUrlPathLength =
        url.dwUserNameLength = 512;

    HINTERNET hSession = NULL;
    HINTERNET hConnect = NULL;
    HINTERNET hRequest = NULL;

    if (http_rq.m_custom_headers.empty())
    {
        http_rq.m_custom_headers.push_back("Content-Type:application/x-www-form-urlencoded");
    }

    do
    {
        if (!WinHttpCrackUrl(snqu::codec::S2W(http_rq.m_url).c_str(), 0, ICU_ESCAPE, &url))
        {
            http_rq.m_err_code = GetLastError();
            //printf("WinHttpCrackUrl \r\n");
            break;
        }

        //初始化一个WinHTTP-session句柄，参数1为此句柄的名称
		//WINHTTP_ACCESS_TYPE_NO_PROXY，不会走代理解析域名
        hSession = WinHttpOpen(NULL, WINHTTP_ACCESS_TYPE_NO_PROXY, NULL, NULL, NULL);
        if (hSession == NULL) {
            http_rq.m_err_code = GetLastError();
            //printf("WinHttpOpen \r\n");
            break;
        }

        if (0 != http_rq.m_port)
        {
            url.nPort = http_rq.m_port;
        }

        //通过上述句柄连接到服务器，需要指定服务器IP和端口号。若连接成功，返回的hConnect句柄不为NULL
        hConnect = WinHttpConnect(hSession, url.lpszHostName, url.nPort, 0);
        if (hConnect == NULL) {
            http_rq.m_err_code = GetLastError();
            //printf("WinHttpConnect \r\n");
            break;
        }

        std::wstring method_str;
        // 通过hConnect句柄创建一个hRequest句柄，用于发送数据与读取从服务器返回的数据。
        switch (http_rq.m_http_method)
        {
        case METHOD_GET:
            method_str = L"GET";
            break;
        case METHOD_POST:
            method_str = L"POST";
            break;
        case METHOD_DELETE:
            method_str = L"DELETE";
            break;
        case METHOD_PUT:
            method_str = L"PUT";
            break;
        default:
            break;
        }

        DWORD ss_type = 0;
        if (snqu::str::nequal(http_rq.m_url.substr(0, 5), "https"))
        {
            ss_type = WINHTTP_FLAG_SECURE;
        }

        LPCWSTR ppwszAcceptTypes[2];    // 创建一个WCHAR*指针数据
        ppwszAcceptTypes[0] = L"*/*";
        ppwszAcceptTypes[1] = NULL;     // 最后一个指针元素必须是NULL，不然判断不了结束位置
        hRequest = WinHttpOpenRequest(hConnect, method_str.c_str(), url.lpszUrlPath, NULL,
                                      WINHTTP_NO_REFERER, ppwszAcceptTypes, ss_type);
        //其中参数2表示请求方式，此处为Post；参数3:给定Post的具体地址，如这里的具体地址为http://192.168.50.112/xxxxx
        if (hRequest == NULL) {
            http_rq.m_err_code = GetLastError();
            //printf("WinHttpOpenRequest \r\n");
            break;
        }

        DWORD dwFlags;
        DWORD dwBuffLen = sizeof(dwFlags);
        dwFlags  = SECURITY_FLAG_IGNORE_UNKNOWN_CA;
        dwFlags |= SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
        dwFlags |= SECURITY_FLAG_IGNORE_CERT_CN_INVALID;
        dwFlags |= SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE;

        WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags));
		dwFlags = WINHTTP_DISABLE_KEEP_ALIVE;
		WinHttpSetOption(hRequest, WINHTTP_OPTION_DISABLE_FEATURE, &dwFlags, sizeof(dwFlags));

        // 指定发送的数据内容
        const void *ss = WINHTTP_NO_REQUEST_DATA;
        if (!http_rq.m_request.empty())
            ss = (const char *)http_rq.m_request.c_str();

        BOOL bResults;
        // 发送请求
        for (auto& item : http_rq.m_custom_headers)
        {
            std::wstring w_item = snqu::codec::S2W(item);
            bResults = WinHttpAddRequestHeaders(hRequest, w_item.c_str(), w_item.length(), WINHTTP_ADDREQ_FLAG_ADD);
        }

        // 设置超时
        ::WinHttpSetTimeouts(hRequest, http_rq.m_time_out * 1000, http_rq.m_time_out * 1000, http_rq.m_time_out * 1000, http_rq.m_time_out * 1000);
        bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, -1L, const_cast<void*>(ss),
            http_rq.m_request.length(), http_rq.m_request.length(), 0);
        if (!bResults) {
            http_rq.m_err_code = GetLastError();

			if (ERROR_WINHTTP_RESEND_REQUEST == http_rq.m_err_code)
			{
				printf("WinHttpSendRequest retry \r\n");
			}
			else if (ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED == http_rq.m_err_code)
			{

			}
            //printf("WinHttpSendRequest  %d\r\n", http_rq.m_err_code);
            break;
        }
        else {
            // 发送请求成功则准备接受服务器的response。注意：在使用 WinHttpQueryDataAvailable和WinHttpReadData前必须使用WinHttpReceiveResponse
            // 才能access服务器返回的数据
            bResults = WinHttpReceiveResponse(hRequest, NULL);
        }

        // 获取服务器返回数据的header信息。这一步我用来获取返回数据的数据类型。
        std::wstring HeaderBuffer;
        DWORD dwSize = 0;
        if (bResults)
        {
            // 获取header的长度
            WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF,
                WINHTTP_HEADER_NAME_BY_INDEX, NULL,
                &dwSize, WINHTTP_NO_HEADER_INDEX);

            // 根据header的长度为buffer申请内存空间
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                HeaderBuffer.resize((dwSize / sizeof(WCHAR)) + 1);

                // 使用WinHttpQueryHeaders获取header信息
                bResults = WinHttpQueryHeaders(hRequest,
                    WINHTTP_QUERY_RAW_HEADERS_CRLF,
                    WINHTTP_HEADER_NAME_BY_INDEX,
                    &HeaderBuffer[0], &dwSize,
                    WINHTTP_NO_HEADER_INDEX);
            }
        }
        else
        {
            http_rq.m_err_code = GetLastError();
            //printf("WinHttpReceiveResponse \r\n");
            break;
        }

        //当接收到原始的HTTP数据时，先将其保存到char[]buffer中，然后利用WinHttpQueryHearders()获取HTTP头，
        //得到内容的Content-Type,这样就知道数据到底是ASCII还是Unicode或者其他。
        //一旦你知道具体的编码方式将其转换成合适编码的字符，存入wchar_t[]中。

        //获取服务器返回数据
        DWORD dwDownloaded = 0;         //实际收取的字符数
        wchar_t *pwText = NULL;
        if (bResults)
        {
            do
            {
                // 获取返回数据的大小（以字节为单位）
                dwSize = 0;
                if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
                    http_rq.m_err_code = GetLastError();
                    //printf("WinHttpQueryDataAvailable \r\n");
                    break;
                }
                if (!dwSize)    break;  //数据大小为0                

                 // 根据返回数据的长度为buffer申请内存空间
                http_rq.m_response.resize(dwSize + 1);

                // 通过WinHttpReadData读取服务器的返回数据
                if (!WinHttpReadData(hRequest, &http_rq.m_response[0], dwSize, &dwDownloaded)) {
                    http_rq.m_err_code = GetLastError();
                    //printf("WinHttpReadData \r\n");
                }
                if (!dwDownloaded)
                    break;

            } while (dwSize > 0);

            // 将返回数据转换成ASCII
            http_rq.m_response = snqu::codec::U2A(http_rq.m_response);
            ret = true;

            // 获取HTTPCODE
            DWORD dwStatusCode = 0;
            DWORD dwSize = sizeof(dwStatusCode);
            WinHttpQueryHeaders(hRequest,
                WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                WINHTTP_HEADER_NAME_BY_INDEX,
                &dwStatusCode, &dwSize, WINHTTP_NO_HEADER_INDEX);
            http_rq.m_http_code = dwStatusCode;
            http_rq.m_err_code = 0;
        }
    }while (0);

    // 依次关闭request，connect，session句柄
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);

    delete url_info;

    return ret;
}

bool WHttpClient::DownLoad(DownloadParam &dl_rq)
{
    bool ret = false;
    URL_INFO* url_info = new URL_INFO();
    memset((char*)url_info, 0, sizeof(URL_INFO));

    URL_COMPONENTSW url = { 0 };
    url.dwStructSize = sizeof(url);
    url.lpszExtraInfo = url_info->szExtraInfo;
    url.lpszHostName = url_info->szHostName;
    url.lpszPassword = url_info->szPassword;
    url.lpszScheme = url_info->szScheme;
    url.lpszUrlPath = url_info->szUrlPath;
    url.lpszUserName = url_info->szUserName;

    url.dwExtraInfoLength =
        url.dwHostNameLength =
        url.dwPasswordLength =
        url.dwSchemeLength =
        url.dwUrlPathLength =
        url.dwUserNameLength = 512;

    HINTERNET hSession = NULL;
    HINTERNET hConnect = NULL;
    HINTERNET hRequest = NULL;

    do 
    {
        if (!WinHttpCrackUrl(snqu::codec::S2W(dl_rq.m_url).c_str(), 0, ICU_ESCAPE, &url))
        {
            dl_rq.m_err_code = GetLastError();
            break;
        }

        if (0 != dl_rq.m_port)
        {
            url.nPort = dl_rq.m_port;
        }

        // 创建一个会话
        hSession = WinHttpOpen(NULL, WINHTTP_ACCESS_TYPE_NO_PROXY, NULL, NULL, 0);
        if (hSession == NULL) {
            dl_rq.m_err_code = GetLastError();
            break;
        }
        
        DWORD dwReadBytes, dwSizeDW = sizeof(dwSizeDW), dwContentSize, dwIndex = 0;
        // 创建一个连接
        hConnect = WinHttpConnect(hSession, url.lpszHostName, url.nPort, 0);
        if (hConnect == NULL) {
            dl_rq.m_err_code = GetLastError();
            ret = false;
            break;
        }
        // 创建一个请求，先查询内容的大小
        DWORD ss_type = WINHTTP_FLAG_REFRESH;
        if (snqu::str::nequal(dl_rq.m_url.substr(0, 5), "https"))
        {
            ss_type |= WINHTTP_FLAG_SECURE;
        }
        hRequest = WinHttpOpenRequest(hConnect, L"HEAD", url.lpszUrlPath, L"HTTP/1.1", WINHTTP_NO_REFERER, 
                                                WINHTTP_DEFAULT_ACCEPT_TYPES, ss_type);
        if (hRequest == NULL) {
            dl_rq.m_err_code = GetLastError();
            ret = false;
            break;
        }

        BOOL bResults = FALSE;
        bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
        if (!bResults) {
            dl_rq.m_err_code = GetLastError();
            break;
        }
        bResults = WinHttpReceiveResponse(hRequest, 0);
        if (!bResults) {
            dl_rq.m_err_code = GetLastError();
            break;
        }
        bResults = WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER, NULL, 
            &dwContentSize, &dwSizeDW, &dwIndex);
        if (!bResults) {
            dl_rq.m_err_code = GetLastError();
            break;
        }
        bResults = WinHttpCloseHandle(hRequest);

        // 创建一个请求，获取数据
        hRequest = WinHttpOpenRequest(hConnect, L"GET", url.lpszUrlPath, L"HTTP/1.1", WINHTTP_NO_REFERER, 
            WINHTTP_DEFAULT_ACCEPT_TYPES, ss_type);
        if (hRequest == NULL) {
            dl_rq.m_err_code = GetLastError();
            ret = false;
            break;
        }
        bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
        if (!bResults) {
            dl_rq.m_err_code = GetLastError();
            break;
        }
        bResults = WinHttpReceiveResponse(hRequest, 0);
        if (!bResults) {
            dl_rq.m_err_code = GetLastError();
            break;
        }

        // 获取HTTPCODE
        DWORD dwStatusCode = 0;
        DWORD dwSize = sizeof(dwStatusCode);
        WinHttpQueryHeaders(hRequest,
            WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX,
            &dwStatusCode, &dwSize, WINHTTP_NO_HEADER_INDEX);
        dl_rq.m_http_code = dwStatusCode;

        if (200 != dl_rq.m_http_code)
        {
            ret = false;
            break;
        }

        // 获取数据
        BYTE *pBuffer = NULL;
        pBuffer = new BYTE[dwContentSize];
        ZeroMemory(pBuffer, dwContentSize);
        bResults = WinHttpReadData(hRequest, pBuffer, dwContentSize, &dwReadBytes);
        if (!bResults) {
            delete pBuffer;
            dl_rq.m_err_code = GetLastError();
            break;
        }

        HANDLE hFile = CreateFileA(dl_rq.m_filename.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            WriteFile(hFile, pBuffer, dwContentSize, &dwReadBytes, NULL);
            CloseHandle(hFile);
            ret = true;
            dl_rq.m_err_code = 0;
        }
        else
        {
            dl_rq.m_err_code = GetLastError();
            ret = false;
        }
            
        delete pBuffer;

    } while (0);

    // 依次关闭request，connect，session句柄
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);

    delete url_info;

    return ret;
}