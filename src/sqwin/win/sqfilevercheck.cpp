#include <sqwin/win/sqfilevercheck.h>
#include <sqwin/win/sqwindows.h>
#include <sqstd/sqsysfun.h>
#include <sqstd/sqstringhelper.h>
#include <sqwin/win/sqfilesignature.h>
#include <sqstd/sqfilerw.h>
#include <codec/sqcodec.h>
#include <rapidjson/rpdjson.h>
#include <sqwin/win/sqpath.h>

namespace snqu{ namespace vercheck{

bool CheckFileVersion(const std::string& full_path, const std::string& check_data, int check_logic)
{
    auto file_version = GetFileVersion(full_path);

    auto check_rst = str::version_compare(file_version, check_data);

    switch (check_rst)
    {
    case 0:
        if (check_logic <= Check_Logic_BelowEqual)
        {
            return true;
        }
        break;
    case 1:
        if (check_logic == Check_Logic_BeyondEqual ||
            check_logic == Check_Logic_Beyond ||
            check_logic == Check_Logic_Unequal)
        {
            return true;
        }
        break;
    case -1:
        if (check_logic == Check_Logic_BelowEqual ||
            check_logic == Check_Logic_Below ||
            check_logic == Check_Logic_Unequal)
        {
            return true;
        }
        break;
    default:
            return false;
        break;
    }

    return false;
}

bool CheckFileMD5(const std::string& full_path, const std::string& check_data, int check_logic)
{
    auto file_md5 = codec::MD5File(full_path);
    bool rel = str::nequal(file_md5, check_data);
	if (check_logic == Check_Logic_Unequal)
		rel = !rel;
    return rel;
}

bool CheckFileSignatrue(const std::string& full_path, const std::string& check_data)
{
    if (check_data.empty())
    {
        return !os::check_file_signature(full_path);
    }

    auto check_list = str::split(check_data, "^");
    return !os::check_file_signature(full_path, check_list);
}

CheckResult VerCheck(int termial_type, const std::string& check_param)
{
    std::string param = codec::B64Decode(check_param);

    CheckResult rst;
    if (param.empty())
        return rst;
    
    // 遍历参数
    using namespace rapidjson;
    Document doc;
    doc.Parse<0>(param.c_str());
    if (doc.HasParseError()) return rst;

    if (doc.HasMember("version_check") && doc["version_check"].IsArray())
    {
        std::string full_path;
        for (size_t i = 0; i < doc["version_check"].Size(); i++)
        {
            Value& item = doc["version_check"][i];

            if (termial_type != item["terminal_type"].GetInt())
                continue;

            // 提取文件路径
            if (!item["relative_path"].Empty())
            {
                std::string temp_path = snqu::path::get_module_path();
                temp_path.append("\\").append(item["relative_path"].GetString());
                temp_path.append(item["file_name"].GetString());
                full_path = snqu::path::absolute_path(temp_path);
            }
            else
            {
                full_path = snqu::path::get_module_path() + "\\" + item["file_name"].GetString();
            }

            // 调用上报服务接口
            bool check_rst = true;

            if (!snqu::FileRW::Exist(full_path))
            {
                switch (item["check_type"].GetInt())
                {
                case Check_File_Version:
                {
                    if (item.HasMember("check_logic") && item.HasMember("check_data"))
                        check_rst = CheckFileVersion(full_path, item["check_data"].GetString(), item["check_logic"].GetInt());
                    break;
                }
                case Check_File_MD5:
                {
                    if (item.HasMember("check_logic") && item.HasMember("check_data"))
                        check_rst = CheckFileMD5(full_path, item["check_data"].GetString(), item["check_logic"].GetInt());
                    break;
                }
                case Check_File_Signature:
                    check_rst = CheckFileSignatrue(full_path, codec::U2A(item["check_data"].GetString()));
                    break;
                default:
                    break;
                }
            }
            else
                check_rst = false;

            if (check_rst)
            {
                rst.check_rst = -1;
                rst.deal_logic = item["deal_method"].GetInt();
                if (!item["deal_tip"].Empty())
                    rst.tip_msg = item["deal_tip"].GetString();
                rst.err_file_path = full_path;
                return rst;
            }
        }
        
    }

    return rst;
}

}}




