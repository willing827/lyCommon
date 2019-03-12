#ifndef JSON_JSON_H_INCLUDED
# define JSON_JSON_H_INCLUDED

# include "autolink.h"
# include "value.h"
# include "reader.h"
# include "writer.h"
# include "features.h"

inline bool JsonParse(const std::string& js_data, Json::Value& ret)
{
    Json::Reader reader;
    if (!reader.parse(js_data, ret) || ret.isNull())
    {
        return false;
    }
    return true;
}

inline std::string JsonFastWrite(const Json::Value& val)
{
    Json::FastWriter writer;
    return writer.write(val);
}

// 基础变量的校验  
#define JsonChkBool(value, strKey) (value.isMember(strKey) && value[strKey].isBool())  
#define JsonChkString(value, strKey) (value.isMember(strKey) && value[strKey].isString())  
#define JsonChkInt(value, strKey) (value.isMember(strKey) && value[strKey].isInt())  
#define JsonChkUInt(value, strKey) (value.isMember(strKey) && (value[strKey].isUInt()|| value[strKey].isInt())) 
#define JsonChkDouble(value, strKey) (value.isMember(strKey) && value[strKey].isDouble())  

#define JsonChkNumber(value, strKey) (value.isMember(strKey) && value[strKey].isNumeric())  
#define JsonChkArray(value, strKey) (value.isMember(strKey) && value[strKey].isArray())  
#define JsonChkObj(value, strKey) (value.isMember(strKey) && value[strKey].isObject()) 

// 得到对应类型的数据，如果数据不存在则得到一个默认值  
#define JsonGetBool(value, strKey) (JsonChkBool(value, strKey)      ? value[strKey].asBool() : false)  
#define JsonGetString(value, strKey) (JsonChkString(value, strKey)  ? value[strKey].asString() : "")  
#define JsonGetInt(value, strKey) (JsonChkInt(value, strKey)    ? value[strKey].asInt() : 0)  
#define JsonGetUInt(value, strKey) (JsonChkUInt(value, strKey)    ? value[strKey].asUInt() : 0)  
#define JsonGetDouble(value, strKey) (JsonChkDouble(value, strKey)  ? ((value)[strKey]).asDouble() : 0)  

#endif // JSON_JSON_H_INCLUDED
