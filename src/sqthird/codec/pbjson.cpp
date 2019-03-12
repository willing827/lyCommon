#include <codec/pbjson.h>
#include <codec/sqcodec.h>
#include <sqstd/sqformat.h>
#include <sqstd/sqstringhelper.h>
#include <sqstd/sqconvert.h>
#include <sqlog/sqlog.h>


#define RETURN_ERR(id, cause)  do{\
                                  if (nullptr != err) *err = cause; \
                                  return id;   \
                                 }while(0)

#define MASK_MESSAGE_NAME "google.protobuf.FieldMask"
using namespace google::protobuf;
using namespace rapidjson;
namespace pbjson
{
    void trans_to_json_object(const Message& msg, Value& root, Value::AllocatorType& allocator, const Message *mask, bool isb64, bool isall);
    bool field2json(const Message& msg, const FieldDescriptor *field, Value& file_value, Value::AllocatorType& allocator, const Message *mask, bool isb64, bool isall)
    {
        const Reflection *ref = msg.GetReflection();
        const bool repeated = field->is_repeated();

        size_t array_size = 0;
        if (repeated)
        {
            array_size = ref->FieldSize(msg, field);
            file_value.SetArray();
        }
        switch (field->cpp_type())
        {
            case FieldDescriptor::CPPTYPE_DOUBLE:
                if (repeated)
                {
                    for (size_t i = 0; i != array_size; ++i)
                        file_value.PushBack(rapidjson::Value(ref->GetRepeatedDouble(msg, field, i)), allocator);
                }
                else
                    file_value.SetDouble(ref->GetDouble(msg, field));
                break;
            case FieldDescriptor::CPPTYPE_FLOAT:
                if (repeated)
                {
                    for (size_t i = 0; i != array_size; ++i)
                        file_value.PushBack(Value(ref->GetRepeatedFloat(msg, field, i)), allocator);
                }
                else
                    file_value.SetFloat(ref->GetFloat(msg, field));
                break;
            case FieldDescriptor::CPPTYPE_INT64:
                if (repeated)
                {
                    for (size_t i = 0; i != array_size; ++i)
                    {
                        int64_t value = ref->GetRepeatedInt64(msg, field, i);
                        rapidjson::Value v(value);
                        file_value.PushBack(v, allocator);
                    }
                }
                else
                    file_value.SetInt64(static_cast<int64_t>(ref->GetInt64(msg, field)));
                break;
            case FieldDescriptor::CPPTYPE_UINT64:
                if (repeated)
                {
                    for (size_t i = 0; i != array_size; ++i)
                    {
                        uint64_t value = ref->GetRepeatedUInt64(msg, field, i);
                        rapidjson::Value v(value);
                        file_value.PushBack(v, allocator);
                    }
                }
                else
                    file_value.SetUint64(static_cast<uint64_t>(ref->GetUInt64(msg, field)));
                break;
            case FieldDescriptor::CPPTYPE_INT32:
                if (repeated)
                {
                    for (size_t i = 0; i != array_size; ++i)
                    {
                        int32_t value = ref->GetRepeatedInt32(msg, field, i);
                        rapidjson::Value v(value);
                        file_value.PushBack(v, allocator);
                    }
                }
                else
                    file_value.SetInt(ref->GetInt32(msg, field));
                break;
            case FieldDescriptor::CPPTYPE_UINT32:
                if (repeated)
                {
                    for (size_t i = 0; i != array_size; ++i)
                    {
                        uint32_t value = ref->GetRepeatedUInt32(msg, field, i);
                        rapidjson::Value v(value);
                        file_value.PushBack(v, allocator);
                    }
                }
                else
                    file_value.SetUint(ref->GetUInt32(msg, field));
                break;
            case FieldDescriptor::CPPTYPE_BOOL:
                if (repeated)
                {
                    for (size_t i = 0; i != array_size; ++i)
                    {
                        bool value = ref->GetRepeatedBool(msg, field, i);
                        rapidjson::Value v(value);
                        file_value.PushBack(v, allocator);
                    }
                }
                else
                    file_value.SetBool(ref->GetBool(msg, field));
                break;
            case FieldDescriptor::CPPTYPE_STRING:
            {
                bool is_binary = field->type() == FieldDescriptor::TYPE_BYTES;
                if (repeated)
                {
                    for (size_t i = 0; i != array_size; ++i)
                    {
                        std::string value = ref->GetRepeatedString(msg, field, i);
                        if (isb64 && is_binary)
                        {
                            value = snqu::codec::B64Encode(value);
                        }
                        rapidjson::Value v(value.c_str(), static_cast<rapidjson::SizeType>(value.size()), allocator);
                        file_value.PushBack(v, allocator);
                    }
                }
                else
                {
                    std::string value = ref->GetString(msg, field);
                    if (isb64 && is_binary)
                    {
                        value = snqu::codec::B64Encode(value);
                    }
                    file_value.SetString(value.c_str(), value.size(), allocator);
                }
                break;
            }
            case FieldDescriptor::CPPTYPE_MESSAGE:
                if (field->message_type()->full_name() == MASK_MESSAGE_NAME)
                {// ¸¨Öú×Ö¶Î£¬²»´¦Àí
                    return false;
                }
                if (repeated)
                {
                    for (size_t i = 0; i != array_size; ++i)
                    {
                        const Message *value = &(ref->GetRepeatedMessage(msg, field, i));
                        rapidjson::Value v;
                        trans_to_json_object(*value, v, allocator, mask, isb64, isall);
                        file_value.PushBack(v, allocator);
                    }
                }
                else
                {
                    const Message *value = &(ref->GetMessage(msg, field));
                    trans_to_json_object(*value, file_value, allocator, mask, isb64, isall);
                }
                break;
            case FieldDescriptor::CPPTYPE_ENUM:
                if (repeated)
                {
                    for (size_t i = 0; i != array_size; ++i)
                    {
                        file_value.PushBack(Value(ref->GetRepeatedEnum(msg, field, i)->number()), allocator);
                    }
                }
                else
                    file_value.SetInt(ref->GetEnum(msg, field)->number());
                break;
            default:
                break;
        }

        return true;
    }

