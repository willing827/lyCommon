
#include <sqdb/sqdbmanager.h>
#include <sqdb/sqdb.h>
#include <thread>
#include <sqstd/sqtypes.h>
#include <sqstd/sqstringhelper.h>
#include <sqdb/sqdberr.h>
#include "sqdblog.h"


SQDBManager::SQDBManager(void)
{
	m_pool_size = 10;
}


SQDBManager::~SQDBManager(void)
{
}

void SQDBManager::uninit()
{
    clear_connect_pool();
}

int SQDBManager::set_connect_parameter(const string& __in host, int __in port, const string& __in user_name, 
                                       const string& __in password, const string& __in db_name, const char* loger_id)
{
    static bool called = false;
    if (!called)
    {
        called = true;
        if (nullptr  == loger_id)
            AddLogModule(KLogModuleId, ktLoggerId);
        else
            AddLogModule(KLogModuleId, loger_id);
        MDLOG(kTrace, "SQDBManager create ");
    }

	int error_code = SQERROR_OK;
	m_connect_parameter.host      = host;
	m_connect_parameter.port      = port;
	m_connect_parameter.user_name = user_name;
	m_connect_parameter.password  = password;
	m_connect_parameter.db_name   = db_name;
    MDLOG(kDebug,"host:[%s] port:[%d] user_name:[%s] password:[%s] db_name:[%s]", host.c_str(), port, user_name.c_str(),
            password.c_str(), db_name.c_str());
	if(!m_connect_pool.size())
	{
		error_code = init_connect_pool();
	}

	return error_code;
}

bool SQDBManager::keep_alive()
{
	bool alive = true;
    lock_guard<mutex> lock(m_pool_mutex);
    for (auto& iter : m_connect_pool)
    {
        if(!iter.first->keep_alive())
        {
			alive = false;
			break;
        }
    }
	return alive;
}

int SQDBManager::set_pool_parameter(int pool_size)
{
	m_pool_size     = pool_size;
	return 0;
}

int SQDBManager::init_connect_pool()
{
	int error_code = SQERROR_OK;
	for (snqu::uint32 p_s = 0; p_s < m_pool_size; p_s++)
	{
		auto connect = new SQDB();
		if(!connect)
        {
            return SQERROR_DB_MEMEROY_ERR;
        }

		int reconnect_num = 0;
		do{
			error_code = connect->open(m_connect_parameter.host, m_connect_parameter.port,
				m_connect_parameter.user_name, m_connect_parameter.password, m_connect_parameter.db_name);
			if(error_code == 0 || reconnect_num++ >= 5 
			   || error_code == 1049 || error_code == 1045 || error_code == 1046)				
			{
				break;
			}
			this_thread::sleep_for(chrono::seconds(5));
			MDLOG(kTrace, "reconnect db");
		}while(1);

        if(error_code)
        {
            delete connect;
            connect = nullptr;
            if (m_connect_pool.size() == 0)
                return SQERROR_DB_CONNECT_ERR;
        }
        else
        {
            m_connect_pool[connect] = false;
        }
	}
	
	return SQERROR_OK;
}

int SQDBManager::get_connect(SQDB*& __in __out connect)
{
	int error_code = 0;
	connect = nullptr;

    assert(m_connect_pool.size() > 0);

    lock_guard<mutex> lock(m_pool_mutex);
	for (auto& iter : m_connect_pool)
	{
		if(iter.first->get_owner_id() == std::this_thread::get_id())
		{
			connect = iter.first;
			return SQERROR_OK;
		}
	}

	for (auto& iter : m_connect_pool)
	{
		if(!iter.second)
		{
			iter.second = true;
			connect = iter.first;
			if(!connect->keep_alive())
			{
				MDLOG(kTrace, "try db reconnect");
				connect->open(m_connect_parameter.host, m_connect_parameter.port,
					m_connect_parameter.user_name, m_connect_parameter.password, m_connect_parameter.db_name);
			}
			return SQERROR_OK;
		}
	}

	return SQERROR_DB_CONNECT_POOL_FULL_ERR;
}

int SQDBManager::free_connect(SQDB* connect)
{
    lock_guard<mutex> lock(m_pool_mutex);
	auto ss_data = m_connect_pool.find(connect);
	if (ss_data != m_connect_pool.end())
	{
		if (ss_data->first->get_owner_id() != std::thread::id())
		{
			return SQERROR_OK;
		}
		else
		{
			ss_data->second = false;
		}
	}
    
	return SQERROR_OK;
}

