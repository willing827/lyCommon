#include <sqdb/base/sqjsontosql.h>
#include <codec/sqcodec.h>

namespace snqu{ 

    using namespace rapidjson;

    void ColNameInsertSql(const rapidjson::Value& row, std::string& sqlstmt)
    {
        if (row.MemberCount() == 0 || !row.IsObject()) return;

        sqlstmt.append("(");
        for (auto iter = row.MemberBegin(); iter != row.MemberEnd(); ++iter)
        {
            if (0 == strcmp("unique_id", iter->name.GetString()))
                continue;
            sqlstmt.append(iter->name.GetString());
            sqlstmt.append(",");
        }
        sqlstmt.erase(--sqlstmt.end());
        sqlstmt.append(")");
    }

    void ValueToSql(std::string& sqlstmt, const rapidjson::Value& field_val)
    {
        switch (field_val.GetType())
        {
        case rapidjson::kStringType:
            if (codec::IsUTF8(field_val.GetString(), field_val.GetStringLength()))
                sqlstmt.append("\"").append(codec::U2A(field_val.GetString())).append("\"");
            else
                sqlstmt.append("\"").append(field_val.GetString()).append("\"");
            break;
        case rapidjson::kNumberType:
            sqlstmt.append(DumpStr(field_val));
            break;
        case rapidjson::kFalseType:
            sqlstmt.append("\"0\"");
            break;
        case rapidjson::kTrueType:
            sqlstmt.append("\"1\"");
            break;
        default:
            sqlstmt.append("\"\"");
            break;
        }
    }

    void FieldValInsertSql(const rapidjson::Value& row, std::string& sqlstmt)
    {
        if (row.MemberCount() == 0 || !row.IsObject()) return;
        
        for (auto iter = row.MemberBegin(); iter != row.MemberEnd(); ++iter)
        {
            if (0 == strcmp("unique_id",iter->name.GetString()))
                continue;
            ValueToSql(sqlstmt, iter->value);
            sqlstmt.append(",");
        }
    }

    std::string JsonToInsertSql(const std::string table_name, const rapidjson::Value& dtset)
    {
        std::string sql_stmt("INSERT INTO ");
        sql_stmt.append(table_name);
        if (dtset.IsObject())
        {
            ColNameInsertSql(dtset, sql_stmt);
            sql_stmt.append(" VALUES\r\n");
            FieldValInsertSql(dtset, sql_stmt);
        }
        else
        {
            if (dtset.Size() == 0) return "";
            auto row = dtset.Begin();
            ColNameInsertSql(*row, sql_stmt);
            sql_stmt.append(" VALUES\r\n");

            while(row != dtset.End())
            {
                sql_stmt.append("(");
                FieldValInsertSql(*row, sql_stmt);
                sql_stmt.erase(--sql_stmt.end());
                sql_stmt.append("),\r\n");
                row++;
            }
            sql_stmt.erase(sql_stmt.end() - 3, sql_stmt.end());
        }
        return sql_stmt;
    }

    std::string JsonToUpdateSql(const std::string table_name, const rapidjson::Value& row,
         std::set<std::string>* except_col_name, std::string* except_val)
    {
        std::string sql_stmt("UPDATE ");
        sql_stmt.append(table_name).append(" SET ");
        for (auto iter = row.MemberBegin(); iter != row.MemberEnd(); ++iter)
        {
            if (except_col_name != nullptr && except_col_name->count(iter->name.GetString()) > 0)
            {
                if (nullptr != except_val)
                {
                    if (!except_val->empty())
                    {
                        except_val->append(" AND ");
                    }
                    except_val->append(iter->name.GetString());
                    except_val->append("=");
                    ValueToSql(*except_val, iter->value);
                }
                continue;
            }
            sql_stmt.append(iter->name.GetString());
            sql_stmt.append("=");
            ValueToSql(sql_stmt, iter->value);
            sql_stmt.append(",");
        }
        sql_stmt.erase(--sql_stmt.end());
        if (nullptr != except_val && !except_val->empty())
            except_val->erase(--except_val->end());
        return sql_stmt;
    }

}


