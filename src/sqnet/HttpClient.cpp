#include <sqnet/HttpClient.h>
#include <curl/curl.h>
#include <string>
#include <sqstd/sqformat.h>
#include <sqstd/sqstringhelper.h>
#include <thread>
#include <sqstd/PieceWrite.h>
#include <sqlog/sqlog.h>
#include <codec/sqcodec.h>
#include <sqwin/win/sqpath.h>
#include <sqstd/sqformat.h>

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"wldap32.lib")


using namespace std;
static bool g_InitStatus = false;

struct PieceParam
{
    PieceWrite* m_piece_writer = NULL;
    BatchDownloadParam* m_dl_param = NULL;
    CURL* m_curl    = NULL;
    int m_err_code  = 0;
    int m_http_code = 0;
    int m_piece_idx = 0;
};
typedef std::shared_ptr<PieceParam> PieceParamPtr;

struct CurlObj
{
    CURL* m_curl = NULL;
    std::recursive_mutex m_keep_alive_mtx;
};

typedef std::unique_ptr<CurlObj> CurlObjptr;

int on_debug(CURL *, curl_infotype itype, char * pData, size_t size, void *)
{
    if (itype == CURLINFO_TEXT)
    {
        //SNLOG(kDebug, "[TEXT]%s\n", pData);
    }
    else if (itype == CURLINFO_HEADER_IN)
    {
        //SNLOG(kDebug, "[HEADER_IN]%s\n", pData);
    }
    else if (itype == CURLINFO_HEADER_OUT)
    {
        //SNLOG(kDebug, "[HEADER_OUT]%s\n", pData);
    }
    else if (itype == CURLINFO_DATA_IN)
    {
        //SNLOG(kDebug, "[DATA_IN]%s\n", pData);
    }
    else if (itype == CURLINFO_DATA_OUT)
    {
        //SNLOG(kDebug,"[DATA_OUT]%s\n", pData);
    }
    return 0;
}

size_t on_write_data(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
    std::string* str = dynamic_cast<std::string*>((std::string *)lpVoid);
    if (NULL == str || NULL == buffer)
    {
        return -1;
    }

    char* pData = (char*)buffer;
    str->append(pData, size * nmemb);
    return nmemb;
}

static size_t save_header(void *ptr, size_t size, size_t nmemb, void *data)
{
    return (size_t)(size * nmemb);
}

static size_t on_download(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
    bool is_continue_ = true;
    DownloadParam* dlparam = static_cast<DownloadParam*>(lpVoid);
    if (NULL == buffer)
    {
        if (dlparam->m_call_back != nullptr)
        {
            dlparam->m_call_back(-1, -1);
        }
        return -1;
    }

    char* pData = (char*)buffer;

    if (NULL != dlparam->get_file())
    {// 没有文件指针不写文件
        is_continue_ = dlparam->get_file()->AlignWrite(pData, size*nmemb);
    }
    else
    {// 没有文件指针就写内存
        dlparam->m_response.append(pData, size * nmemb);
    }

    if (dlparam->m_call_back != NULL && 1 == is_continue_)
    {
        dlparam->m_downloaded += size * nmemb;
        // 这里支持用户打断
        is_continue_ = dlparam->m_call_back(dlparam->m_file_len, dlparam->m_downloaded);
    }

    if (is_continue_)
        return nmemb;

    return -1;
}

static size_t on_piece_download(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
    bool is_continue_ = true;
    PieceParam* dlparam = static_cast<PieceParam*>(lpVoid);
    if (NULL == buffer)
    {
        if (dlparam->m_dl_param->m_call_back != nullptr)
        {
            dlparam->m_dl_param->m_call_back(-1, -1);
        }
        return -1;
    }

    char* pData = (char*)buffer;

    if (NULL != dlparam->m_piece_writer)
    {// 写文件
        is_continue_ = dlparam->m_piece_writer->Write(dlparam->m_piece_idx, (const char*)buffer, size * nmemb);
    }
    else
    {// 没有文件指针就失败
        return -1;
    }

    if (dlparam->m_dl_param->m_call_back != nullptr && 0 == is_continue_)
    {
        dlparam->m_dl_param->m_downloaded += size * nmemb;
        // 这里支持用户打断
        is_continue_ = dlparam->m_dl_param->m_call_back(dlparam->m_dl_param->m_file_len, dlparam->m_dl_param->m_downloaded);
    }

    if (is_continue_)
        return nmemb;

    return -1;
}

