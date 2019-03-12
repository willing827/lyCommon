#pragma once
#include <string>

//need cryptlib_mt.lib

namespace snqu{ namespace safe{

std::string crypto_AES128_CBC_encrypt(const std::string& plaint, const std::string& sKey, const std::string& sIV);

std::string crypto_AES128_CBC_decrypt(const std::string& plaint, const std::string& sKey, const std::string& sIV);

}}