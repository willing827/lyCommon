/*------------------------------------------------------------------------- --*/
/*  sqdb.h                                                                   */
/*                                                                           */
/*  History                                                                  */
/*      05/21/2015                                                           */
/*                                                                           */
/*  Author                                                                   */
/*                                                                    */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*-------------------------------------------------------------------------  --*/

// description: database inteface for calling by UI layer

#ifndef SQDB_H
#define SQDB_H
//#pragma once

#include <string>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <list>
#include <map>
#include <memory>
#include <thread>
#include "google/protobuf/message.h"
#include "sqdbuti.h"

using namespace std;
using namespace google;

typedef struct st_mysql_res MYSQL_RES;
class SQDBManager;
class SQMysql;

class SQDB
{
	friend class SQDBManager;
private:
	SQDB(void);
public:
	virtual ~SQDB(void);
public:
	static SQDB& instance();
    void set_owner_id(std::thread::id);
    std::thread::id& get_owner_id();
    int convert_binary_string(std::string& in_data);
private:
	int open(const string& __in host, int __in port, const string& __in user_name, const string& __in password, const string& __in db_name);
	bool keep_alive(); // return true connect is valid, return false connect is invalid
	int close();	
	int sql_execute(const string& __in sql_statement, __int64* __out insert_auto_id = nullptr); // for insert, update, delete
	int sql_query(const string& __in sql_statement, IDataSet& __out result); // for select
	int sql_query(const string& __in sql_statement, vector<vector<FieldAttribute> >& __out result); 
	template<class message_type>
	int sql_query(const string& __in sql_statement, vector<message_type>& __out result)
	{
		int error_code = 0;
		vector<vector<FieldAttribute> >  attribute_result;
		error_code = sql_query(sql_statement, attribute_result);
		for(size_t i = 0; i < attribute_result.size(); ++i)
		{
			message_type item;
			SQDBUti::parse_attribute(attribute_result[i], item);
			result.push_back(item);
		}
		return error_code;
	}
	int insert_items(const string& __in table_name, const IDataSet& __in insertdata, __int64* __out insert_auto_id = nullptr); 
	int insert_items(const string& __in table_name, const vector<FieldAttribute> & __in insert_data, __int64* __out insert_auto_id = nullptr); 
	int into_item(const string& __in insert_name, const string& __in table_name, const vector<FieldAttribute> & __in insert_data, __int64* __out insert_auto_id = nullptr); 
	int insert_items(const string& __in table_name, const vector<vector<FieldAttribute> >& __in insert_datas); 
	int insert_items(const string& __in table_name, const protobuf::Message& __in insert_data, __int64* __out insert_auto_id = nullptr);
    template<class message_type>
	int insert_items(const string& __in table_name, const vector<message_type>& __in insertdata)
	{
		int error_code = 0;
		for (auto iter = insertdata.begin(); iter != insertdata.end(); ++iter)
		{				
			error_code = insert_items(table_name, *iter);
			if(error_code)
			{
				break;
			}
		}
		return error_code;
	}
	int get_items(const string& __in table_name, IDataSet& __out result);
	int get_items(const string& __in table_name, const string& __in condition, const string& __in tail_condition, IDataSet& __out result);
	int get_items(const string& __in table_name, const vector<FieldAttribute>* __in select_fields, const vector<WhereField>* __in where_fields, const string& __in limit_condition, vector<vector<FieldAttribute> >& __out result);
	int get_items(const string& __in table_name, const vector<FieldAttribute>* __in select_fields, SqlStream& __in where_fields, const string& __in limit_condition, vector<vector<FieldAttribute> >& __out result);
	template<class message_type>
	int get_items(const string& __in table_name, const vector<FieldAttribute>* __in select_fields, const vector<WhereField>* __in where_fields, const string& __in limit_condition, vector<message_type>& __out result)
	{
		int error_code = 0;
		vector<vector<FieldAttribute> >  attribute_result;
		if(select_fields)
		{
			error_code = get_items(table_name, select_fields, where_fields, limit_condition, attribute_result);
		}
		else
		{
			message_type item;	
			vector<FieldAttribute> select_fields;
			SQDBUti::parse_message(item, select_fields);
			error_code = get_items(table_name, &select_fields, where_fields, limit_condition, attribute_result);
		}
		for(size_t i = 0; i < attribute_result.size(); ++i)
		{
			message_type item;
			SQDBUti::parse_attribute(attribute_result[i], item);
			result.push_back(item);
		}
		return error_code;
	}
	//update table, if where_fields is null, make primary keys of update_data to where condition, if where_fields is not null, but where_fields is empty will not where condition
	int update_items(const string& __in table_name, const vector<WhereField>* __in where_fields, const vector<FieldAttribute> & __in update_data);
	int update_items(const string& __in table_name, const vector<WhereField>* __in where_fields, const protobuf::Message& __in update_data);
	// if item exist then delete item and insert new item else only insert new item
	int replace_items(const string& __in table_name, const vector<WhereField>* __in where_fields, const vector<FieldAttribute> & __in replace_data);
	int replace_items(const string& __in table_name, const vector<WhereField>* __in where_fields, const protobuf::Message& __in replace_data);
	// if item exist then update item else insert new item
    int copy_items(const string& __in table_name, const vector<WhereField>* __in where_fields, const vector<FieldAttribute> & __in copy_data);
    int copy_items(const string& __in table_name, const vector<WhereField>* __in where_fields, const protobuf::Message& __in copy_data);

	int delete_items(const string& __in table_name, const vector<WhereField>* __in where_fields = nullptr);
	bool record_exist(const string& __in table_name, const vector<WhereField>* __in where_fields);
	bool record_exist(const string& __in table_name,SqlStream& __in where_fields);
	int transaction_begin();
	int transaction_commit();
	int transaction_rollback();
	int parse_wherefield(const vector<WhereField>* __in where_fields, string& __out where_field_value);
private:
	//static int parse_sqlresult(MYSQL_RES* __in sql_result, vector<string>* __out field_names, vector<protobuf::FieldDescriptor::CppType>* __out field_types, vector<vector<string> >* __out results);
	int parse_sqlresult(MYSQL_RES* __in sql_result, vector<vector<FieldAttribute> >& __out results);
	int get_primary_key(const string& __in db_name, const string& __in table_name, vector<PrimaryKey>& __out primary_keys);
	int get_auto_incement_primary_key_name(const string& __in table_name, string& __out primary_key_name);
	int get_where_field(const string& __in table_name, const vector<FieldAttribute>& __in update_data, vector<WhereField>& __out where_field);
    string m_connect_str;
    string m_db_name;    
	SQMysql* m_mysql;
    std::thread::id m_thread_id;

};

#endif // SQDB_H 