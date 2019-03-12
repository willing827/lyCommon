/*

 */

#ifndef PBJSON_HPP_
#define PBJSON_HPP_

#include <string>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <rapidjson/rpdjson.h>
#include <google/protobuf/util/field_mask_util.h>

#define ERR_INVALID_ARG -1
#define ERR_INVALID_PB -2
#define ERR_UNKNOWN_FIELD -3
#define ERR_INVALID_JSON -4

namespace pbjson
{
    template <typename MsgT>
    inline void MsgAddMaskHelper(MsgT& message, std::string msg_dot_name)
    {
        using namespace google::protobuf;
        auto dot_pos = msg_dot_name.find(".");
        if (dot_pos != std::string::npos)
            msg_dot_name.erase(msg_dot_name.begin(), msg_dot_name.begin() + dot_pos + 1);
        else
        {
            dot_pos = msg_dot_name.find(">");
            if (dot_pos != std::string::npos)
                msg_dot_name.erase(msg_dot_name.begin(), msg_dot_name.begin() + dot_pos + 1);
        }
        dot_pos = msg_dot_name.find("(");
        if (dot_pos != std::string::npos)
            msg_dot_name.erase(msg_dot_name.begin() + dot_pos, msg_dot_name.end());
        util::FieldMaskUtil::AddPathToFieldMask<MsgT>(msg_dot_name.c_str(), message.mutable_mask());
    }

    template <typename MsgT>
    inline void MsgAddAllMask(MsgT& message)
    {
        google::protobuf::util::FieldMaskUtil::GetFieldMaskForAllFields<MsgT>(message.mutable_mask());
    }
    
#define MSG_ADD_MASK(msg, msg_dot_field_name) pbjson::MsgAddMaskHelper(msg, #msg_dot_field_name)
    /*
        proto 3.3后 默认值不再保存is_set属性,所以 0 和 "" 默认转换是没有该key值的，
        需要用默认值的就添加google.protobuf.FieldMask mask字段
    */
    std::string pb2json(const google::protobuf::Message& msg, bool isb64 = false, bool isall = false);
    void pb2jsonobject(const google::protobuf::Message& msg, rapidjson::Value& val, rapidjson::Value::AllocatorType& allocator, bool isb64 = false, bool isall = false);

    // auto_convert 会把满足条件的json string字段转为pb的number字段
    int json2pb(const std::string& json, google::protobuf::Message& msg, std::string* err = nullptr, bool auto_convert = true);
    int jsonobject2pb(const rapidjson::Value* json, google::protobuf::Message& msg, std::string* err = nullptr, bool auto_convert = true);

    template<class message_type>
    std::string pb2json(const std::vector<message_type>& msgs, bool isall = false)
    {
        rapidjson::Document d;
        auto& root = d.SetArray();

        for (auto& item : msgs)
        {
            rapidjson::Value val;
            pb2jsonobject(item, val, d.GetAllocator(), isall);
            root.PushBack(val, d.GetAllocator());
        }

        return DumpStr(d);
    }

    template<class message_type>
    int  json2pb(const std::string& json, std::vector<message_type>& msgs, std::string* err = nullptr, bool auto_convert = true)
    {
        rapidjson::Document d;
        d.Parse<0>(json.c_str());
        if (d.HasParseError() || !d.IsArray())
        {
            if (nullptr != err)
                *err += d.GetParseError();
            return ERR_INVALID_ARG;
        }

		size_t max = d.Size();
		for (size_t i = 0; i < max; i++)
		{
			const auto& value = d[i];
			message_type new_item;
			int ret = jsonobject2pb(&value, new_item, err, auto_convert);
			if (ret) 
                return ret;
			msgs.push_back(std::move(new_item));
		}
        return 0;
    }

    template<class message_type>
    int  json2pb(const std::string& json, std::vector<std::shared_ptr<message_type>>& msgs, std::string* err = nullptr, bool auto_convert = true)
    {
        rapidjson::Document d;
        d.Parse<0>(json.c_str());
        if (d.HasParseError())
        {
            if (nullptr != err)
                *err += d.GetParseError();
            return ERR_INVALID_ARG;
        }

        if (!d.IsArray() && d.IsObject())
        {
            auto new_item = std::make_shared<message_type>();
            int ret = jsonobject2pb(&d, *new_item, err, auto_convert);
            if (ret) return ret;
            msgs.push_back(std::move(new_item));
            return 0;
        }
        size_t max = d.Size();
        for (size_t i = 0; i < max; i++)
        {
            const auto& value = d[i];
            auto new_item = std::make_shared<message_type>();
            int ret = jsonobject2pb(&value, *new_item, err, auto_convert);
            if (ret) return ret;
            msgs.push_back(std::move(new_item));
        }
        return 0;
    }
}


#endif /* PBJSON_HPP_ */