struct CHttpClient::impl 
{
    bool m_bDebug_ = false;
    std::map<int, CurlObjptr> m_gl_curl;

    int   m_running_handles = 0;
    map<CURL*, pair<char*, string>> m_post_data;
    list<string> m_content;
    function<int(int error_code, const string& response)> m_post_callback_func = nullptr;

    ~impl() {}

	void AddFormData(HttpRqParam &http_rq)
	{
		for (auto& param_array : http_rq.m_form_param)
		{
			std::vector<struct curl_forms> m_froms;
			std::string content_type, from_name;
			int i = 0;
			for (auto param_item = param_array.second.begin(); param_item != param_array.second.end(); param_item++, i++)
			{
				if (param_array.second.size() > 1)
				{
					from_name = snqu::fmt::FormatEx("%s[%d]", param_array.first.c_str(), i);
				}
				else
					from_name = param_array.first;

				m_froms.push_back({ CURLFORM_COPYNAME, from_name.data() });
				if (param_item->is_buffer) {
					m_froms.push_back({ CURLFORM_BUFFER, param_item->value.data() });
					m_froms.push_back(
						{ CURLFORM_BUFFERPTR, reinterpret_cast<const char*>(param_item->data.data()) });
					m_froms.push_back(
						{ CURLFORM_CONTENTSLENGTH, reinterpret_cast<const char*>(param_item->data.length()) });
				}
				else if (param_item->is_file) {
					m_froms.push_back({ CURLFORM_FILE, param_item->value.data() });
				}
				else {
					m_froms.push_back({ CURLFORM_COPYCONTENTS, param_item->value.data() });
				}
				content_type = snqu::path::get_externtion(param_item->value);
				if (!content_type.empty())
				{
					snqu::str::trim(content_type, ".");
					content_type = GetContentType(content_type);
					m_froms.push_back({ CURLFORM_CONTENTTYPE, content_type.c_str() });
				}
				
				m_froms.push_back({ CURLFORM_END, nullptr });
				curl_formadd(&http_rq.m_formdata, &http_rq.m_formlast, CURLFORM_ARRAY, m_froms.data(), CURLFORM_END);
			}
		}
	}

    void* NormalSet(HttpRqParam &http_rq)
    {
        http_rq.m_err_code = CURLE_FAILED_INIT;

        if (http_rq.m_url.length() < 5)
            return NULL;

        CURL* curl = GetCurl(http_rq.m_keep_alive_id);
        if (NULL == curl)
        {
            return curl;
        }

        if (m_bDebug_)
        {
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
            curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, on_debug);
        }

        /* 访问的地址 */
        curl_easy_setopt(curl, CURLOPT_URL, http_rq.m_url.c_str());
        if (0 != http_rq.m_port)
            curl_easy_setopt(curl, CURLOPT_PORT, http_rq.m_port);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
        // gzip支持
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");

