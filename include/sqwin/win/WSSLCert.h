#pragma once
#include <string>

namespace snqu{ namespace cert{

/**********************************

cert_pwd:证书密码
common_name:证书使用者名称，windows显示列为颁发给 用此对应指定证书

****************************************************************/
bool ImportPFXData(const std::string& cert_data, const std::wstring& cert_pwd, const std::wstring& common_name);
bool ImportCertFile(const std::wstring& cert_path, const std::wstring& cert_pwd);
bool DeleteCert(const std::wstring& common_name);

}}