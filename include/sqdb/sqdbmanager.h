/*------------------------------------------------------------------------- --*/
/*  SQDBManager.h                                                        */
/*                                                                           */
/*  History                                                                  */
/*      05/28/2015                                                           */
/*                                                                           */
/*  Author                                                                   */
/*                                                                    */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*------------------------------------------------------------------------  --*/

// description: database connect pool

#ifndef SQDBMANAGER_H
#define SQDBMANAGER_H
//#pragma once

#include <map>
#include <mutex>
#include <thread>
#include <sqstd/sqsingleton.h>
#include "sqdb.h"
using namespace std;

class SQDB;

class SQDBManager
{
	typedef struct tagConnectParameter
	{
		string  host;
		int     port;
		string  user_name;
		string  password; 
		string  db_name;

	}ConnectParameter;

public:
	SQDBManager(void);
	virtual ~SQDBManager(void);
public:
	int set_connect_parameter(const string& __in host, int __in port, const string& __in user_name, 
                              const string& __in password, const string& __in db_name, const char* loger_id = nullptr);
    void uninit();
    bool keep_alive();
	int set_pool_parameter(int pool_size);
	int sql_execute(const string& __in sql_statement, __int64* __out insert_auto_id = nullptr); // for insert, update, delete
	int sql_query(const string& __in sql_statement, IDataSet& __out result); // for select
	int sql_query(const string& __in sql_statement, vector</*row*/vector/*column*/<FieldAttribute> >& __out result); 
public:
    int insert_json(const string& sql_json, const std::string& table_name, __int64* insert_auto_id = nullptr);
///add new 1-7
public:
	int get_items(const string& __in table_name, SqlStream & where_filed,IDataSet& __out result);
	int update_items(const string& table_name, SqlStream& __in update_data,SqlStream & where_filed);
	int replace_items(const string& __in table_name, IDataSet& __in data_set,SqlStream & where_filed);
	int copy_items(const string& __in table_name, SqlStream& __in update_data,IDataSet& __in insert_data,SqlStream & where_filed);
	int delete_items(const string& table_name, SqlStream & where_filed);

public:
	int insert_items(const string& __in table_name, const IDataSet& __in insertdata, __int64* __out insert_auto_id = nullptr);
	int insert_items(const string& __in table_name, const vector<FieldAttribute> & __in insert_data, __int64* __out insert_auto_id = nullptr); 
	int insert_items(const string& __in table_name, const vector<vector<FieldAttribute> >& __in insert_data); 
	int insert_items(const string& __in table_name, const protobuf::Message& __in insert_data, __int64* __out insert_auto_id = nullptr);

	template<class message_type>
	int insert_items(const string& __in table_name, const vector<message_type>& __in insertdata)
	{
		int error_code = 0;
		SQDB* connect = nullptr;
		error_code = get_connect(connect);
		if(connect)
		{
			error_code =connect->insert_items(table_name, insertdata);
			free_connect(connect);
			connect = nullptr;
		}
		return error_code;
	}

    template<class message_type>
    int get_items(const string& __in table_name, const vector<FieldAttribute>* __in select_fields, 
        const vector<WhereField>* __in where_fields, const string& __in limit_condition, vector<message_type>& __out result)
    {
        int error_code = 0;
        SQDB* connect = nullptr;
        error_code = get_connect(connect);
        if(connect)
        {
            error_code =connect->get_items(table_name, select_fields, where_fields, limit_condition, result);
            free_connect(connect);
            connect = nullptr;
        }
        return error_code;
    }
//private:
    template<class message_type>
    int sql_query(const string& __in sql_statement, vector<message_type>& __out result)
    {
        int error_code = 0;
        SQDB* connect = nullptr;
        error_code = get_connect(connect);
        if(connect)
        {
            error_code =connect->sql_query(sql_statement, result);
            free_connect(connect);
            connect = nullptr;
        }
        return error_code;
    }
public:

	int get_items(const string& __in table_name, IDataSet& __out result);
	int get_items(const string& __in table_name, const string& __in condition, const string& __in tail_condition, IDataSet& __out result);
    int get_items(const string& __in table_name, const vector<FieldAttribute>* __in select_fields, 
        const vector<WhereField>* __in where_fields, const string& __in limit_condition, vector<vector<FieldAttribute> >& __out result);

	int delete_items(const string& __in table_name, const vector<WhereField>* __in where_fields = nullptr);
	bool record_exist(const string& __in table_name, const vector<WhereField>* __in where_fields);
	bool record_exist(const string& __in table_name, SqlStream& __in where_fields);

    int copy_items(const string& __in table_name, const vector<WhereField>* __in where_fields, const protobuf::Message& __in copy_data);

	int do_transaction(std::function<int(void)> trans_func);

    int convert_binary_string(std::string& data); // Ð§ÂÊµÍÏÂ

private:
	int get_connect(SQDB*& __in __out connect);
	int free_connect(SQDB* __in connect);
	int clear_connect_pool();
	int init_connect_pool();

private:
	map<SQDB*, bool>  m_connect_pool;
	size_t m_pool_size;
	ConnectParameter m_connect_parameter;
	mutex m_pool_mutex;
};

typedef SQSingleton<SQDBManager> sglDbMgr;

#endif // SQDBMANAGER_H