        /* 返回数据读取 */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, on_write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&http_rq.m_response);

        /* 发送的数据 */
        switch (http_rq.m_http_method)
        {
        case METHOD_GET:
            curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
            break;
        case METHOD_POST:
            curl_easy_setopt(curl, CURLOPT_POST, 1);
			if (http_rq.m_form_param.empty())
			{//表单为空才能这样处理
				curl_easy_setopt(curl, CURLOPT_POSTFIELDS, http_rq.m_request.c_str());
				curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, http_rq.m_request.size());
			}
            break;
        case METHOD_PUT:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
			if (http_rq.m_form_param.empty())
			{//表单为空才能这样处理
				curl_easy_setopt(curl, CURLOPT_POSTFIELDS, http_rq.m_request.c_str());
				curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, http_rq.m_request.size());
			}
            break;
        case METHOD_DELETE:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

            break;
        default:
            break;
        }
		// 添加表单数据
		if (!http_rq.m_form_param.empty()) 
		{
			AddFormData(http_rq);
			curl_easy_setopt(curl, CURLOPT_HTTPPOST, http_rq.m_formdata);
		}

        /**
        * 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
        * 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
        */
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
        if (0 != http_rq.m_keep_alive_id)
        {
            /* enable TCP keep-alive for this transfer */
            curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1);
            int tmp_ps = http_rq.m_time_out - 5;
            tmp_ps = tmp_ps > 5 ? tmp_ps : 5;
            /* 空闲多久发送一个心跳包 */
            curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, tmp_ps);
            /* 心跳包间隔多久发一个 */
            curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, tmp_ps);
        }
        /* 超时时间设置 */
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, http_rq.m_time_out);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, http_rq.m_time_out);

        if (!http_rq.m_ca_path.empty())
        {
            //缺省情况就是PEM，所以无需设置，另外支持DER
            //curl_easy_setopt(curl,CURLOPT_SSLCERTTYPE,"PEM");
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
            curl_easy_setopt(curl, CURLOPT_CAINFO, http_rq.m_ca_path.c_str());
        }
        else
        {
            if (snqu::str::nequal(http_rq.m_url.substr(0, 5), "https"))
            {
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
            }
        }

        if (http_rq.m_custom_headers.size() > 0)
        {
            for (auto& pos : http_rq.m_custom_headers)
            {
                http_rq.m_headers = curl_slist_append(http_rq.m_headers, pos.c_str());
            }
            /* set our custom set of headers */
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, http_rq.m_headers);
        }

        if (http_rq.m_need_header)
            curl_easy_setopt(curl, CURLOPT_HEADER, 1);  //Response要求header头

        http_rq.m_err_code = CURLE_OK;

        return curl;
    }

    static int on_curl_writer(char *data, size_t size, size_t nmemb, void *userp)
    {
        std::string* str = dynamic_cast<std::string*>((std::string *)userp);
        if (NULL == str || NULL == data)
        {
            return 0;
        }
        str->append(data, size * nmemb);
        size_t Length = (size_t)(size * nmemb);

        return Length;
    }

    int check_http_code(CURL* curl)
    {
        int http_code = 0;
		int ret = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

        if (!ret)
        {
            ret = http_code;
        }

        return ret;
    }

	int get_redirect_url(CURL* curl, char** rederectUrl)
	{
		int http_code = 0;
		int ret = curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL, rederectUrl);

		if (!ret) {
			ret = http_code;
		}
		return ret;
	}

    bool get_file_length(const std::string& url, int &len_out, int& res)
    {
        len_out = -1;
        auto curl = GetCurl(0);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HEADER, 1);    //只要求header头
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1);    //不需求body
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, save_header);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, save_header);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2);

        if (snqu::str::nequal(url.substr(0, 5), "https"))
        {
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
        }
        res = curl_easy_perform(curl);

        if (res == CURLE_OPERATION_TIMEDOUT)
        {//长度获取失败超时重试一次
            res = curl_easy_perform(curl);
        }
        if (res == CURLE_OK)
        {
            double size = 0.0;
            res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &size);
            len_out = (int)size;
			SNLOG(snqu::kTrace, "get_file_length size[%f] len_out[%d] res[%d", size, len_out, res);
        }
        ClearCurl(curl, 0);
        if (len_out <= 0)
        {
            return false;
        }
        return res == CURLE_OK;
    }

    CURL* GetCurl(int keep_alive_id = 0)
    {
        if (keep_alive_id)
        {
            if (m_gl_curl.count(keep_alive_id) != 0)
            {
                m_gl_curl[keep_alive_id]->m_keep_alive_mtx.lock();
                curl_easy_reset(m_gl_curl[keep_alive_id]->m_curl);
                return m_gl_curl[keep_alive_id]->m_curl;
            }
            else
            {
                auto new_obj = std::make_unique<CurlObj>();
                new_obj->m_curl = curl_easy_init();
                auto new_val = std::make_pair(keep_alive_id, std::move(new_obj));
                m_gl_curl.insert(std::move(new_val));
                m_gl_curl[keep_alive_id]->m_keep_alive_mtx.lock();
                return m_gl_curl[keep_alive_id]->m_curl;
            }
        }

        return curl_easy_init();
    }

    void ClearCurl(CURL*& curl, int keep_alive_id = 0)
    {
        if (keep_alive_id == 0 && curl != NULL)
        {
            if (curl != NULL)
            {
                curl_easy_cleanup(curl);
                curl = NULL;
            }
        }
        else
        {
            if (curl != NULL)
                m_gl_curl[keep_alive_id]->m_keep_alive_mtx.unlock();
        }
    }
};

