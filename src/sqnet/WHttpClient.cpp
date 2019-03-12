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

        //��ʼ��һ��WinHTTP-session���������1Ϊ�˾��������
		//WINHTTP_ACCESS_TYPE_NO_PROXY�������ߴ����������
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

        //ͨ������������ӵ�����������Ҫָ��������IP�Ͷ˿ںš������ӳɹ������ص�hConnect�����ΪNULL
        hConnect = WinHttpConnect(hSession, url.lpszHostName, url.nPort, 0);
        if (hConnect == NULL) {
            http_rq.m_err_code = GetLastError();
            //printf("WinHttpConnect \r\n");
            break;
        }

        std::wstring method_str;
        // ͨ��hConnect�������һ��hRequest��������ڷ����������ȡ�ӷ��������ص����ݡ�
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

        LPCWSTR ppwszAcceptTypes[2];    // ����һ��WCHAR*ָ������
        ppwszAcceptTypes[0] = L"*/*";
        ppwszAcceptTypes[1] = NULL;     // ���һ��ָ��Ԫ�ر�����NULL����Ȼ�жϲ��˽���λ��
        hRequest = WinHttpOpenRequest(hConnect, method_str.c_str(), url.lpszUrlPath, NULL,
                                      WINHTTP_NO_REFERER, ppwszAcceptTypes, ss_type);
        //���в���2��ʾ����ʽ���˴�ΪPost������3:����Post�ľ����ַ��������ľ����ַΪhttp://192.168.50.112/xxxxx
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

        // ָ�����͵���������
        const void *ss = WINHTTP_NO_REQUEST_DATA;
        if (!http_rq.m_request.empty())
            ss = (const char *)http_rq.m_request.c_str();

        BOOL bResults;
        // ��������
        for (auto& item : http_rq.m_custom_headers)
        {
            std::wstring w_item = snqu::codec::S2W(item);
            bResults = WinHttpAddRequestHeaders(hRequest, w_item.c_str(), w_item.length(), WINHTTP_ADDREQ_FLAG_ADD);
        }

        // ���ó�ʱ
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
            // ��������ɹ���׼�����ܷ�������response��ע�⣺��ʹ�� WinHttpQueryDataAvailable��WinHttpReadDataǰ����ʹ��WinHttpReceiveResponse
            // ����access���������ص�����
            bResults = WinHttpReceiveResponse(hRequest, NULL);
        }

        // ��ȡ�������������ݵ�header��Ϣ����һ����������ȡ�������ݵ��������͡�
        std::wstring HeaderBuffer;
        DWORD dwSize = 0;
        if (bResults)
        {
            // ��ȡheader�ĳ���
            WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF,
                WINHTTP_HEADER_NAME_BY_INDEX, NULL,
                &dwSize, WINHTTP_NO_HEADER_INDEX);

            // ����header�ĳ���Ϊbuffer�����ڴ�ռ�
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                HeaderBuffer.resize((dwSize / sizeof(WCHAR)) + 1);

                // ʹ��WinHttpQueryHeaders��ȡheader��Ϣ
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

        //�����յ�ԭʼ��HTTP����ʱ���Ƚ��䱣�浽char[]buffer�У�Ȼ������WinHttpQueryHearders()��ȡHTTPͷ��
        //�õ����ݵ�Content-Type,������֪�����ݵ�����ASCII����Unicode����������
        //һ����֪������ı��뷽ʽ����ת���ɺ��ʱ�����ַ�������wchar_t[]�С�

        //��ȡ��������������
        DWORD dwDownloaded = 0;         //ʵ����ȡ���ַ���
        wchar_t *pwText = NULL;
        if (bResults)
        {
            do
            {
                // ��ȡ�������ݵĴ�С�����ֽ�Ϊ��λ��
                dwSize = 0;
                if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
                    http_rq.m_err_code = GetLastError();
                    //printf("WinHttpQueryDataAvailable \r\n");
                    break;
                }
                if (!dwSize)    break;  //���ݴ�СΪ0                

                 // ���ݷ������ݵĳ���Ϊbuffer�����ڴ�ռ�
                http_rq.m_response.resize(dwSize + 1);

                // ͨ��WinHttpReadData��ȡ�������ķ�������
                if (!WinHttpReadData(hRequest, &http_rq.m_response[0], dwSize, &dwDownloaded)) {
                    http_rq.m_err_code = GetLastError();
                    //printf("WinHttpReadData \r\n");
                }
                if (!dwDownloaded)
                    break;

            } while (dwSize > 0);

            // ����������ת����ASCII
            http_rq.m_response = snqu::codec::U2A(http_rq.m_response);
            ret = true;

            // ��ȡHTTPCODE
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

    // ���ιر�request��connect��session���
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

        // ����һ���Ự
        hSession = WinHttpOpen(NULL, WINHTTP_ACCESS_TYPE_NO_PROXY, NULL, NULL, 0);
        if (hSession == NULL) {
            dl_rq.m_err_code = GetLastError();
            break;
        }
        
        DWORD dwReadBytes, dwSizeDW = sizeof(dwSizeDW), dwContentSize, dwIndex = 0;
        // ����һ������
        hConnect = WinHttpConnect(hSession, url.lpszHostName, url.nPort, 0);
        if (hConnect == NULL) {
            dl_rq.m_err_code = GetLastError();
            ret = false;
            break;
        }
        // ����һ�������Ȳ�ѯ���ݵĴ�С
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

        // ����һ�����󣬻�ȡ����
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

        // ��ȡHTTPCODE
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

        // ��ȡ����
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

    // ���ιر�request��connect��session���
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);

    delete url_info;

    return ret;
}