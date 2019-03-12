#include <sqwin/win/sqreg.h>

namespace snqu { namespace reg{

    int OpenKey(HKEY root_key, const std::string& path, HKEY* ret_key, bool is64)
    {
        DWORD   dwFlags = KEY_ALL_ACCESS;
        if (is64)
        {
            dwFlags |= KEY_WOW64_64KEY;
        }

        return RegOpenKeyExA((HKEY)root_key, path.c_str(), 0, dwFlags, ret_key);
    }

    int CreateKey(HKEY root_key, const std::string& path, HKEY* ret_key, bool is64)
    {
        HKEY	key = 0;
        DWORD	op = REG_CREATED_NEW_KEY;
        DWORD   dwFlags = KEY_ALL_ACCESS;
        if (is64)
        {
            dwFlags |= KEY_WOW64_64KEY;
        }

        long	result = RegCreateKeyExA((HKEY)root_key, path.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, dwFlags, NULL, &key, &op);

        if (ret_key)	*ret_key = key;
        else if (key)	CloseKey(key);

        return result;
    }

    int CloseKey(HKEY key)
    {
        long	ret = ERROR_SUCCESS;

        if (key) ret = RegCloseKey((HKEY)key);

        return ret;
    }

    int RemoveVal(HKEY root_key, const std::string& path, const std::string& attr_name)
    {
        HKEY	key = NULL;
        long	result = ERROR_SUCCESS;
        DWORD	type = 0, need_size = 0;

        if (attr_name.empty())
        {
            result = RegOpenKeyExA((HKEY)root_key, path.c_str(), 0, KEY_ALL_ACCESS, &key);
            if (key)
            {
                result = RegDeleteValueA(key, attr_name.c_str());
                CloseKey(key);
            }
        }
        else
        {
            result = RemoveKey(root_key, path);
        }

        return result;
    }

    int RemoveKey(HKEY root_key, const std::string& path)
    {
        typedef LSTATUS(APIENTRY*	_RegDeleteKeyExA)(
            __in HKEY hKey,
            __in LPCSTR lpSubKey,
            __in REGSAM samDesired,
            __reserved DWORD Reserved
            );

        long				result = 0;
        _RegDeleteKeyExA	del = (_RegDeleteKeyExA)GetProcAddress(GetModuleHandleA("advapi.dll"), "RegDeleteKeyExA");
        if (del)	result = del((HKEY)root_key, path.c_str(), KEY_WOW64_64KEY, 0);
        else		result = RegDeleteKeyA((HKEY)root_key, path.c_str());

        return result;
    }

    int GetStr(HKEY root_key, const std::string& path, const std::string& attr_name, std::string& value, bool is64)
    {
        HKEY	key = NULL;
        long	result = OpenKey(root_key, path, (HKEY*)&key, is64);

        if (key)
        {
            DWORD	type = 0, dwCount = 0;
            //先查询键值的长度
            result = RegQueryValueExA(key, attr_name.c_str(), NULL, &type, NULL, &dwCount);
            if (result == ERROR_SUCCESS)
            {
                value.resize(dwCount);
                //查询键值
                result = RegQueryValueExA(key, attr_name.c_str(), NULL, &type, (LPBYTE)value.data(), &dwCount);
                if (type != REG_SZ && type != REG_MULTI_SZ && type != REG_EXPAND_SZ)
                    result = ERROR_INVALID_DATA;
                else if (dwCount > value.size())
                    result = ERROR_INSUFFICIENT_BUFFER;
            }
            CloseKey(key);
        }

        return result;
    }

    int GetInt(HKEY root_key, const std::string& path, const std::string& attr_name, uint32& value, bool is64)
    {
        HKEY	key = NULL;
        long	result = OpenKey(root_key, path, (HKEY*)&key, is64);
        DWORD	type = 0, need_size = 4;

        if (key)
        {
            result = RegQueryValueExA(key, attr_name.c_str(), NULL, &type, (LPBYTE)&value, &need_size);

            if (type != REG_DWORD && type != REG_DWORD_LITTLE_ENDIAN && type != REG_DWORD_BIG_ENDIAN)
            {
                result = ERROR_INVALID_DATA;
            }
            else if (type == REG_DWORD_BIG_ENDIAN)
            {
                //result = ERROR_INVALID_DATA;
            }

            CloseKey(key);
        }
        return result;
    }

    int GetBinary(HKEY root_key, const std::string& path, const std::string& attr_name, std::string& data, bool is64)
    {
        HKEY	key = NULL;
        long	result = OpenKey(root_key, path, (HKEY*)&key, is64);
        DWORD	type = 0, dwCount = 0;

        if (key)
        {
            result = RegQueryValueExA(key, attr_name.c_str(), NULL, &type, NULL, &dwCount);
            if (result == ERROR_SUCCESS)
            {
                data.resize(dwCount);
                result = RegQueryValueExA(key, attr_name.c_str(), NULL, &type, NULL, &dwCount);
                //查询键值
                if (type != REG_BINARY)
                    result = ERROR_INVALID_DATA;
                else if (data.size() < dwCount)
                    result = ERROR_INSUFFICIENT_BUFFER;
            }

            CloseKey(key);
        }
        return result;
    }

    int SetStr(HKEY root_key, const std::string& path, const std::string& attr_name, const std::string& value, reg_sz_type type, bool is64)
    {
        HKEY	key = NULL;
        long	result = CreateKey(root_key, path, (HKEY*)&key, is64);
        if (key)
        {
            result = RegSetValueExA(key, attr_name.c_str(), 0, type, (const BYTE*)value.c_str(), value.length());

            CloseKey(key);
        }
        return result;
    }

    int SetInt(HKEY root_key, const std::string& path, const std::string& attr_name, uint32 value, bool is64)
    {
        HKEY	key = NULL;
        long	result = CreateKey(root_key, path, (HKEY*)&key, is64);

        if (key)
        {
            result = RegSetValueExA(key, attr_name.c_str(), 0, REG_DWORD, (const BYTE*)&value, sizeof(value));

            CloseKey(key);
        }
        return result;
    }

    int SetBinary(HKEY root_key, const std::string& path, const std::string& attr_name, const std::string& data, bool is64)
    {
        HKEY	key = NULL;
        long	result = CreateKey(root_key, path, (HKEY*)&key, is64);
        if (key)
        {
            result = RegSetValueExA(key, attr_name.c_str(), 0, REG_BINARY, (const BYTE*)data.c_str(), data.length());

            CloseKey(key);
        }
        return result;
    }

    void ForeachSubKey(HKEY root_key, const std::string& path, enum_reg_key_callback cb, void* user_data, bool is64)
    {
        HKEY	key = NULL;

        if (OpenKey(root_key, path.c_str(), (HKEY*)&key, is64) == ERROR_SUCCESS && key)
        {
            char	buf[256] = { 0 };
            DWORD	size = _countof(buf) - 1, len = size, ind = 0;

            while (RegEnumKeyExA(key, ind++, buf, &len, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
            {
                if (!cb(root_key, path, buf, user_data))	break;
                len = size;
            }
            RegCloseKey(key);
        }
    }

	std::string GetInstallPath(const std::string& app_name, const std::string& key_name)
	{
		std::string base_key = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\";
		std::string path;
		std::string temp_key_val = base_key + app_name;
		if (GetStr(HKEY_LOCAL_MACHINE, temp_key_val, key_name, path, false))
		{
			int err = GetLastError();
			return "";
		}
		return path;
	}
}}