int SQDBManager::clear_connect_pool()
{
	lock_guard<mutex> lock(m_pool_mutex);
	for(auto& iter_pool : m_connect_pool)
	{
        iter_pool.first->close();
        delete iter_pool.first;	
	}
	m_connect_pool.clear();
	return SQERROR_OK;
}

int SQDBManager::sql_execute(const string& __in sql_statement, __int64* __out insert_auto_id)
{
	int error_code = 0;
	SQDB* connect = nullptr;
	error_code = get_connect(connect);
	if(connect)
	{
		error_code =connect->sql_execute(sql_statement, insert_auto_id);
		free_connect(connect);
		connect = nullptr;
	}
	return error_code;
}

int SQDBManager::sql_query(const string& __in sql_statement, IDataSet& __out result)
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

int SQDBManager::sql_query(const string& __in sql_statement, vector<vector<FieldAttribute> >& __out result) 
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

int SQDBManager::insert_items(const string& __in table_name, const IDataSet& __in result, __int64* __out insert_auto_id)
{
	int error_code = 0;
	SQDB* connect = nullptr;
	error_code = get_connect(connect);
	if(connect)
	{
		error_code =connect->insert_items(table_name, result, insert_auto_id);
		free_connect(connect);
		connect = nullptr;
	}
	return error_code;
}

int SQDBManager::insert_items(const string& __in table_name, const vector<FieldAttribute> & __in insert_data, __int64* __out insert_auto_id) 
{
	int error_code = 0;
	SQDB* connect = nullptr;
	error_code = get_connect(connect);
	if(connect)
	{
		error_code =connect->insert_items(table_name, insert_data, insert_auto_id);
		free_connect(connect);
		connect = nullptr;
	}
	return error_code;
}

int SQDBManager::insert_items(const string& __in table_name, const vector<vector<FieldAttribute> >& __in insert_data) 
{
	int error_code = 0;
	SQDB* connect = nullptr;
	error_code = get_connect(connect);
	if(connect)
	{
		error_code =connect->insert_items(table_name, insert_data);
		free_connect(connect);
		connect = nullptr;
	}
	return error_code;
}

int SQDBManager::insert_items(const string& __in table_name, const protobuf::Message& __in insert_data, __int64* __out insert_auto_id)
{
	int error_code = 0;
	SQDB* connect = nullptr;
	error_code = get_connect(connect);
	if(connect)
	{
		error_code =connect->insert_items(table_name, insert_data, insert_auto_id);
		free_connect(connect);
		connect = nullptr;
	}
	return error_code;
}

int SQDBManager::get_items(const string& __in table_name, IDataSet& __out result)
{
	int error_code = 0;
	SQDB* connect = nullptr;
	error_code = get_connect(connect);
	if(connect)
	{
		error_code =connect->get_items(table_name, result);
		free_connect(connect);
		connect = nullptr;
	}
	return error_code;
}

int SQDBManager::get_items(const string& __in table_name, const string& __in condition, const string& __in tail_condition, IDataSet& __out result)
{
	int error_code = 0;
	SQDB* connect = nullptr;
	error_code = get_connect(connect);
	if(connect)
	{
		error_code =connect->get_items(table_name, condition, tail_condition, result);
		free_connect(connect);
		connect = nullptr;
	}
	return error_code;
}

int SQDBManager::get_items(const string& __in table_name, const vector<FieldAttribute>* __in select_fields, 
                           const vector<WhereField>* __in where_fields, const string& __in limit_condition, 
                           vector<vector<FieldAttribute> >& __out result)
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

int SQDBManager::delete_items(const string& __in table_name, const vector<WhereField>* __in where_fields)
{
	int error_code = 0;
	SQDB* connect = nullptr;
	error_code = get_connect(connect);
	if(connect)
	{
		error_code =connect->delete_items(table_name, where_fields);
		free_connect(connect);
		connect = nullptr;
	}
	return error_code;
}

