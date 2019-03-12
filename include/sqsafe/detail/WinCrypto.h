#pragma once
/*
	Encrypting Data with windows CNG api
*/
#include <string>

namespace snqu{

	bool WinEncrypt(const std::string& input_data, const std::string& key, const std::string& iv, std::string& out_data);
	bool WinDecrypt(const std::string& input_data, const std::string& key, const std::string& iv, std::string& out_data);

}