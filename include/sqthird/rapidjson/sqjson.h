#pragma once  

#include "rpdjson.h"  
#include <string>

namespace snqu { namespace jsdetail{

template<class CodecType, bool IsUtf8>
class JsonInterface
{
public:
	typedef rapidjson::GenericValue<CodecType> SqValue;
	typedef rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> SqAlloc;

    JsonInterface(SqValue* val, SqAlloc* alloc)
        : m_val(val), m_alloc(alloc)
    {}
    ~JsonInterface() {}

    #define CHECK_SET_OBJ(val) if (!val.IsObject()) val.SetObject()
    inline void Add(const char* name) { CHECK_SET_OBJ(GetVal()); GetVal().AddMember(SqValue(name, strlen(name), GetAlloc()), SqValue(rapidjson::kNullType), GetAlloc()); }
    inline void Add(const char* name, int val) { CHECK_SET_OBJ(GetVal()); GetVal().AddMember(SqValue(name, strlen(name), GetAlloc()), val, GetAlloc()); }
    inline void Add(const char* name, unsigned int val) { CHECK_SET_OBJ(GetVal()); GetVal().AddMember(SqValue(name, strlen(name), GetAlloc()), val, GetAlloc()); }
    inline void Add(const char* name, __int64 val) { CHECK_SET_OBJ(GetVal()); GetVal().AddMember(SqValue(name, strlen(name), GetAlloc()), val, GetAlloc()); }
    inline void Add(const char* name, unsigned __int64 val) { CHECK_SET_OBJ(GetVal()); GetVal().AddMember(SqValue(name, strlen(name), GetAlloc()), val, GetAlloc()); }
    inline void Add(const char* name, float val) { CHECK_SET_OBJ(GetVal()); GetVal().AddMember(SqValue(name, strlen(name), GetAlloc()), val, GetAlloc()); }
    inline void Add(const char* name, double val) { CHECK_SET_OBJ(GetVal()); GetVal().AddMember(SqValue(name, strlen(name), GetAlloc()), val, GetAlloc()); }
    inline void Add(const char* name, const char* val, int len) 
    { 
        CHECK_SET_OBJ(GetVal()); 
        
//         if (IsUtf8)
//         {
//             if (!codec::IsUTF8(val, len))
//             {
//                 auto temp = codec::A2U(std::string(val, len));
//                 GetVal().AddMember(SqValue(name, strlen(name), GetAlloc()), SqValue(temp.c_str(), temp.length(), GetAlloc()), GetAlloc());
//                 return;
//             }
//         }
//         else
//         {
//             if (codec::IsUTF8(val, len))
//             {
//                 auto temp = codec::U2A(std::string(val, len));
//                 GetVal().AddMember(SqValue(name, strlen(name), GetAlloc()), SqValue(temp.c_str(), temp.length(), GetAlloc()), GetAlloc());
//                 return;
//             }
//         }

        GetVal().AddMember(SqValue(name, strlen(name), GetAlloc()), SqValue(val, len, GetAlloc()), GetAlloc());
    }

    inline void Add(const char* name, const std::string& val) { Add(name, val.c_str(), val.length()); }
    inline void Add(const std::string& name, int val) { Add(name.c_str(), val); }
    inline void Add(const std::string& name, unsigned int val) { Add(name.c_str(), val); }
    inline void Add(const std::string& name, __int64 val) { Add(name.c_str(), val); }
    inline void Add(const std::string& name, unsigned __int64 val) { Add(name.c_str(), val); }
    inline void Add(const std::string& name, float val) { Add(name.c_str(), val); }
    inline void Add(const std::string& name, double val) { Add(name.c_str(), val); }
    inline void Add(const std::string& name, const char* val, int len) { Add(name.c_str(), val, len); }
    inline void Add(const std::string& name, const std::string& val) { Add(name.c_str(), val); }

    inline void operator = (const char* val) 
    { 
//         if (IsUtf8)
//         {
//             size_t len = strlen(val);
//             if (!codec::IsUTF8(val, len))
//             {
//                 auto temp = codec::A2U(std::string(val, len));
//                 GetVal().SetString(temp.c_str(), temp.length(), GetAlloc());
//                 return;
//             }
//         }
//         else
//         {
//             size_t len = strlen(val);
//             if (codec::IsUTF8(val, len))
//             {
//                 auto temp = codec::U2A(std::string(val, len));
//                 GetVal().SetString(temp.c_str(), temp.length(), GetAlloc());
//                 return;
//             }
//         }
        GetVal().SetString(val, strlen(val), GetAlloc());
    }