CHttpClient::CHttpClient(void) 
    : m_impl(new impl)
{
}

CHttpClient::~CHttpClient(void)
{
}

CHttpClient& CHttpClient::instance()
{
	static CHttpClient _instance;
	return _instance;
}

int CHttpClient::globalInit()
{
    if (g_InitStatus)
        return CURLE_OK;

    int nret = curl_global_init(CURL_GLOBAL_ALL);
    if (CURLE_OK == nret)
    {
        //返回成功  
        g_InitStatus = true;
    }

    return nret;
}

void CHttpClient::globalUninit()
{
    if (!g_InitStatus) return;

    curl_global_cleanup();
    //返回成功  
    g_InitStatus = false;
}

bool CHttpClient::Request(HttpRqParam &http_rq)
{
    http_rq.Clear();
	CURL* curl = m_impl->NormalSet(http_rq);
	
	if (NULL == curl)
	{
        http_rq.m_err_code = ERR_BAD_HD;
        m_impl->ClearCurl(curl, http_rq.m_keep_alive_id);
		return false;
	}

    http_rq.m_err_code = curl_easy_perform(curl);
    http_rq.m_http_code = m_impl->check_http_code(curl);

    m_impl->ClearCurl(curl, http_rq.m_keep_alive_id);

	/* free the custom headers */ 
    if (NULL != http_rq.m_headers)
	    curl_slist_free_all(http_rq.m_headers);

	if (NULL != http_rq.m_formdata)
		curl_formfree(http_rq.m_formdata);

	return http_rq.m_err_code == CURLE_OK;
}

bool CHttpClient::DownLoad(DownloadParam &dl_rq)
{
    bool ret = false;
    CURL* curl = NULL;
    do 
    {
        // 取得文件长度
        int file_len = 0;
        if (!m_impl->get_file_length(dl_rq.m_url, file_len, dl_rq.m_err_code))
        {
			SNLOG(snqu::kTrace, "get_file_length failed");
            break;
        }

        if (0 == file_len)
        {
            dl_rq.m_err_code = ERR_BAD_FILE_LEN;
            break;
        }
        dl_rq.m_file_len = file_len;
        curl = m_impl->NormalSet(dl_rq);
        if (NULL == curl)
        {
            dl_rq.m_err_code = ERR_BAD_HD;
            break;
        }
        
        if (!dl_rq.m_filename.empty())
        {
            //重新下载文件
            ::remove(dl_rq.m_filename.c_str());
            auto file = std::make_shared<snqu::FileRW>();
            if (!file->Open(dl_rq.m_filename.c_str()))
            {
				SNLOG(snqu::kTrace, "Open file  failed");
                dl_rq.m_err_code = GetLastError();
                break;
            }
            //删除失败
            if (file->Size() != 0)
            {
                dl_rq.m_err_code = ERR_FILE_DEL;
                break;
            }
                

            dl_rq.file_ = file;
        }
        else
        {
            dl_rq.m_downloaded = dl_rq.m_response.length();
        }
        
        //当HTTP返回值大于等于400的时候，请求失败
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
        auto ccode = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, on_download);
        ccode = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&dl_rq);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, dl_rq.m_time_out);
        //ccode = curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, CURL_MAX_READ_SIZE);

        dl_rq.m_err_code = curl_easy_perform(curl);
        dl_rq.m_http_code = m_impl->check_http_code(curl);
        
        if (dl_rq.m_err_code == CURLE_OK)
        {
            if (dl_rq.m_http_code == 200 || dl_rq.m_http_code == 304 || dl_rq.m_http_code == 204)
            {
                int final_size = 0;
                if (dl_rq.m_filename.empty())
                {//下载到内存中的长度
                    final_size = dl_rq.m_response.size();
                }
                else if (nullptr != dl_rq.file_)
                {//下载到文件长度
                    final_size = dl_rq.file_->Size();
                }

                ret = true;
                //如果文件采用压缩传输这个长度就不正确
//                 if (final_size  == (int)dl_rq.m_file_len)
//                 {
//                     // 下载后的总长度匹配则下载成功
//                     
//                     break;
//                 }
            }
			if (dl_rq.m_http_code == 302)
			{
				char* rederectUrl = { 0 };
				m_impl->get_redirect_url(curl, &rederectUrl);
				dl_rq.m_redirectUrl = rederectUrl;
			}
        }
        else
        {
            dl_rq.m_http_code;
        }

        if (!dl_rq.m_filename.empty() && nullptr != dl_rq.file_)
            dl_rq.file_->Close();

        /* free the custom headers */
        if (NULL != dl_rq.m_headers)
            curl_slist_free_all(dl_rq.m_headers);
        
    } while (0);

    m_impl->ClearCurl(curl, 0);
    return ret;
}

