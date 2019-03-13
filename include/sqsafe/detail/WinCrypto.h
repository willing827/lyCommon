#pragma once
/*
	Encrypting Data with windows CNG api
	os version win7º∞win7“‘…œ
*/
#include <string>

namespace snqu{
	
	std::string WinMD5(const std::string& input);
	std::string WinSHA1(const std::string& input);
	std::string WinSHA256(const std::string& input);
	std::string WinBase64(const std::string& input);
	
	/*cbc mode*/
	bool WinAESEncrypt(const std::string& input_data, const std::string& key, const std::string& iv, std::string& out_data);
	bool WinAESDecrypt(const std::string& input_data, const std::string& key, const std::string& iv, std::string& out_data);

	/*cbc mode*/
	bool WinDESEncrypt(const std::string& input_data, const std::string& key, const std::string& iv, std::string& out_data);
	bool WinDESDecrypt(const std::string& input_data, const std::string& key, const std::string& iv, std::string& out_data);

	bool WinRC4Encrypt(const std::string& input_data, const std::string& key, std::string& out_data);
	bool WinRC4Decrypt(const std::string& input_data, const std::string& key, std::string& out_data);

}