    bool has_mask_field(const FieldDescriptor *field, const Message *mask)
    {
        if (nullptr == mask)
            return false;

        auto *p_mask = (google::protobuf::FieldMask*)(mask);
        return util::FieldMaskUtil::IsPathInFieldMask(field->name(), *p_mask);
    }

    void trans_to_json_object(const Message& msg, Value& root, rapidjson::Value::AllocatorType& allocator, const Message *mask, bool isb64, bool isall)
    {
        try
        {
            root.SetObject();
            const Descriptor *d = msg.GetDescriptor();
            if (!d) return;
            size_t count = d->field_count();

            for (size_t i = 0; i != count; ++i)
            {
                const FieldDescriptor *field = d->field(i);
                if (!field) return;

                const Reflection *ref = msg.GetReflection();
                if (!ref) return;

                rapidjson::Value field_name(field->name().c_str(), field->name().size());
                rapidjson::Value field_value;
                bool is_add_to_json = false;
                do
                {
                    if (has_mask_field(field, mask) || isall)
                    {
                        is_add_to_json = true;
                        break;
                    }

                    if (field->is_repeated())
                    {
                        if (0 != ref->FieldSize(msg, field))
                        {
                            is_add_to_json = true;
                            break;
                        }
                    }
                    else if (ref->HasField(msg, field))
                    {
                        is_add_to_json = true;
                        break;
                    }
                } while (0);

                if (is_add_to_json && field2json(msg, field, field_value, allocator, mask, isb64, isall))
                    root.AddMember(field_name, field_value, allocator);
            }
        }
        catch (std::exception* e)
        {
            SNLOG(snqu::kError, "trans_to_json_object exception [%s]", e->what());
        }
        catch (...)
        {
			SNLOG(snqu::kFatal, "trans_to_json_object exception unhandle");
        }
    }
    int parse_json(const rapidjson::Value* json, Message& msg, std::string* err, bool auto_convert);
    int json2field(const rapidjson::Value* json, Message& msg, const FieldDescriptor *field, std::string* err, bool auto_convert)
    {
        const Reflection *ref = msg.GetReflection();
        const bool repeated = field->is_repeated();
        using namespace snqu;
        switch (field->cpp_type())
        {
            case FieldDescriptor::CPPTYPE_INT32:
            {
                int32_t value = 0;
                if (json->GetType() == rapidjson::kNumberType)
                    value = json->GetInt();
                else if (auto_convert && json->GetType() == rapidjson::kStringType)
                    value = str::ToInt32(json->GetString());
                else
                    RETURN_ERR(ERR_INVALID_JSON, snqu::fmt::Format("Field [{0}] Not a number", field->name()));
                repeated ? ref->AddInt32(&msg, field, value) : ref->SetInt32(&msg, field, value);
                break;
            }
            case FieldDescriptor::CPPTYPE_UINT32:
            {
                uint32_t value = 0;
                if (json->GetType() == rapidjson::kNumberType)
                    value = json->GetUint();
                else if (auto_convert && json->GetType() == rapidjson::kStringType)
                    value = str::ToUInt32(json->GetString());
                else
                    RETURN_ERR(ERR_INVALID_JSON, snqu::fmt::Format("Field [{0}] Not a number", field->name()));
                repeated ? ref->AddUInt32(&msg, field, value) : ref->SetUInt32(&msg, field, value);
                break;
            }
            case FieldDescriptor::CPPTYPE_INT64:
            {
                int64_t value = 0;
                if (json->GetType() == rapidjson::kNumberType)
                    value = json->GetInt64();
                else if (auto_convert && json->GetType() == rapidjson::kStringType)
                    value = str::ToInt64(json->GetString());
                else
                    RETURN_ERR(ERR_INVALID_JSON, snqu::fmt::Format("Field [{0}] Not a number", field->name()));
                repeated ? ref->AddInt64(&msg, field, value) : ref->SetInt64(&msg, field, value);
                break;
            }
            case FieldDescriptor::CPPTYPE_UINT64:
            {
                uint64_t value = 0;
                if (json->GetType() == rapidjson::kNumberType)
                    value = json->GetUint64();
                else if (auto_convert && json->GetType() == rapidjson::kStringType)
                    value = str::ToUInt64(json->GetString());
                else
                    RETURN_ERR(ERR_INVALID_JSON, snqu::fmt::Format("Field [{0}] Not a number", field->name()));
                repeated ? ref->AddUInt64(&msg, field, value) : ref->SetUInt64(&msg, field, value);
                break;
            }
            case FieldDescriptor::CPPTYPE_DOUBLE:
            {
                double value = 0;
                if (json->GetType() == rapidjson::kNumberType)
                    value = json->GetDouble();
                else if (auto_convert && json->GetType() == rapidjson::kStringType)
                    value = str::ToDouble(json->GetString());
                else
                    RETURN_ERR(ERR_INVALID_JSON, snqu::fmt::Format("Field [{0}] Not a number", field->name()));
                repeated ? ref->AddDouble(&msg, field, value) : ref->SetDouble(&msg, field, value);
                break;
            }
            case FieldDescriptor::CPPTYPE_FLOAT:
            {
                float value = 0;
                if (json->GetType() == rapidjson::kNumberType)
                    value = json->GetFloat();
                else if (auto_convert && json->GetType() == rapidjson::kStringType)
                    value = str::ToFloat(json->GetString());
                else
                    RETURN_ERR(ERR_INVALID_JSON, snqu::fmt::Format("Field [{0}] Not a number", field->name()));
                repeated ? ref->AddFloat(&msg, field, value) : ref->SetFloat(&msg, field, value);
                break;
            }
            case FieldDescriptor::CPPTYPE_BOOL:
            {
                if (json->GetType() != rapidjson::kTrueType && json->GetType() != rapidjson::kFalseType)
                    RETURN_ERR(ERR_INVALID_JSON, snqu::fmt::Format("Field [{0}] Not a bool", field->name()));
                bool value = json->GetBool();
                repeated ? ref->AddBool(&msg, field, value) : ref->SetBool(&msg, field, value);
                break;
            }
            case FieldDescriptor::CPPTYPE_STRING:
            {
                if (json->GetType() != rapidjson::kStringType)
                    RETURN_ERR(ERR_INVALID_JSON, snqu::fmt::Format("Field [{0}] Not a string", field->name()));
                const char* value = json->GetString();
                uint32_t str_size = json->GetStringLength();
                std::string str_value(value, str_size);
//                 if (field->type() == FieldDescriptor::TYPE_BYTES)
//                 {
//                     if (repeated)
//                     {
//                         ref->AddString(&msg, field, snqu::codec::B64Decode(str_value));
//                     }
//                     else
//                     {
//                         ref->SetString(&msg, field, snqu::codec::B64Decode(str_value));
//                     }
//                 }
//                 else
                {
                    repeated ? ref->AddString(&msg, field, str_value) : ref->SetString(&msg, field, str_value);
                }
                break;
            }
            case FieldDescriptor::CPPTYPE_MESSAGE:
            {
                Message *mf = (repeated) ? ref->AddMessage(&msg, field) : ref->MutableMessage(&msg, field);
                return parse_json(json, *mf, err, auto_convert);
            }
            case FieldDescriptor::CPPTYPE_ENUM:
            {
                const EnumDescriptor *ed = field->enum_type();
                const EnumValueDescriptor *ev = 0;
                if (json->GetType() == rapidjson::kNumberType)
                    ev = ed->FindValueByNumber(json->GetInt());
                else if (json->GetType() == rapidjson::kStringType)
                    ev = ed->FindValueByName(json->GetString());
                else
                    RETURN_ERR(ERR_INVALID_JSON, snqu::fmt::Format("Field [{0}] Not an integer or string", field->name()));
                if (!ev)
                    RETURN_ERR(ERR_INVALID_JSON, snqu::fmt::Format("Field [{0}] Enum value not found", field->name()));
                repeated ? ref->AddEnum(&msg, field, ev) : ref->SetEnum(&msg, field, ev);
                break;
            }
            default:
                break;
        }
        return 0;
    }

