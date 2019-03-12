#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <string>
#include <set>
#include <sqstd/sqconvert.h>

namespace snqu{ namespace str{

inline static std::string& trim(std::string& text, const std::string& trim_chs = " \n\r\t")
{
    if (!text.empty())
    {
        text.erase(0, text.find_first_not_of(trim_chs));
        text.erase(text.find_last_not_of(trim_chs) + 1);
    }
    return text;
}

std::vector<std::string> split(const std::string& s, const std::string& delim);
std::set<std::string> split_to_set(const std::string& s, const std::string& delim);

std::string rally(const std::vector<std::string>& s, const std::string& delim);
std::string rally(const std::set<std::string>& s, const std::string& delim);


char* split_strings(char *lpszSrc, char *lpszBTag, char *lpszETag, char *lpszValue, int nValueSize);

// 分割字符串到指定的数组中
void split_strings2list(const std::string& src, const char* tag, std::vector<int>& alist);
void split_strings2list(const std::string& src, const char* tag, std::vector<std::string>& alist);

// 不区分大小写的字条串比较 true 相等 false  不相等
inline bool nequal(const std::string& str1, const std::string& str2)
{
	if (str1.size() != str2.size())
		return false;
    return 0 == _strnicmp(str1.c_str(), str2.c_str(), str2.length());
}

inline std::string& to_upper(std::string& param)
{
    std::transform((param).begin(), (param).end(), (param).begin(), toupper);
    return param;
}

inline std::string to_upper(const std::string& param)
{
    std::string temp(param);
    std::transform((temp).begin(), (temp).end(), (temp).begin(), toupper);
    return temp;
}

inline std::string& to_lower(std::string& param)
{
	std::transform((param).begin(), (param).end(), (param).begin(), tolower);
	return param;
}

inline std::string to_lower(const std::string& param)
{
	std::string temp(param);
	std::transform((temp).begin(), (temp).end(), (temp).begin(), tolower);
	return temp;
}

inline void str_replace(std::string& input, char replaced, char to)
{
	for (int i = 0; i < (int)input.size(); i++)
	{
		if (input[i] == replaced)
			input[i] = to;
	}
}

inline void string_replace(std::string &strBase, std::string strSrc, std::string strDes)  
{  
    std::string::size_type pos = 0;  
    std::string::size_type srcLen = strSrc.size();  
    std::string::size_type desLen = strDes.size();  
    pos=strBase.find(strSrc, pos);   
    while ((pos != std::string::npos))  
    {  
        strBase.replace(pos, srcLen, strDes);  
        pos=strBase.find(strSrc, (pos+desLen));  
    }  
} 

inline void split_path_file(const std::string& input, std::string& dir, std::string& file_name)
{
	if (input.empty()) return;
	auto cinput = input;
	str_replace(cinput, '/', '\\');
	auto pos = cinput.rfind("\\") + 1;
	if (pos != std::string::npos)
	{
		file_name = cinput.substr(pos, cinput.size() - pos);
		dir = cinput.substr(0, pos);
	}
}

inline std::string path_remove_file(const std::string& input)
{
	if (input.empty()) return "";
	std::string path;
	auto cinput = input;
	str_replace(cinput, '/', '\\');
	auto pos = cinput.rfind("\\") + 1;
	if (pos != std::string::npos)
	{
		path = cinput.substr(0, pos);
	}
	return path;
}

/* 保留2位的小数转换,传入值为100倍的整数 */
std::string cent_to_string(int value);

/* 保留2位的小数转换 */
std::string to_string(double value, int decplaces=3);
#define NORMAL_TIME_STR "%y%m/%d %H:%M:%S"
std::string to_string(time_t, std::string format = "%m/%d-%H:%M:%S");

bool is_id(const std::string&);

// 根据身份证号取生日
std::pair<bool/*is_male*/, std::string> get_birthday_from_id(const std::string&);
bool check_birthday(const std::string& birth, time_t ntime);

// 截取右侧
std::string get_right(const std::string&, size_t len);

// 版本号比较
// @return 0 相等 1 ver1大于ver2 -1 ver1小于ver2
#define SQ_VER_EQUAL  0
#define SQ_VER_BEYOND 1
#define SQ_VER_LESS  -1
int version_compare(const std::string& ver1, const std::string& ver2);

}}