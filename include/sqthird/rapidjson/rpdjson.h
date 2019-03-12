#pragma once  

#include "detail/rapidjson.h"  
#include "detail/document.h"  
#include "detail/writer.h"  
#include "detail/prettywriter.h"  
#include "detail/stringbuffer.h" 

namespace rapidjson {

// 基础变量的校验  
#define RpjsChkBool(value, strKey) (value.HasMember(strKey) && value[strKey].IsBool())  
#define RpjsChkString(value, strKey) (value.HasMember(strKey) && value[strKey].IsString())  
#define RpjsChkInt32(value, strKey) (value.HasMember(strKey) && value[strKey].IsInt())  
#define RpjsChkUint32(value, strKey) (value.HasMember(strKey) && value[strKey].IsUint())  
#define RpjsChkInt64(value, strKey) (value.HasMember(strKey) && value[strKey].IsInt64())  
#define RpjsChkUint64(value, strKey) (value.HasMember(strKey) && value[strKey].IsUint64())  
#define RpjsChkFloat(value, strKey) (value.HasMember(strKey) && value[strKey].IsFloat())  
#define RpjsChkDouble(value, strKey) (value.HasMember(strKey) && value[strKey].IsDouble())  

#define RpjsChkNumber(value, strKey) (value.HasMember(strKey) && value[strKey].IsNumber())  
#define RpjsChkArray(value, strKey) (value.HasMember(strKey) && value[strKey].IsArray())  
#define RpjsChkObj(value, strKey) (value.HasMember(strKey) && value[strKey].IsObject()) 

// 得到对应类型的数据，如果数据不存在则得到一个默认值  
#define RpjsGetBool(value, strKey) (RpjsChkBool(value, strKey)      ? value[strKey].GetBool() : false)  
#define RpjsGetString(value, strKey) (RpjsChkString(value, strKey)  ? value[strKey].GetString() : "")  
#define RpjsGetInt32(value, strKey) (RpjsChkInt32(value, strKey)    ? value[strKey].GetInt() : 0)  
#define RpjsGetUint32(value, strKey) (RpjsChkUint32(value, strKey)  ? value[strKey].GetUint() : 0)  
#define RpjsGetInt64(value, strKey) (RpjsChkInt64(value, strKey)    ? ((value)[strKey]).GetInt64() : 0)  
#define RpjsGetUint64(value, strKey) (RpjsChkUint64(value, strKey)  ? ((value)[strKey]).GetUint64() : 0)  
#define RpjsGetFloat(value, strKey) (RpjsChkFloat(value, strKey)    ? ((value)[strKey]).GetFloat() : 0)  
#define RpjsGetDouble(value, strKey) (RpjsChkDouble(value, strKey)  ? ((value)[strKey]).GetDouble() : 0)  

// 得到Value指针  
#define RpjsGetValuePtr(value, strKey) (((value).HasMember(strKey)) ? &((value)[strKey]) : nullptr) 

    inline std::string DumpStr(const Value& root)
    {
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        root.Accept(writer);
        return buffer.GetString();
    }

    inline std::string DumpPrettyStr(const Value& doc)
    {
        StringBuffer buffer;
        PrettyWriter<StringBuffer> writer(buffer);
        doc.Accept(writer);
        return buffer.GetString();
    }
	
}