    static int parse_json(const rapidjson::Value* json, Message& msg, std::string* err, bool auto_convert)
    {
        if (NULL == json)
        {
            return ERR_INVALID_ARG;
        }
        if (json->GetType() == rapidjson::kArrayType && json->Size() == 0)
            return 0;

        if (json->GetType() != rapidjson::kObjectType)
        {
            return ERR_INVALID_ARG;
        }
        const Descriptor *d = msg.GetDescriptor();
        const Reflection *ref = msg.GetReflection();
        if (!d || !ref)
        {
            RETURN_ERR(ERR_INVALID_PB, "invalid pb object");
        }
        for (auto itr = json->MemberBegin(); itr != json->MemberEnd(); ++itr)
        {
            const char* name = itr->name.GetString();
            const FieldDescriptor *field = d->FindFieldByName(name);
            if (!field)
                field = ref->FindKnownExtensionByName(name);
            if (!field)
                continue; // TODO: we should not fail here, instead write this value into an unknown field
            if (itr->value.GetType() == rapidjson::kNullType) {
                ref->ClearField(&msg, field);
                continue;
            }
            if (field->is_repeated())
            {
                if (itr->value.GetType() != rapidjson::kArrayType)
                    RETURN_ERR(ERR_INVALID_JSON, snqu::fmt::Format("Field [{0}] Not array", field->name()));
                for (auto ait = itr->value.Begin(); ait != itr->value.End(); ++ait)
                {
                    int ret = json2field(ait, msg, field, err, auto_convert);
                    if (ret != 0) return ret;
                }
            }
            else
            {
                int ret = json2field(&(itr->value), msg, field, err, auto_convert);
                if (ret != 0) return ret;
            }
        }
        return 0;
    }

