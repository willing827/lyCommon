#pragma once
#include <string>

namespace snqu { namespace codec {

std::string convert_utf8_to_ansi(const std::string& utf8);

std::string convert_ansi_to_utf8(const std::string& ansi);

std::wstring convert_ansi_to_unicode(const std::string& ansi);

std::string convert_unicode_to_ansi(const std::wstring& unicode);

std::string convert_unicode_to_utf8(const std::wstring& unicode);

std::wstring convert_utf8_to_unicode(const std::string& utf8);

bool is_UTF8(const char* str, int size);
bool is_unicode(const char* bom_str);
bool is_utf8(const char* bom_str);  //webutf8
bool is_ansi(const char* bom_str);

std::string bom_str_to_ansi(const char* bom_str, size_t len);
std::string bom_str_to_utf8(const char* bom_str, size_t len);
std::wstring bom_str_to_unicode(const char* bom_str, size_t len);
std::string bom_str_from_ansi(const char* ansi_str);
std::string bom_str_from_utf8(const char* utf8_str);
std::string bom_str_from_unicode(const wchar_t* unic_str);

}
}