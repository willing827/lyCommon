/*------------------------------------------------------------------------- --*/
/*  sqdbuti.h                                                                */
/*                                                                           */
/*  History                                                                  */
/*      07/8/2015                                                           */
/*                                                                           */
/*  Author                                                                   */
/*                                                                    */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*-------------------------------------------------------------------------  --*/

// description: 提供工和数据库操作相关的工具函数

#ifndef SQDBUTI_H
#define SQDBUTI_H
//#pragma once

#include <string>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <list>
#include <map>
#include <memory>
#include "google/protobuf/message.h"
#include <sstream>
#include <functional>

namespace snqu { namespace db {
        // 支持的数据库
        enum DBType
        {
            DB_Mysql = 0,
            DB_Sqlite3,
        };

typedef std::vector<std::string> DBConnParam;

        enum MysqlConnParam
        {
            DBConn_host = 0,
            DBConn_port,
            DBConn_user_name,
            DBConn_password,
            DBConn_db_name,

            DBConn_Mysql_size,
        };

        enum Sqlite3ConnParam
        {
            Conn_Sqlite_Path = 0,    // 文件路径 
            Conn_Sqlite_Aeskey,      // 加密KEY 

            DBConn_Sqlite3_size,
        };

        enum DBError
        {
            ERR_ERR = -1,	//失败
            ERR_OK = 0,	    //成功
            ERR_DB_CONN,	//连接错误 
            ERR_DB_PARAM,	//参数错误
            ERR_DB_ENCYPT,	//加密出错
            ERR_DB_EXEC,	//操作出错
            ERR_DB_FILE_RW,	//文件读写出错
            ERR_DB_SSL,		//证书校验失败
            ERR_DB_TRANS,	//事务失败
            ERR_DB_QUERY,	//获取数据出错
            ERR_DB_POOL_MAX,//连接池已满
            ERR_DB_NO_COND, //无修改条件
        };

        // 支持的数据类型
        enum DBDataType
        {
            DBType_UNKNOW = 0,
            DBType_INT32 = 1,
            DBType_INT64 = 2,
            DBType_UINT32 = 3,
            DBType_UINT64 = 4,
            DBType_DOUBLE = 5,
            DBType_BLOB = 6,
            DBType_STRING = 7,
        };
}}

typedef  std::shared_ptr<google::protobuf::Message> proto_ptr;

class IDataSet
{
public:
    virtual proto_ptr new_item() = 0;
    virtual proto_ptr add_item(proto_ptr item) = 0;
    virtual void foreach_item(std::function<void(const proto_ptr)>) const = 0;
};

template <typename MESSAGE>
class DataSet : public IDataSet
{
    typedef std::shared_ptr<MESSAGE> Msg_ptr;
public:
    DataSet()
    {}

    ~DataSet()
    {
        clear();
    }

    virtual proto_ptr new_item() override
    {
        auto m = std::make_shared<MESSAGE>();	
        return m;
    };
    virtual proto_ptr add_item(proto_ptr item) override
    {		
        m_data_items.push_back(static_pointer_cast<MESSAGE>(item));
        return item;
    };

    virtual void foreach_item(std::function<void(const proto_ptr)> call_func) const override
    {
        for (const auto iter : m_data_items)
        {
            call_func(iter);
        }
    }

    std::list<Msg_ptr>& get_items()
    {
        return m_data_items;
    }

    void clear()
    {
        m_data_items.clear();
    }

    int size()
    {
        return m_data_items.size();
    }

private:
    std::list<Msg_ptr> m_data_items;
    bool m_is_release;
};

typedef struct tagPrimaryKey
{
    std::string name;
    int    type; // use google::protobuf::FieldDescriptor::CppType type
    bool   auto_increment;
    tagPrimaryKey()
    {
        auto_increment = false;
    }
}PrimaryKey;


