/*---------------------------------------------------------------------------*/
/*  sqdbdataset.h                                                            */
/*                                                                           */
/*  History                                                                  */
/*      05/22/2017 create                                                    */
/*                                                                           */
/*  Author                                                                   */
/*      feng hao                                                             */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*---------------------------------------------------------------------------*/
#ifndef SQDB_DB_RECORD_H
#define SQDB_DB_RECORD_H

#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <map>
#include <set>
#include <sqstd/sqconvert.h>
#include <rapidjson/rpdjson.h>
#include <sqstd/sqsingleton.h>
#include "sqdbencryptor.h"

namespace snqu{ namespace db{

    namespace detail {

        class DBDataSetBase
        {
            friend class SQDBPool;
        public:
            DBDataSetBase(DBEncryptor* input = nullptr)
            {
                Clear();
                m_db_en_ptr = input;
            }

            void AddRow() { m_records.PushBack(rapidjson::Value(rapidjson::kObjectType), m_records.GetAllocator()); }
            inline auto GetColName(const char* colname)
            {
                colname = detail::ShortName(colname);

                auto col_name = m_col_names.find(colname);
                if (col_name == m_col_names.end())
                {
                    col_name = m_col_names.insert(m_col_names.end(), colname);
                }
                return col_name;
            }

            inline void SetVal(const char* colname, const char* col_data, int size, bool is_encrypt = true)
            {
                auto col_name = GetColName(colname);
                if (m_records.Empty()) return;
                auto end = m_records.End(); end--;
                rapidjson::Value val;
                if (is_encrypt && nullptr != m_db_en_ptr && m_db_en_ptr->IsEnField(colname))
                {//加密处理
                    std::string temp = m_db_en_ptr->m_encrypt_fun(std::string(col_data, size));
                    val.SetString(temp.c_str(), temp.length(), m_records.GetAllocator());
                }
                else
                {
                    val.SetString(col_data, size, m_records.GetAllocator());
                }
                end->AddMember(rapidjson::StringRef(col_name->c_str()), val, m_records.GetAllocator());
            }

            template<class T>
            void SetVal(const char* colname, T col_val, bool is_encrypt = true)
            {
                auto col_name = GetColName(colname);
                if (m_records.Empty()) return;
                auto end = m_records.End(); end--;
                rapidjson::Value val(col_val);
                if (is_encrypt && nullptr != m_db_en_ptr && m_db_en_ptr->IsEnField(colname))
                {
                    std::string temp = std::to_string(col_val);
                    temp = m_db_en_ptr->m_encrypt_fun(temp);
                    val.SetString(temp.c_str(), temp.length(), m_records.GetAllocator());
                }
                end->AddMember(rapidjson::StringRef(col_name->c_str()), val, m_records.GetAllocator());
            }

            int  GetRowCount() const { return m_records.Size(); }
            bool IsEmpty() { return GetRowCount() == 0; }
            void ResetRow() const { m_cur_row_id = 0; }

            bool NextRow() const
            {
                m_cur_row_id++;
                if (m_cur_row_id >= GetRowCount())
                    return false;
                return true;
            }

            inline std::string Dump() const { return std::move(rapidjson::DumpStr(m_records)); }
            inline std::string DebugDump() const { return std::move(rapidjson::DumpPrettyStr(m_records)); }

            template <typename T>
            T GetVal(const char* colname) const
            {
                colname = detail::ShortName(colname);
                if (nullptr != m_db_en_ptr && m_db_en_ptr->IsEnField(colname))
                {
                    std::string des_data = m_db_en_ptr->m_decrypt_fun(GetValPri<std::string>(colname));
                    return str::ToNumber<T>(des_data);
                }
                return GetValPri<T>(colname);
            }

            template <>
            std::string GetVal<std::string>(const char* colname) const
            {
                colname = detail::ShortName(colname);
                if (nullptr != m_db_en_ptr && m_db_en_ptr->IsEnField(colname))
                {
                    std::string des_data = m_db_en_ptr->m_decrypt_fun(GetValPri<std::string>(colname));
                    return des_data;
                }
                return GetValPri<std::string>(colname);
            }

            template <>
            bool GetVal<bool>(const char* colname) const
            {
                colname = detail::ShortName(colname);
                if (nullptr != m_db_en_ptr && m_db_en_ptr->IsEnField(colname))
                {
                    std::string des_data = m_db_en_ptr->m_decrypt_fun(GetValPri<std::string>(colname));
                    return "true" == des_data;
                }
                return GetValPri<bool>(colname);
            }

            void Clear()
            {
                m_records.SetArray();
                m_records.Clear();
                m_cur_row_id = 0;
            }

            void PrintSet()
            {
                for (auto& item : m_col_names)
                {
                    printf("%s\r\n", item.c_str());
                }
            }

        protected:
            DBEncryptor* m_db_en_ptr;
            rapidjson::Document    m_records;   // 数据集
            std::set<std::string>  m_col_names; // 列名
            mutable int m_cur_row_id;

            template <typename T>
            T GetValPri(const char* colname) const { return RpjsGetBool(m_records[m_cur_row_id], colname); }
            template <>
            std::string GetValPri(const char* colname) const { return RpjsGetString(m_records[m_cur_row_id], colname); }
            template <>
            int GetValPri(const char* colname) const { return RpjsGetInt32(m_records[m_cur_row_id], colname); }
            template <>
            uint32_t GetValPri(const char* colname) const { return RpjsGetUint32(m_records[m_cur_row_id], colname); }
            template <>
            int64_t GetValPri(const char* colname) const { return RpjsGetInt64(m_records[m_cur_row_id], colname); }
            template <>
            uint64_t GetValPri(const char* colname) const { return RpjsGetUint64(m_records[m_cur_row_id], colname); }
            template <>
            float GetValPri(const char* colname) const { return RpjsGetFloat(m_records[m_cur_row_id], colname); }
            template <>
            double GetValPri(const char* colname) const { return RpjsGetDouble(m_records[m_cur_row_id], colname); }
        };
    }

    class DBDataSet : public snqu::db::detail::DBDataSetBase
    {
    public:
        friend class SQDBPool;
        DBDataSet()
            : snqu::db::detail::DBDataSetBase(&SQSingleton<DBEncryptor>::instance())
        {
        }

        void SetJsonData(rapidjson::Value& input_data)
        {
            if (!input_data.IsArray())
            {
                m_records.PushBack(input_data, m_records.GetAllocator());
            }
            else
            {
                auto axsiz = input_data.Size();
                for (size_t i = 0; i < axsiz; i++)
                {
                    m_records.PushBack(input_data[i], m_records.GetAllocator());
                }
            }
        }

        ~DBDataSet() {};

// #if _DEBUG
//         rapidjson::Value& GetJsonData() { return m_records; };
// #endif
    };

}
}


#endif // SQDB_DB_RECORD_H