bool SQDBManager::record_exist(const string& __in table_name, const vector<WhereField>* __in where_fields)
{
	bool is_exist = false;
	SQDB* connect = nullptr;
	get_connect(connect);
	if(connect)
	{
		is_exist =connect->record_exist(table_name, where_fields);
		free_connect(connect);
		connect = nullptr;
	}
	return is_exist;
}
bool SQDBManager::record_exist(const string& __in table_name, SqlStream& __in where_fields)
{
	bool is_exist = false;
	SQDB* connect = nullptr;
	get_connect(connect);
	if(connect)
	{
		is_exist =connect->record_exist(table_name, where_fields);
		free_connect(connect);
		connect = nullptr;
	}
	return is_exist;
}

int SQDBManager::do_transaction(std::function<int(void)> trans_func)
{
	int error_code = 0;
	SQDB* connect = nullptr;
	error_code = get_connect(connect);

	if(connect)
	{
        if (connect->get_owner_id() != std::thread::id())
        {// 已经在事务中
            return trans_func();
        }
        else
        {
            connect->set_owner_id(std::this_thread::get_id());
        }

		error_code = connect->transaction_begin();
		if(SQERROR_OK != error_code)
        {
            connect->set_owner_id(std::thread::id());
			return error_code;
        }

		error_code = trans_func();
		if(error_code == SQERROR_OK)
			error_code = connect->transaction_commit();

		if(SQERROR_OK != error_code)
		{
			connect->transaction_rollback();
		}

        connect->set_owner_id(std::thread::id());

        free_connect(connect);
	}

	return error_code;
}

int SQDBManager::convert_binary_string(std::string& data)
{
    int error_code = 0;
    SQDB* connect = nullptr;
    error_code = get_connect(connect);
    if(connect)
    {
        error_code = connect->convert_binary_string(data);
        free_connect(connect);
        connect = nullptr;
    }
    return error_code;
}

int SQDBManager::get_items(const string& __in table_name, SqlStream&  where_filed,IDataSet& __out result)
{
	string sql_str = "select * from "+ table_name +" " +where_filed.str();

	return sql_query(sql_str, result);
}
int SQDBManager::update_items(const string& table_name,SqlStream& __in update_data,SqlStream & where_filed)
{

	string sql_str = "update "+ table_name +update_data.str() +" "+ where_filed.str();

	return sql_execute(sql_str);

}
int SQDBManager::replace_items(const string& __in table_name,IDataSet& __in data_set,SqlStream & where_filed)
{
	if(record_exist(table_name,where_filed))
	{
		delete_items(table_name,where_filed);
	}
	

	return insert_items(table_name,data_set);

}
int SQDBManager::copy_items(const string& __in table_name, SqlStream& __in update_data,IDataSet& __in insert_data,SqlStream & where_filed)
{
	int errn = 0;
	if(record_exist(table_name,where_filed))
	{
		errn = update_items(table_name,update_data,where_filed);
	}
	else
	{
		insert_items(table_name,insert_data);
	}

	return errn;
}

int SQDBManager::copy_items(const string& __in table_name, const vector<WhereField>* __in where_fields, const protobuf::Message& __in copy_data)
{
    int error_code = 0;
    SQDB* connect = nullptr;
    error_code = get_connect(connect);
    if(connect)
    {
        error_code =connect->copy_items(table_name, where_fields, copy_data);
        free_connect(connect);
        connect = nullptr;
    }
    return error_code;
}

int SQDBManager::delete_items(const string& table_name, SqlStream & where_filed)
{
	string sql_str = "delete  from "+ table_name + " " + where_filed.str();
	return sql_execute(sql_str);
}

namespace jsdetail{

