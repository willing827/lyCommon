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
    //֧�������������
    std::string JsonToInsertSql(const std::string table_name, const rapidjson::Value& dtset);
    //ֻ֧�ֵ�������
    //except_col_name�������ų���Ҫ���ֶ�
    //except_val���԰�except_col_name�����ֶε�name=value and ...����
    std::string JsonToUpdateSql(const std::string table_name, const rapidjson::Value& row,
                                std::set<std::string>* except_col_name = nullptr, std::string* except_val = nullptr);
}


#endif // SQDB_JSON_SQL_H