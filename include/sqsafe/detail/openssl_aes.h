#pragma once
#include <string>

//need libcrypto.lib crypt32.lib

namespace snqu{ namespace safe{

std::string openssl_cbc_128_encrypt(const std::string& in_data, const std::string& key, const std::string& iv);

std::string openssl_cbc_128_decrypt(const std::string& in_data, const std::string& key, const std::string& iv);

}}