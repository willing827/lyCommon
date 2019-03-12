#include <sqwin/win/sqwinftp.h>
#include <sqwin/win/sqwindows.h>
#include <sqwin/sqwin.h>
#include <sqstd/sqfilerw.h>
#include <WinInet.h>


#pragma comment (lib, "wininet.lib")

namespace snqu {namespace net{

struct FtpClient::impl
{
    HINTERNET hInternet_;
    HINTERNET hConnect_;    // internet 连接

    
    impl()
        : hConnect_(NULL)
        , hInternet_(NULL)
    {}

    bool is_conn()
    {
        bool ret = (NULL != hInternet_) && (NULL != hConnect_);
        return ret;
    }

    bool set_ftp_dir(const std::string &ftp_dir)
    {
        // 设置FTP的上传目录
        if (FALSE == FtpSetCurrentDirectoryA(hConnect_, ftp_dir.c_str()))
        {
            if (FALSE == FtpCreateDirectoryA(hConnect_, ftp_dir.c_str()))
            {
                return false;
            }

            if (FALSE == FtpSetCurrentDirectoryA(hConnect_, ftp_dir.c_str()))
            {
                return false;
            }
        }

        return true;
    }

    ~impl(){};
};

FtpClient::FtpClient()
    : impl_(new impl)
{}

FtpClient::~FtpClient()
{
    close();
}

bool FtpClient::connect(const ftp_conn_param_t &param, int out_time_sec, std::string remote_path)
{
    impl_->hInternet_ = InternetOpenA("LogReport", INTERNET_OPEN_TYPE_DIRECT, 
        NULL, NULL, INTERNET_FLAG_NO_CACHE_WRITE);
    if ( NULL == impl_->hInternet_ )
    { 
        return false;
    }
    int err_code = 0;
    DWORD out_time = out_time_sec * 1000;

    if(FALSE == InternetSetOptionA(impl_->hInternet_, INTERNET_OPTION_CONNECT_TIMEOUT, &out_time, 4))
    {
        err_code = GetLastError();
        return false;
    }

    impl_->hConnect_ = InternetConnectA(impl_->hInternet_, param.m_server_ip.c_str(), param.m_server_port, 
        param.m_user_name.c_str(), param.m_user_pwd.c_str(), INTERNET_SERVICE_FTP, 
        INTERNET_FLAG_EXISTING_CONNECT || INTERNET_FLAG_PASSIVE,0 );
    if ( NULL == impl_->hConnect_ )
    {
        InternetCloseHandle(impl_->hInternet_);
        return false;
    }

    if (!impl_->set_ftp_dir(remote_path))
    {
        return false;
    }

    return true;
}

bool FtpClient::report_file(const std::string &local_path)
{
    if (!impl_->is_conn())
    {
        return false;
    }

    // 判断本地文件存在
	if (!snqu::FileRW::Exist(local_path))
    {
        return false;
    }

    int pRes = FtpPutFileA(impl_->hConnect_, local_path.c_str(), 
        local_path.c_str(), FTP_TRANSFER_TYPE_ASCII,0);

    if(pRes==0)
    {
        return false;
    }

    return true;
}

void FtpClient::close()
{
    if (NULL != impl_->hConnect_)
    {
        InternetCloseHandle(impl_->hConnect_);
        impl_->hConnect_ = NULL;
    }
    if (NULL != impl_->hInternet_)
    {
        InternetCloseHandle(impl_->hInternet_);
        impl_->hInternet_ = NULL;
    }
}

}}