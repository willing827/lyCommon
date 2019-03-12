/*---------------------------------------------------------------------------*/
/*  sqjsontosql.h                                                            */
/*                                                                           */
/*  History                                                                  */
/*      08/02/2017 create                                                    */
/*                                                                           */
/*  Author                                                                   */
/*      feng hao                                                             */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*---------------------------------------------------------------------------*/
#ifndef SQDB_JSON_SQL_H
#define SQDB_JSON_SQL_H
#include <rapidjson/rpdjson.h>
#include <set>

namespace snqu{ 
    //支持数组或单条数据
    std::string JsonToInsertSql(const std::string table_name, const rapidjson::Value& dtset);
    //只支持单条数据
    //except_col_name集可以排除不要的字段
    //except_val可以把except_col_name所有字段的name=value and ...返回
    std::string JsonToUpdateSql(const std::string table_name, const rapidjson::Value& row,
                                std::set<std::string>* except_col_name = nullptr, std::string* except_val = nullptr);
}


#endif // SQDB_JSON_SQL_H