void CHttpClient::BatchDownLoad(BatchDownloadParam &dl_rq)
{
    bool ret = false;
    std::vector<PieceParamPtr> piece_params;
    std::vector<std::thread> thds;
    dl_rq.m_err_code = -1;
    do
    {
        // 取得文件长度
        int file_len = 0;
        if (!m_impl->get_file_length(dl_rq.m_url, file_len, dl_rq.m_err_code))
        {
            break;
        }

        if (0 == file_len)
        {
            dl_rq.m_err_code = ERR_BAD_FILE_LEN;
            break;
        }
            
        dl_rq.m_file_len = file_len;

        //初始化下载缓存
        PieceWrite write_param;
        dl_rq.m_err_code = write_param.BuildFile(dl_rq.m_filename.c_str(), dl_rq.m_file_len, dl_rq.m_pieces);
        if (dl_rq.m_err_code) 
            break;

        for (int i = 0; i < dl_rq.m_pieces; i++)
        {//分片下载
            auto temp_piece = std::make_shared<PieceParam>();
            temp_piece->m_curl = m_impl->NormalSet(dl_rq);
            if (NULL == temp_piece->m_curl) break;
            temp_piece->m_dl_param = &dl_rq;
            temp_piece->m_piece_writer = &write_param;
            
            //当HTTP返回值大于等于400的时候，请求失败
            curl_easy_setopt(temp_piece->m_curl, CURLOPT_FAILONERROR, 1L);
            auto ccode = curl_easy_setopt(temp_piece->m_curl, CURLOPT_WRITEFUNCTION, on_piece_download);
            ccode = curl_easy_setopt(temp_piece->m_curl, CURLOPT_WRITEDATA, (void *)temp_piece.get());
            // 下载大文件不超时，想停止，打断下载的回调就好
            ccode = curl_easy_setopt(temp_piece->m_curl, CURLOPT_TIMEOUT, 0);
            //连续5秒下载低于50bytes/s就停止下载报错
            ccode = curl_easy_setopt(temp_piece->m_curl, CURLOPT_LOW_SPEED_LIMIT, 50);
            ccode = curl_easy_setopt(temp_piece->m_curl, CURLOPT_LOW_SPEED_TIME, 10);

            //设置下载的分片
            temp_piece->m_piece_idx = i;
            auto range = write_param.GetPieceRange(i);
            if (0 == range.second)
                break;

            std::string srange = snqu::fmt::Format("{0}-{1}", range.first, range.second);
            //printf("srange:%s\r\n", srange.c_str());
            curl_easy_setopt(temp_piece->m_curl, CURLOPT_RANGE, srange.c_str());

            piece_params.push_back(temp_piece);
        }

        for (auto& item : piece_params)
        {
            thds.push_back(std::thread([&]()
            {
                item->m_err_code = curl_easy_perform(item->m_curl);
                item->m_http_code = m_impl->check_http_code(item->m_curl);

                if (item->m_http_code == CURLE_OPERATION_TIMEDOUT)
                {// 网络超时就再重试一次
                    //恢复文件指针的位置
                    item->m_piece_writer->ClearPiece(item->m_piece_idx);
                    item->m_dl_param->m_downloaded = 0;
                    item->m_err_code = curl_easy_perform(item->m_curl);
                    item->m_http_code = m_impl->check_http_code(item->m_curl);
                }

                //printf("m_piece_idx:%d ret %d %d \r\n", item->m_piece_idx, item->m_err_code, item->m_http_code);
            }));
        }

        for (auto& item : thds)
        {
            item.join();
        }

        write_param.Close();

    } while (0);

    for (auto& item : piece_params)
    {
        if (item->m_err_code && !dl_rq.m_err_code)
        {
            dl_rq.m_err_code = item->m_err_code;
            dl_rq.m_http_code = item->m_http_code;
        }
        
        m_impl->ClearCurl(item->m_curl, 0);
    }
    piece_params.clear();
}


