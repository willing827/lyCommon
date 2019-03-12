#pragma once
#include <sqwin/sqwin.h>
#include <sqwin/win/sqos.h>

//注册表读写
// all functions return  ZERO indicates success, other is error_code

namespace snqu { namespace reg{

    enum reg_sz_type
    {
        REGSTR_SINGLE = 1,	// REG_SZ,
        REGSTR_EXPAND = 2,	// REG_EXPAND_SZ,
        REGSTR_MULTI  = 7,	// REG_MULTI_SZ,
    };

    int CreateKey(HKEY root_key, const std::string& path, HKEY* ret_key, bool is64 = os::IsWow64());
    int CloseKey(HKEY key);

    /*Will delete the key given by path if this value is NULL*/
    int RemoveVal(HKEY root_key, const std::string& path, const std::string& attr_name);
    int RemoveKey(HKEY root_key, const std::string& path);

    int GetStr(HKEY root_key, const std::string& path, const std::string& attr_name, std::string& value, bool is64 = os::IsWow64());
    int GetInt(HKEY root_key, const std::string& path, const std::string& attr_name, uint32& value, bool is64 = os::IsWow64());
    int GetBinary(HKEY root_key, const std::string& path, const std::string& attr_name, std::string& data, bool is64 = os::IsWow64());

    // create if path or attr not existing
    int SetStr(HKEY root_key, const std::string& path, const std::string& attr_name, const std::string& value, reg_sz_type type = REGSTR_SINGLE, bool is64 = os::IsWow64());
    int SetInt(HKEY root_key, const std::string& path, const std::string& attr_name, uint32 value, bool is64 = os::IsWow64());
    int SetBinary(HKEY root_key, const std::string& path, const std::string& attr_name, const std::string& data, bool is64 = os::IsWow64());
    
    // 返回false停止遍历
    typedef bool(__stdcall* enum_reg_key_callback)(HKEY root_key, const std::string& path, const std::string& found_child_key, void* user_data);
    void ForeachSubKey(HKEY root_key, const std::string& path, enum_reg_key_callback cb, void* user_data, bool is64 = os::IsWow64());

	//获取软件的安装目录
	std::string GetInstallPath(const std::string& app_name, const std::string& key_name);
}}