    inline void operator = (const std::string& val) 
    { 
//         if (IsUtf8)
//         {
//             if (!codec::IsUTF8(val))
//             {
//                 auto temp = codec::A2U(val);
//                 GetVal().SetString(temp.c_str(), temp.length(), GetAlloc());
//                 return;
//             }
//         }
//         else
//         {
//             if (codec::IsUTF8(val))
//             {
//                 auto temp = codec::U2A(val);
//                 GetVal().SetString(temp.c_str(), temp.length(), GetAlloc());
//                 return;
//             }
//         }
        GetVal().SetString(val.c_str(), val.length(), GetAlloc());
    }

    inline void operator = (int val) { GetVal().SetInt(val); }
    inline void operator = (unsigned int val) { GetVal().SetUint(val); }
    inline void operator = (__int64 val) { GetVal().SetInt64(val); }
    inline void operator = (unsigned __int64 val) { GetVal().SetUint64(val); }
    inline void operator = (float val) { GetVal().SetFloat(val); }
    inline void operator = (double val) { GetVal().SetDouble(val); }

    void Del(const char* name);
    inline void Del(const std::string& name) { Del(name.c_str()); }

    inline JsonInterface operator [](size_t idx)
    {
        if (!GetVal().IsArray()) GetVal().SetArray();
        if (GetVal().Size() <= idx)
        {
            size_t cap = idx - GetVal().Size() + 1;
            for (size_t i = 0; i < cap; i++)
            {
                GetVal().PushBack(SqValue(rapidjson::kNullType), GetAlloc());
            }
        }
        return JsonInterface(&(GetVal()[idx]), &GetAlloc());
    }

    inline JsonInterface operator [](const std::string& name)
    {
        if (!GetVal().IsObject()) GetVal().SetObject();
        if (!GetVal().HasMember(name.c_str())) Add(name.c_str());
        return JsonInterface(&(GetVal()[name.c_str()]), &GetAlloc());
    }

    std::string Dump() const
    {
        using namespace rapidjson;
        GenericStringBuffer<CodecType> buffer;
        Writer<GenericStringBuffer<CodecType>> writer(buffer);
		try
		{
			if (m_val->Accept(writer))
				return buffer.GetString();
		}
		catch (...)
		{
		}
		return "";
    }

    std::string DebugDump() const
    {
        using namespace rapidjson;
        GenericStringBuffer<CodecType> buffer;
        PrettyWriter<GenericStringBuffer<CodecType>> writer(buffer);
		try
		{
			if (m_val->Accept(writer))
				return buffer.GetString();
		}
		catch (...)
		{
		}
		return "";
    }

    void Clear()
    {
        if (GetVal().IsArray())
            GetVal().Clear();
        else if (GetVal().IsObject())
            GetVal().SetObject();
        else
            GetVal().SetNull();
    }

	inline virtual SqValue& GetVal() { return *(m_val); };


    inline bool IsNull() { return GetVal().IsNull(); }
	inline bool GetBool() { return GetVal().IsBool() ? GetVal().GetBool() : false; }
    inline int GetInt() { return GetVal().IsInt() ? GetVal().GetInt() : 0; };
    inline unsigned int GetUint() { return GetVal().IsUint() ? GetVal().GetUint() : 0; };
    inline __int64 GetInt64() { return GetVal().IsInt64() ? GetVal().GetInt64() : 0; };
    inline unsigned __int64 GetUint64() { return GetVal().IsUint64() ? GetVal().GetUint64() : 0; };
    inline float GetFloat() { return GetVal().IsFloat() ? GetVal().GetFloat() : 0; };
    inline double GetDouble() { return GetVal().IsDouble() ? GetVal().GetDouble() : 0; };
    inline std::string GetString() { return GetVal().IsString() ? GetVal().GetString() : ""; };

protected:
    inline virtual SqAlloc& GetAlloc() { return *(m_alloc); };

    SqValue* m_val;
    SqAlloc* m_alloc;
};


template<class CodecType, bool IsUtf8>
class BaseJsVal : public JsonInterface<CodecType, IsUtf8>
{
public:
	typedef rapidjson::GenericValue<CodecType> SqValue;
    typedef rapidjson::GenericDocument<CodecType> SqDoc;

    BaseJsVal()
        : JsonInterface<CodecType, IsUtf8>(nullptr, nullptr)
    {
        __super::m_val = &m_root;
		__super::m_alloc = &m_root.GetAllocator();
    }

    inline bool Parse(const char* data)
    {
        m_root.Parse<0>(data);
		auto ret = m_root.GetParseError();
        return !m_root.HasParseError();
    }
    inline bool Parse(const std::string& data) { return Parse(data.c_str()); }

private:
    SqDoc m_root;
};

} //namespace jsdetail


typedef jsdetail::BaseJsVal<rapidjson::ASCII<>, false> JsVal;
typedef jsdetail::BaseJsVal<rapidjson::UTF8<>, true> JsValU; // UTF8
}