typedef struct tagFieldAttribute
{
private:
	std::string _name;
	int    _type; // use google::protobuf::FieldDescriptor::CppType type
	std::string _value;
	bool   _has_value; // 2015.7.17 替代 option，保留option的目的是为了兼容已有代码, has_value设置为true的条件是value被赋值，option 是根据 message的 HasField
	static std::map<std::string, int> encrypt_field;
    static std::map<std::string, int> utf8_field;
	static std::string           aes_key;
	bool is_encrypt;
public:
	//bool   option;

public:
	tagFieldAttribute();
public:
	void name(const std::string& __name);
    const std::string& name() const;
	bool has_value() const;
	int  type() const;
	std::string value_decrypt(const std::string& value,int type,bool has_value,bool is_encrypt);
	std::string value(const std::string& _value, int type = google::protobuf::FieldDescriptor::CppType::CPPTYPE_STRING, bool has_value = true);
	std::string value(int _value);
	std::string value(unsigned int _value);
	std::string value(__int64 _value);
	std::string value(unsigned __int64 _value);
	std::string value() const;	
    const std::string& refvalue() const;
	std::string sql_value() const;
	static std::string add_quto(const std::string& value);
	std::string encrypt_sql_value()const;

	static std::string encrypt_value(const std::string& sql_value);
    static std::string encrypt_valueex(const std::string& sql_value);

	static bool is_encrypt_field(const std::string& name, std::string& short_name);
    static bool is_utf8_field(const std::string& name, std::string& short_name);

	static int set_encrypt_cfg(const std::map<std::string/*字段名*/, int/*字段类型*/>& _encrypt_field, const std::string& _aes_key);
///////12-5	///////encrypt 

	void set_encrpyt_flag(bool flag);

	typedef std::function<std::string(const std::string& indata)> fun_handle_ency;
	static fun_handle_ency m_fun_encry;
	static fun_handle_ency m_fun_decry;

	static int set_encrypt_cfg(const std::map<std::string/*字段名*/, int/*字段类型*/>& _encrypt_field, fun_handle_ency fun_ency, fun_handle_ency fun_decry);
	static int set_utf8_cfg(const std::map<std::string/*字段名*/, int/*字段类型*/>& _encrypt_field);
	static std::string m_key;
}FieldAttribute;

typedef struct tagWhereField
{
    std::string compare;
    std::string right_link;
    std::string left_bracket;
    std::string right_bracket;
    FieldAttribute field;
}WhereField;

typedef  std::shared_ptr<google::protobuf::Message> proto_ptr;

class SQDBUti
{

private:
	SQDBUti(void);
public:
	virtual ~SQDBUti(void);

public:
	static int parse_message(const google::protobuf::Message& __in message, std::vector<FieldAttribute>& __out field_attributes, bool has_value = false, bool is_all = false);

	static int parse_attribute(const std::vector<FieldAttribute>& __in field_attributes, google::protobuf::Message& __out message);
	static FieldAttribute* find_field_attribute_by_name(std::vector<FieldAttribute>& __in field_attributes, const std::string& name);
	static const FieldAttribute* find_field_attribute_by_name(const std::vector<FieldAttribute>& __in field_attributes, const std::string& name);
	static int set_encrypt_option(const std::map<std::string/*字段名*/, int/*字段类型*/>& encrypt_field, const std::string& aes_key);

};

enum opertype
{
    ssnull = -1,
    ssbracketl = 0,
    ssbracketr,
    ssisequal,
    ssunequal,
    ssbigger,
    sssmaller,
    sswhere,
    ssand,
	ssset,
	ssdot,
};

struct SqlOper
{
    opertype m_type;

    SqlOper(opertype type = ssnull)
    {
        m_type = type;
    }
};

class SqlStream
{
public:
    SqlStream();
    ~SqlStream(){}
	SqlStream::SqlStream(const SqlStream &){}
    SqlStream& operator<< (__int64 data);
    SqlStream& operator<< (int data);
    SqlStream& operator<< (std::string data);
    SqlStream& operator<< (char* data);
    SqlStream& operator<< (const opertype& data);
    std::string str() { return m_ss.str(); }
    void clear() { m_ss.str(""); };
private:
    bool is_value;
    std::string last_item_name;
	std::stringstream m_ss;
};


#endif // SQDBUTI_H