///////////////////////////////////////////////////////////////////////////////////////////////

void CHttpClient::SetDebug(bool bDebug){ m_impl->m_bDebug_ = bDebug; }

bool CHttpClient::FtpUpload(const std::string& strUrl, 
                           int port, 
                           const std::string& user_name, 
                           const std::string& password, 
                           const std::string& file_path)
{
    CURL* curl;    
    CURLcode res;    
    //char errorBuf[CURL_ERROR_SIZE];    
    FILE *sendFile;    
    curl_slist *slist=NULL;    
    char usrpasswd[64 + 1];    
          
    //打开ftp上传的源文件   
    if(NULL == (sendFile = fopen(file_path.c_str(), "rb")))    
    {    
        return false;    
    }    
          
    //获取需要发送文件的大小   
    fseek(sendFile, 0, SEEK_END);    
    long sendSize = ftell(sendFile);    
    if(sendSize < 0)    
    {    
        fclose(sendFile);    
        return false;    
    }    
    fseek(sendFile, 0L, SEEK_SET);    
         
    if((curl = m_impl->GetCurl(false)) == NULL)
    {    
        fclose(sendFile);    
        return false;    
    }    
       
    //设置ftp上传url,组成如下的URL    
    //ftp://192.168.31.145//root/subdir/curl/testftp.txt    
    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str()); 
    sprintf(usrpasswd, "%s:%s", user_name.c_str(), password.c_str());  
    curl_easy_setopt(curl, CURLOPT_USERPWD, usrpasswd);    
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);    
    curl_easy_setopt(curl, CURLOPT_READDATA, sendFile); 
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);    
    //需sftp支持
    //curl_easy_setopt(curl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_PASSWORD);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE, sendSize);    
    curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1);   
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    /* 超时时间设置 */
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, DEFAULT_TIMEOUT);
       
    res = curl_easy_perform(curl);    
    fclose(sendFile);    
    m_impl->ClearCurl(curl, 0);
    return 0 == res;
}

struct FtpFile {
    const char *filename;
    FILE *stream;
};

static size_t my_fwrite(void *buffer, size_t size, size_t nmemb,
                        void *stream)
{
    struct FtpFile *out=(struct FtpFile *)stream;
    if(out && !out->stream) {
        /* open file for writing */ 
        fopen_s(&out->stream, out->filename, "wb");
        if(!out->stream)
            return -1; /* failure, can't open file to write */ 
    }
    return fwrite(buffer, size, nmemb, out->stream);
}
 
void CHttpClient::SftpGet()
{
    CURL *curl;
    CURLcode res;
    struct FtpFile ftpfile={
    "D:/yourfile.bin", /* name to store the file as if successful */ 
    NULL
    };
 
    curl = m_impl->GetCurl(0);
    if(curl) 
    {
        /*
        * You better replace the URL with one that works!
        */ 
        curl_easy_setopt(curl, CURLOPT_URL, "sftp://root@120.27.130.123:22/root/server.php");
        /* Define our callback to get called when there's data to be written */ 
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_fwrite);
        /* Set a pointer to our struct to pass to the callback */ 
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpfile);
        /* Switch on full protocol/debug output */ 
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
 
        res = curl_easy_perform(curl);
 
        /* always cleanup */ 
        m_impl->ClearCurl(curl, 0);

        if(CURLE_OK != res) {
            /* we failed */ 
            fprintf(stderr, "curl told us %d\n", res);
        }
    }
 
    if(ftpfile.stream)
        fclose(ftpfile.stream); /* close the local file */ 
 
}