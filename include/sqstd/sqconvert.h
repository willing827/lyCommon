#ifndef _SQ_TYPE_CONVERT_H
#define _SQ_TYPE_CONVERT_H
#include <string>
#include <locale>
#include <limits>
/*
    这些函数可能抛出异常
*/

namespace snqu { namespace str{

    bool IsNum(const char* param);

    inline bool IsNum(const std::string& param)
    {
        return IsNum(param.c_str());
    }

    inline __int64 ToInt64(const char* str_val)  
    { 
        try
        {
            return std::stoll(str_val);
        }
        catch (std::exception& e)
        {
            auto err = e.what();
        }
        return 0;
    }

    inline unsigned __int64   ToUInt64(const char* str_val) 
    { 
        try
        {
            return std::stoull(str_val);
        }
        catch (std::exception& e)
        {
            auto err = e.what();
        }
        return 0;
    }

    inline __int32    ToInt32(const char* str_val)  
    { 
        try
        {
            return std::stoi(str_val);
        }
        catch (std::exception& e)
        {
            auto err = e.what();
        }
        return 0;
    }
    inline unsigned __int32   ToUInt32(const char* str_val) 
    { 
        try
        {
            return std::stoul(str_val);
        }
        catch (std::exception& e)
        {
            auto err = e.what();
        }
        return 0;
    }

    inline double   ToDouble(const char* str_val) 
    { 
        try
        {
            return std::stod(str_val);
        }
        catch (std::exception& e)
        {
            auto err = e.what();
        }
        return 0;
    }

    inline float    ToFloat(const char* str_val)  
    { 
        try
        {
            return std::stof(str_val);
        }
        catch (std::exception& e)
        {
            auto err = e.what();
        }
        return 0;
    }

    short ToShort(const char* str_val);
    unsigned short ToUShort(const char* str_val);

    inline __int64    ToInt64(const std::string& str_val)  { return ToInt64(str_val.c_str()); }
    inline unsigned __int64   ToUInt64(const std::string& str_val) { return ToUInt64(str_val.c_str()); }
    inline __int32    ToInt32(const std::string& str_val)  { return ToInt32(str_val.c_str()); }
    inline unsigned __int32   ToUInt32(const std::string& str_val) { return ToUInt32(str_val.c_str()); }
    inline double   ToDouble(const std::string& str_val) { return ToDouble(str_val.c_str()); }
    inline float    ToFloat(const std::string& str_val)  { return ToFloat(str_val.c_str()); }
    inline short   ToShort(const std::string& str_val) { return ToShort(str_val.c_str()); }
    inline unsigned short ToUShort(const std::string& str_val) { return ToUShort(str_val.c_str()); }

    template <class T> inline T ToNumber(const std::string& input) { return T; }
    template <> inline uint64_t ToNumber(const std::string& input) { return str::ToUInt64(input); }
    template <> inline int64_t ToNumber(const std::string& input) { return str::ToInt64(input); }
    template <> inline uint32_t ToNumber(const std::string& input) { return str::ToUInt32(input); }
    template <> inline int32_t ToNumber(const std::string& input) { return str::ToInt32(input); }
    template <> inline float ToNumber<float>(const std::string& input) { return str::ToFloat(input); }
    template <> inline double ToNumber<double>(const std::string& input) { return str::ToDouble(input); }
    template <> inline bool ToNumber(const std::string& input) { return 0 != str::ToInt32(input); }
    template <> inline short ToNumber<short>(const std::string& input) { return str::ToShort(input); }
    template <> inline unsigned short ToNumber<unsigned short>(const std::string& input) { return 0 != str::ToUShort(input); }
    
}}
#endif//_SQ_TYPE_CONVERT_H