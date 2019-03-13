#pragma once
#include <string>

namespace snqu{ namespace cert{

/**********************************
从内存中将ssl证书导入系统
@param1 cert_data	PFX Blob data
@param2 cert_pwd	证书密码
@param3 common_name 证书使用者名称，windows显示列为颁发给 用此对应指定证书
****************************************************************/
bool ImportPFXData(const std::string& cert_data, const std::wstring& cert_pwd, const std::wstring& common_name);
/**********************************
从磁盘中中将ssl证书文件导入系统
@param1 cert_path	证书目录
@param2 cert_pwd	证书密码
****************************************************************/
bool ImportCertFile(const std::wstring& cert_path, const std::wstring& cert_pwd);
/**********************************
从系统中删除证书
@param1 common_name 证书使用者名称，windows显示列为颁发给 用此对应指定证书
****************************************************************/
bool DeleteCert(const std::wstring& common_name);

}}