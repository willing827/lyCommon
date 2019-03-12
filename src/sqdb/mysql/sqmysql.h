/*---------------------------------------------------------------------------*/
/*  sqmysql.h                                                                */
/*                                                                           */
/*  History                                                                  */
/*      05/21/2015                                                           */
/*                                                                           */
/*  Author                                                                   */
/*                                                                    */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*---------------------------------------------------------------------------*/

// description: mysql inteface for calling by database interface class

#ifndef SQMYSQL_H
#define SQMYSQL_H
//#pragma once

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <database/mysql/mysql.h>

using namespace::std;
class SQDB;

class SQMysql
{
	friend class SQDB;
private:
	SQMysql(void);
public:
	virtual ~SQMysql(void);
public:
	static SQMysql& instance();

public:
	int connect(const string& __in host, int __in port, const string& __in user_name, const string& __in password, const string& __in db_name);
	int disconnect();
	int sql_execute(const string& __in query, __int64* __out insert_auto_id = nullptr,bool is_convert_to_utf8 = true);
	int sql_query(const string& __in query, MYSQL_RES*& __out result);
	int get_result(MYSQL_RES*& __out result);
	void free_result(MYSQL_RES* __in result);
	int transaction_begin();
	int transaction_commit();
	int transaction_rollback();
	int ping();
	int next_result();

private:
	MYSQL* m_mysql;	

public:	
	static string  wstr_to_str(const wstring& wstr, int codepage);
	static wstring str_to_wstr(const string& str, int codepage);
	static string  asc_to_utf8(const string& str);
	static string  utf8_to_asc(const string& str);
	int utf8_encode(int32_t codepoint, char *buffer, int *size);
	int utf8_check_first(char byte);
	int utf8_check_full(const char *buffer, int size, int32_t *codepoint);
	const char *utf8_iterate(const char *buffer, int32_t *codepoint);
	int utf8_check_string(const char *string, int length);
};

#endif // SQMYSQL_H