    /*struct json_autoptr {
        json_t * ptr;
        json_autoptr(json_t *json) : ptr(json) {}
        ~json_autoptr() { if (ptr) json_decref(ptr); }
        json_t * release() { json_t *tmp = ptr; ptr = 0; return tmp; }
    };

    // 本地不处理的字段
    bool check_pass(const char *name)
    {
        if (str::nequal(name, "_id") || str::nequal(name, "site_id") || 
            str::nequal(name, "is_sync") || str::nequal(name, "isroam"))
            return true;
        return false;
    }

    void array2column(json_t *jt, const std::string& table_name, std::string& sqlss)
    {
        sqlss += "INSERT INTO ";
        sqlss += table_name;
        sqlss += " (";
        bool is_start = true;
        for (void *i = json_object_iter(jt); i; i = json_object_iter_next(jt, i))
        {  
            if (check_pass(json_object_iter_key(i)))
                continue;
            if (!is_start)
                sqlss += ",";
            else
                is_start = false;
            sqlss += json_object_iter_key(i);
        }
        sqlss += ") ";
    }

    void array2value(json_t *jt, std::string& valuess)
    {
        valuess += "(";
        bool is_start = true;
        for (void *i = json_object_iter(jt); i; i = json_object_iter_next(jt, i))
        {  
            const char *name = json_object_iter_key(i);
            if (check_pass(name))
                continue;
            if (!is_start)
                valuess += ",";
            else
                is_start = false;
            json_t *jf = json_object_iter_value(i);

            if (json_is_string(jf))
            {
                if (tagFieldAttribute::is_encrypt_field(name, std::string()))
                {
                    valuess += "'";
                    valuess += tagFieldAttribute::encrypt_valueex(json_string_value(jf));
                    valuess += "'";
                }
                else
                {
                    valuess += "'";
                    valuess += json_string_value(jf);
                    valuess += "'";
                }
            }
            else if(json_is_integer(jf))
            {
                if (tagFieldAttribute::is_encrypt_field(name, std::string()))
                    valuess += tagFieldAttribute::encrypt_valueex(std::to_string(json_integer_value(jf)));
                else
                    valuess += std::to_string(json_integer_value(jf));
            }
        }
        valuess += ")";
    }

    std::string jsarray2sql(const std::string& table_name, json_t *root)
    {
        std::string sqlss;
        sqlss.reserve(1024000);

        bool is_start = true;
        for (size_t j = 0; j < json_array_size(root); j++)
        {
            auto jt = json_array_get(root, j);
            if (!json_is_object(jt))
            {
                return "";
            }

            if (!is_start)
                sqlss += ",";
            else
            {
                array2column(jt, table_name, sqlss);
                sqlss += " VALUES";
                is_start = false;
            }
            array2value(jt, sqlss);
        }

        return sqlss;
    }

    std::string jsobj2sql(const std::string& table_name, json_t *root)
    {
        std::string columnsql, valuesql;
        columnsql.reserve(10240);
        columnsql += "INSERT INTO ";
        columnsql += table_name;
        columnsql += " (";
        valuesql += " VALUES (";
        bool is_start = true;
        for (void *i = json_object_iter(root); i; i = json_object_iter_next(root, i))
        {
            const char *name = json_object_iter_key(i);
            if (check_pass(name))
                continue;
            json_t *jf = json_object_iter_value(i);

            if (json_is_object(jf) || json_is_array(jf))
            {
                return "";
            }
            if (!is_start)
            {
                columnsql += ",";
                valuesql  += ",";
            }
            else
                is_start = false;
            columnsql += name;  

            if (json_is_string(jf))
            {
                if (tagFieldAttribute::is_encrypt_field(name, std::string()))
                {
                    valuesql += "'";
                    valuesql += tagFieldAttribute::encrypt_valueex(json_string_value(jf));
                    valuesql += "'";
                }
                else
                {
                    valuesql += "'";
                    valuesql += json_string_value(jf);
                    valuesql += "'";
                }
            }
            else if(json_is_integer(jf))
            {
                if (tagFieldAttribute::is_encrypt_field(name, std::string()))
                    valuesql += tagFieldAttribute::encrypt_valueex(std::to_string(json_integer_value(jf)));
                else
                    valuesql += std::to_string(json_integer_value(jf));
            }
        }

        valuesql += ")";
        columnsql += ")";
        columnsql += valuesql;
        return columnsql;
    }

    std::string json2insertsql(const std::string &jsonstr, const std::string& table_name)
    {
        json_error_t error;
        auto root = json_loads(jsonstr.c_str(), jsonstr.length(), &error);

        if (!root)
            return "";

        jsdetail::json_autoptr _auto(root);
        std::string sqlstat = "";

        if (json_is_array(root))
        {
            sqlstat = jsarray2sql(table_name, root);
        }
        else if(json_is_object(root))
        {
            sqlstat = jsobj2sql(table_name, root);
        }

        return snqu::codec::U2A(sqlstat);
    }*/
}

int SQDBManager::insert_json(const string& sql_json, const std::string& table_name, __int64* insert_auto_id)
{
    if (sql_json.empty())
        return SQERROR_OK;
    MDLOG(kTrace, "insert_json mk start");
    //auto sql = jsdetail::json2insertsql(sql_json, table_name);
    //MDLOG(kTrace, "insert_json mk end len:%d", sql.length());
    //return sql_execute(sql);
	return SQERROR_ERR;
}