#pragma once
#include <string>
#include <memory>

namespace snqu { namespace net{

struct ftp_conn_param_t
{
    std::string m_server_ip;
    int m_server_port;
    std::string m_user_name;
    std::string m_user_pwd;
};

class FtpClient
{
public:
    FtpClient();
    ~FtpClient();

    // ͬ������ftp������
    bool connect(const ftp_conn_param_t &param, int out_time_sec = 5, 
        std::string remote_path = "LogReport\\");     

    // ͬ���ϱ��ļ�
    bool report_file(const std::string &local_path);       
    void close();

private:
    struct impl;
    std::unique_ptr<impl> impl_;
};

}}