    const Message *get_pbmask(const Message& msg)
    {
        const Descriptor *d = msg.GetDescriptor();
        const Reflection *ref = msg.GetReflection();
        const Message *mask = nullptr;
        size_t count = d->field_count();

        for (size_t i = 0; i != count; ++i)
        {
            const FieldDescriptor *field = d->field(i);
            if (field->name() != "mask") continue;
            if (field->cpp_type() != FieldDescriptor::CPPTYPE_MESSAGE) continue;
            if (field->message_type()->full_name() != MASK_MESSAGE_NAME) continue;

            mask = &(ref->GetMessage(msg, field));
            break;
        }

        return mask;
    }

    std::string pb2json(const Message& msg, bool isb64, bool isall)
    {
        rapidjson::Value::AllocatorType allocator;
        rapidjson::Value root;
        trans_to_json_object(msg, root, allocator, get_pbmask(msg), isb64, isall);
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        root.Accept(writer);
        return buffer.GetString();
    }

    void pb2jsonobject(const google::protobuf::Message& msg, rapidjson::Value& val, rapidjson::Value::AllocatorType& allocator, bool isb64, bool isall)
    {
        trans_to_json_object(msg, val, allocator, get_pbmask(msg), isb64, isall);
    }

    int json2pb(const std::string& json, google::protobuf::Message& msg, std::string* err, bool auto_convert)
    {
        rapidjson::Document d;
        d.Parse<0>(json.c_str());
        if (d.HasParseError())
        {
            if (nullptr != err)
                *err += d.GetParseError();
            return ERR_INVALID_ARG;
        }
        int ret = jsonobject2pb(&d, msg, err, auto_convert);
        return ret;
    }
    int jsonobject2pb(const rapidjson::Value* json, google::protobuf::Message& msg, std::string* err, bool auto_convert)
    {
        int ret = 0;

        try
        {
            ret = parse_json(json, msg, err, auto_convert);
        }
        catch (std::exception* e)
        {
			SNLOG(snqu::kError, "jsonobject2pb exception [%s]", e->what());
        }
        catch (...)
        {
			SNLOG(snqu::kFatal, "jsonobject2pb exception unhandle");
        }

        return ret;
    }

}
