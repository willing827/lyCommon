
#include <sqdb/sqdb.h>
#include "mysql/sqmysql.h"
#include <sqstd/sqstringhelper.h>
#include <sstream>
#include <codec/sqcodec.h>

SQDB::SQDB(void)
{
	m_connect_str = "";
	m_mysql = new SQMysql();
}


SQDB::~SQDB(void)
{
	close();
	if(m_mysql)
	{
		delete m_mysql;
		m_mysql = nullptr;
	}
}

SQDB& SQDB::instance()
{
	static SQDB _instance;
	return _instance;
}

int SQDB::open(const string& __in host, int __in port, const string& __in user_name, 
               const string& __in password, const string& __in db_name)
{
	int error_code = 0;
	string connect_str = host + std::to_string(port) + user_name + password + db_name;
	close();
	m_connect_str = connect_str;
	error_code = m_mysql->connect(host, port, user_name, password, db_name);
	m_db_name = db_name;
	return error_code;
}

bool SQDB::keep_alive()
{
	bool alive = false;
	int ping_status = 1;
	if(m_mysql)
	{
		ping_status = m_mysql->ping();
	}
	if(ping_status == 0)
	{
		alive = true;
	}
	return alive;
}

int SQDB::close()
{
	if(!m_connect_str.empty())
	{
		m_connect_str = "";
		m_mysql->disconnect();
	}
	return 0;
}


int SQDB::sql_execute(const string& __in sql_statement, __int64* __out insert_auto_id)
{
	int error_code = 0;
	error_code = m_mysql->sql_execute(sql_statement, insert_auto_id);
	return error_code;
}

int SQDB::sql_query(const string& __in sql_statement, IDataSet& __out result)
{
	int error_code = 0;
	MYSQL_RES* sql_result = nullptr;
	error_code = m_mysql->sql_query(sql_statement, sql_result);
	if(sql_result)
	{
		vector<vector<FieldAttribute> > attribute_result;
		parse_sqlresult(sql_result, attribute_result);
		for(size_t i = 0; i < attribute_result.size(); ++i)
		{
			auto item = result.new_item();
			SQDBUti::parse_attribute(attribute_result[i], *item);
			result.add_item(item);
		}
		m_mysql->free_result(sql_result);
		int next_result = 0;
		do{
			next_result = m_mysql->next_result();
			//fprintf(stdout, "next_result:%d\n", next_result);
		}while(next_result == 0);
	}
	return error_code;
}

int SQDB::sql_query(const string& __in sql_statement,  vector<vector<FieldAttribute> >& __out result)
{
	int error_code = 0;
	MYSQL_RES* sql_result = nullptr;
	error_code = m_mysql->sql_query(sql_statement, sql_result);
	if(sql_result)
	{
		parse_sqlresult(sql_result, result);
		m_mysql->free_result(sql_result);
		int next_result = 0;
		do{
			next_result = m_mysql->next_result();		
		}while(next_result == 0);
	}
	return error_code;
}

int SQDB::insert_items(const string& __in table_name, const IDataSet& __in result, __int64* __out insert_auto_id)
{
	int error_code = 0;
    result.foreach_item([&](const proto_ptr iter){
            vector<FieldAttribute> field_attributes;
            SQDBUti::parse_message((*iter), field_attributes);
            string sql_field_name;
            string sql_field_value;			
            for(size_t i = 0; i < field_attributes.size(); ++i)
            {
                if(field_attributes[i].name().empty() || field_attributes[i].sql_value().empty() || "mask" == field_attributes[i].name())
                {
                    continue;
                }
                sql_field_name += field_attributes[i].name();
                sql_field_value += field_attributes[i].encrypt_sql_value();	
                sql_field_name += ",";
                sql_field_value += ",";
            }
            if(!sql_field_name.empty() && !sql_field_value.empty())
            {
                sql_field_name.erase(sql_field_name.size() - 1);
                sql_field_value.erase(sql_field_value.size() - 1);
            }
            string sql_statement =  "insert into " + table_name + " (" + sql_field_name + ") " + " values (" + sql_field_value + ");";
            error_code = sql_execute(sql_statement, insert_auto_id);
    });

	return error_code;
}

int SQDB::insert_items(const string& __in table_name, const vector<FieldAttribute> & __in insert_data, __int64* __out insert_auto_id)
{
	int error_code = 0;
	error_code = into_item("insert", table_name, insert_data, insert_auto_id);
	return error_code;
}

int SQDB::into_item(const string& __in insert_name, const string& __in table_name, const vector<FieldAttribute> & __in insert_data, __int64* __out insert_auto_id)
{
	int error_code = 0;
	string sql_field_name;
	string sql_field_value;			
	for(size_t i = 0; i < insert_data.size(); ++i)
	{
		if(insert_data[i].name().empty() || insert_data[i].sql_value().empty() || "mask" == insert_data[i].name())
		{
			continue;
		}
		sql_field_name += insert_data[i].name();
		sql_field_value += insert_data[i].encrypt_sql_value();
		sql_field_name += ",";
		sql_field_value += ",";
	}
	if(!sql_field_name.empty() && !sql_field_value.empty())
	{
		sql_field_name.erase(sql_field_name.size() - 1);
		sql_field_value.erase(sql_field_value.size() - 1);
	}
	string sql_statement =  insert_name + " into " + table_name + " (" + sql_field_name + ") " + " values (" + sql_field_value + ");";

	string convsql = snqu::codec::U2A(sql_statement);
	error_code = sql_execute(sql_statement, insert_auto_id);
	return error_code;
}

int SQDB::insert_items(const string& __in table_name, const vector<vector<FieldAttribute> >& __in insert_datas)
{
	int error_code = 0;
	for(auto iter = insert_datas.begin(); iter != insert_datas.end(); ++iter)
	{
		error_code = insert_items(table_name, *iter);
		if (error_code)
		{
			break;
		}
	}
	return error_code;
}

int SQDB::insert_items(const string& __in table_name, const protobuf::Message& __in insert_data, __int64* __out insert_auto_id)
{
	int error_code = 0;
	vector<FieldAttribute> field_attributes;
	SQDBUti::parse_message(insert_data, field_attributes);
	error_code = insert_items(table_name, field_attributes, insert_auto_id);
	return error_code;
}

int SQDB::get_items(const string& __in table_name, IDataSet& __out result)
{
	return get_items(table_name, "", "", result);
}

int SQDB::get_items(const string& __in table_name, const string& __in condition, const string& __in tail_condition, IDataSet& __out result)
{
	int error_code = 0;
	auto item = result.new_item();
	vector<FieldAttribute> field_attributes;
	SQDBUti::parse_message(*item, field_attributes);
	string sql_field_name;		
	for(size_t i = 0; i < field_attributes.size(); ++i)
	{
		if(field_attributes[i].name().empty() || "mask" == field_attributes[i].name())
		{
			continue;
		}
		sql_field_name += field_attributes[i].name();
		sql_field_name += ",";			
	}
	if(!sql_field_name.empty())
	{
		sql_field_name.erase(sql_field_name.size() - 1);
	}
	string sql_statement =  "select " +  sql_field_name + " from " + table_name + " " + condition + " " + tail_condition + ";";
	error_code = sql_query(sql_statement, result);
	return error_code;
}

int SQDB::get_items(const string& __in table_name, const vector<FieldAttribute>* __in select_fields, const vector<WhereField>* __in where_fields, const string& __in limit_condition, vector<vector<FieldAttribute> >& __out result)
{
	int error_code = 0;
	string select_field_name;
	string where_field_value;
	if(select_fields && !select_fields->empty())
	{
		for(size_t i = 0; i < select_fields->size(); ++i)
		{
			if((*select_fields)[i].name().empty() || "mask" == (*select_fields)[i].name())
			{
				continue;
			}
			select_field_name += (*select_fields)[i].name();
			select_field_name += ",";	
		}
		if(!select_field_name.empty())
		{
			select_field_name.erase(select_field_name.size() - 1);
		}
	}
	if(select_field_name.empty())
	{
		select_field_name = " * ";
	}
	parse_wherefield(where_fields, where_field_value);
	string sql_statement =  "select " +  select_field_name + " from " + table_name + where_field_value + " " + limit_condition + ";";
	error_code = sql_query(sql_statement, result);
	return error_code;
}
int SQDB::get_items(const string& __in table_name, const vector<FieldAttribute>* __in select_fields, SqlStream& __in where_fields, const string& __in limit_condition, vector<vector<FieldAttribute> >& __out result)
{
	int error_code = 0;
	string select_field_name;
	string where_field_value;
	if(select_fields && !select_fields->empty())
	{
		for(size_t i = 0; i < select_fields->size(); ++i)
		{
			if((*select_fields)[i].name().empty() || "mask" == (*select_fields)[i].name())
			{
				continue;
			}
			select_field_name += (*select_fields)[i].name();
			select_field_name += ",";	
		}
		if(!select_field_name.empty())
		{
			select_field_name.erase(select_field_name.size() - 1);
		}
	}
	if(select_field_name.empty())
	{
		select_field_name = " * ";
	}
	string sql_statement =  "select " +  select_field_name + " from " + table_name + " " +where_fields.str() + " " + limit_condition + ";";
	error_code = sql_query(sql_statement, result);
	return error_code;
}
int SQDB::update_items(const string& __in table_name, const vector<WhereField>* __in where_fields, const vector<FieldAttribute> & __in update_data)
{
    int error_code = 0;
    do
    {
        if(update_data.empty())
        {	
            break;
        }
        string update_field_value;
        string where_field_value;
        string auto_increment_name;
        get_auto_incement_primary_key_name(table_name, auto_increment_name);
        if(where_fields)
        {
            parse_wherefield(where_fields, where_field_value);
            for(size_t i = 0; i < update_data.size(); ++i)
            {
                if(!update_data[i].has_value()
                    || update_data[i].name().empty()
					|| update_data[i].sql_value().empty()
                    || (update_data[i].name() == auto_increment_name && update_data[i].sql_value() == "0"))
                {
                    continue;
                }
                update_field_value += update_data[i].name() + "=" + update_data[i].encrypt_sql_value();
                update_field_value += ",";							
            }
            if(!update_field_value.empty())
            {
                update_field_value.erase(update_field_value.size() - 1);
            }
        }
        else
        {
            vector<WhereField> temp_where_fields;
            error_code = get_where_field(table_name, update_data, temp_where_fields);
            if(error_code)
            {
                break;
            }			
            for(size_t i = 0; i < update_data.size(); ++i)
            {
                if(!update_data[i].has_value()
                    || update_data[i].name().empty()
                    || (update_data[i].sql_value().empty())
                    || (update_data[i].name() == auto_increment_name && update_data[i].sql_value() == "0"))
                {
                    continue;
                }
                bool is_where = false;
                for(size_t j = 0; j < temp_where_fields.size(); ++j)
                {
                    if(temp_where_fields[j].field.name() == update_data[i].name())
                    {
                        is_where = true;
                        break;
                    }
                }
                if(is_where)
                {
                    continue;
                }
                update_field_value += update_data[i].name() + "=" + update_data[i].encrypt_sql_value();
                update_field_value += ",";
            }
            if(!update_field_value.empty())
            {
                update_field_value.erase(update_field_value.size() - 1);
            }
            parse_wherefield(&temp_where_fields, where_field_value);
        }
        string sql_statement =  "update " +  table_name + " set " + update_field_value + where_field_value + ";";
        if (snqu::codec::IsUTF8(sql_statement))
            sql_statement = snqu::codec::U2A(sql_statement);

        error_code = sql_execute(sql_statement,nullptr);
    }while(0);
    return error_code;
}

int SQDB::update_items(const string& __in table_name, const vector<WhereField>* __in where_fields, const protobuf::Message& __in update_data)
{
	int error_code = 0;
	vector<FieldAttribute> field_attributes;
	SQDBUti::parse_message(update_data, field_attributes);
	error_code = update_items(table_name, where_fields, field_attributes);
	return error_code;
}

int SQDB::replace_items(const string& __in table_name, const vector<WhereField>* __in where_fields, const vector<FieldAttribute> & __in replace_data)
{
	int error_code = 0;
	do
	{
		if(replace_data.empty())
		{	
			break;
		}
		if(where_fields)
		{
			bool is_exist = record_exist(table_name, where_fields);
			if(is_exist)
			{
				error_code = delete_items(table_name, where_fields);
				if(error_code)
				{
					break;
				}
			}
			error_code = insert_items(table_name, replace_data);
		}
		else
		{
			vector<PrimaryKey> primary_keys;
			error_code = get_primary_key(m_db_name, table_name, primary_keys);
			if(error_code)
			{
				break;
			}
			if(primary_keys.size() == 1)
			{
				error_code = into_item("replace", table_name, replace_data);
			}
			else
			{
				vector<WhereField> temp_where_fields;
				error_code = get_where_field(table_name, replace_data, temp_where_fields);
				if(error_code)
				{
					break;
				}
				if(!temp_where_fields.empty())
				{
					bool is_exist = record_exist(table_name, &temp_where_fields);
					if(is_exist)
					{
						error_code = delete_items(table_name, &temp_where_fields);
						if(error_code)
						{
							break;
						}
					}
				}
				error_code = insert_items(table_name, replace_data);
			}
		}
	}while(0);
	return error_code;
}

int SQDB::replace_items(const string& __in table_name, const vector<WhereField>* __in where_fields, const protobuf::Message& __in replace_data)
{
	int error_code = 0;
	vector<FieldAttribute> field_attributes;
	SQDBUti::parse_message(replace_data, field_attributes);
	error_code = replace_items(table_name, where_fields, field_attributes);
	return error_code;
}

int SQDB::copy_items(const string& __in table_name, const vector<WhereField>* __in where_fields, const vector<FieldAttribute> & __in copy_data)
{
	int error_code = 0;
	do
	{
		if(copy_data.empty())
		{	
			break;
		}
		if(where_fields)
		{
			bool is_exist = record_exist(table_name, where_fields);
			if(is_exist)
			{
				error_code = update_items(table_name, where_fields, copy_data);
			}
			else
			{
				error_code = insert_items(table_name, copy_data);
			}
		}
		else
		{
			vector<WhereField> temp_where_fields;
			error_code = get_where_field(table_name, copy_data, temp_where_fields);
			if(error_code)
			{
				break;
			}
			if(!temp_where_fields.empty())
			{
				bool is_exist = record_exist(table_name, &temp_where_fields);
				if(is_exist)
				{
					error_code = update_items(table_name, nullptr, copy_data);
				}
				else
				{
					error_code = insert_items(table_name, copy_data);
				}
			}
			else
			{
				error_code = insert_items(table_name, copy_data);
			}
		}
	}while(0);
	return error_code;
}

int SQDB::copy_items(const string& __in table_name, const vector<WhereField>* __in where_fields, const protobuf::Message& __in copy_data)
{
	int error_code = 0;
	vector<FieldAttribute> field_attributes;
	SQDBUti::parse_message(copy_data, field_attributes, false, true);
	error_code = copy_items(table_name, where_fields, field_attributes);
	return error_code;
}

int SQDB::delete_items(const string& __in table_name, const vector<WhereField>* __in where_fields)
{
	int error_code = 0;
	string where_field_value;
	parse_wherefield(where_fields, where_field_value);
	string sql_statement =  "delete from " +  table_name + where_field_value + ";";
	error_code = sql_execute(sql_statement);
	return error_code;
}

bool SQDB::record_exist(const string& __in table_name, const vector<WhereField>* __in where_fields)
{
	bool is_exist = false;
	int error_code = 0;
	FieldAttribute select_field;
	vector<FieldAttribute> select_fields;
	vector<vector<FieldAttribute> > result;

	select_field.name("count(*)");
	select_fields.push_back(select_field);
	error_code = get_items(table_name, &select_fields, where_fields, " limit 1", result);
	if(error_code == 0 && !result.empty() && !result[0].empty())
	{
        if (atoi(result[0][0].value().c_str()) > 0)
		    is_exist = true;
	}	
	return is_exist;
}

bool SQDB::record_exist(const string& __in table_name,SqlStream& __in where_fields)
{
	bool is_exist = false;
	int error_code = 0;
	FieldAttribute select_field;
	vector<FieldAttribute> select_fields;
	vector<vector<FieldAttribute> > result;

	select_field.name("count(*)");
	select_fields.push_back(select_field);
	error_code = get_items(table_name, &select_fields, where_fields, " limit 1", result);
	if(error_code == 0 && !result.empty() && !result[0].empty())
	{
		if (atoi(result[0][0].value().c_str()) > 0)
			is_exist = true;
	}	
	return is_exist;
}

int SQDB::transaction_begin()
{
	return m_mysql->transaction_begin();
}

int SQDB::transaction_commit()
{
	return m_mysql->transaction_commit();
}

int SQDB::transaction_rollback()
{
	return m_mysql->transaction_rollback();
}

/*
* 2015-11-26 22:09:47 wenjunbo
* 修改说明：对mysql无符号整形类型字段添加对应的C++数据类型映射
*/
int SQDB::parse_sqlresult(MYSQL_RES* __in sql_result, vector<vector<FieldAttribute> >& __out results)
{
	if (sql_result)
	{
		unsigned int num_fields = mysql_num_fields(sql_result);
		MYSQL_FIELD *fields = mysql_fetch_fields(sql_result);	
		MYSQL_ROW row;		
        std::string temp_str;
		while ((row = mysql_fetch_row(sql_result)))
		{
            vector<FieldAttribute> one_record_attribute;
            unsigned long *lengths = mysql_fetch_lengths(sql_result);
			for (unsigned int i = 0; i < num_fields; ++i)
			{
                FieldAttribute	field_attribute;
				if(strlen(fields[i].name) > 0)
				{
					if(fields[i].name[0] == '@')
					{
						field_attribute.name(fields[i].name + 1);
					}
					else
					{
						field_attribute.name(fields[i].name);
					}
				}
				int field_type = 0;
				switch (fields[i].type)
				{
                case MYSQL_TYPE_TINY:
				case MYSQL_TYPE_SHORT:
				case MYSQL_TYPE_LONG:

					if(fields[i].flags & UNSIGNED_FLAG) {
						field_type = protobuf::FieldDescriptor::
							CppType::CPPTYPE_UINT32;
					} else {
						field_type = protobuf::FieldDescriptor::
							CppType::CPPTYPE_INT32;
					}
					break;

				case MYSQL_TYPE_LONGLONG:

					if(fields[i].flags & UNSIGNED_FLAG) {
						field_type = protobuf::FieldDescriptor::
							CppType::CPPTYPE_UINT64;
					} else {
						field_type = protobuf::FieldDescriptor::
							CppType::CPPTYPE_INT64;					
					}
					break;

                case MYSQL_TYPE_LONG_BLOB:
				case MYSQL_TYPE_BLOB:
				case MYSQL_TYPE_VAR_STRING:
					field_type = protobuf::FieldDescriptor::CppType::CPPTYPE_STRING;
					break;
				default:
					fprintf(stderr, " parse_sqlresult do not data type, type:%d\n", fields[i].type);
					break;
				}

                if(row[i] != NULL && (lengths[i] > 0))
                    temp_str.assign(row[i], lengths[i]);
                else
                    temp_str = "";

				field_attribute.value_decrypt(temp_str, field_type,true,true);
				one_record_attribute.emplace_back(field_attribute);
			}
			results.emplace_back(one_record_attribute);
		}
	}
	return 0;
}

int SQDB::get_primary_key(const string& __in db_name, const string& __in table_name, vector<PrimaryKey>& __out primary_keys)
{
	int error_code = 1;
	do
	{
		if(db_name.empty())
		{
			break;
		}
		if(table_name.empty())
		{
			break;
		}
		//select column_name, data_type, column_key from information_schema.columns where table_schema='nbtest' and table_name='t_vip_user' and column_key='PRI';
		stringstream sql_statement;
		sql_statement << "select column_name, data_type, extra, column_key from information_schema.columns where table_schema='" << db_name << "' and table_name='" << table_name << "' and column_key='PRI';";
		vector<vector<FieldAttribute> > result;
		int error_code = sql_query(sql_statement.str(), result);
		if(error_code)
		{
			break;
		}
		map<string, int> data_types;
		data_types["int"] = protobuf::FieldDescriptor::CppType::CPPTYPE_INT32;
		data_types["varchar"] = protobuf::FieldDescriptor::CppType::CPPTYPE_STRING;	
		for (size_t i = 0; i != result.size(); ++i)
		{
			PrimaryKey primary_key;
			for(size_t j = 0; j != result[i].size(); ++j)
			{
				string name  = result[i][j].name();
				string value = result[i][j].value();
				if(name == "column_name")
				{
					primary_key.name = value;
				}
				else if(name == "data_type")
				{
					primary_key.type = data_types[value];
				}
				else if(name == "extra")
				{
					if(value == "auto_increment")
					{
						primary_key.auto_increment = true;
					}
					else
					{
						primary_key.auto_increment = false;
					}
				}
			}
			primary_keys.push_back(primary_key);
		}
		error_code = 0;
	}while(0);
	return error_code;
}

int SQDB::get_auto_incement_primary_key_name(const string& __in table_name, string& __out primary_key_name)
{
	int error_code = 0;
	do
	{
		vector<PrimaryKey> primary_keys;
		error_code = get_primary_key(m_db_name, table_name, primary_keys);
		if(error_code)
		{
			break;
		}
		for(size_t i = 0; i < primary_keys.size(); ++i)
		{
			if(primary_keys[i].auto_increment)
			{
				primary_key_name = primary_keys[i].name;
				break;
			}
		}
	}while(0);
	return error_code;
}

int SQDB::parse_wherefield(const vector<WhereField>* __in where_fields, string& __out where_field_value)
{
	int error_code = 0;
	if(where_fields)
	{
		for(size_t i = 0; i < where_fields->size(); ++i)
		{
			if((*where_fields)[i].compare.empty() || (*where_fields)[i].field.name().empty() || (*where_fields)[i].field.sql_value().empty())
			{
				continue;
			}
			if(!(*where_fields)[i].left_bracket.empty())
			{
				where_field_value += (*where_fields)[i].left_bracket + " ";
			}
			if(!(*where_fields)[i].compare.empty())
			{
                where_field_value += (*where_fields)[i].field.name() + (*where_fields)[i].compare + (*where_fields)[i].field.encrypt_sql_value();
			}
			else
			{
				where_field_value += (*where_fields)[i].field.name();
			}
			if(!(*where_fields)[i].right_bracket.empty())
			{
				where_field_value += " " + (*where_fields)[i].right_bracket;
			}
			if((*where_fields)[i].right_link.empty())
			{
				where_field_value += " and ";
			}
			else
			{
				where_field_value += " " + (*where_fields)[i].right_link + " ";
			}
		}
		if(!where_field_value.empty())
		{
			where_field_value.erase(where_field_value.size() - strlen(" and "));
			where_field_value = " where " + where_field_value;
		}
	}	
	return error_code;
}

int SQDB::get_where_field(const string& __in table_name, const vector<FieldAttribute>& __in update_data, vector<WhereField>& __out where_fields)
{
	int error_code = 0;
	do
	{
		if(update_data.empty())
		{	
			break;
		}
		vector<PrimaryKey> primary_keys;
		error_code = get_primary_key(m_db_name, table_name, primary_keys);
		if(error_code)
		{
			break;
		}
		for(size_t i = 0; i < update_data.size(); ++i)
		{
			if(update_data[i].name().empty() || update_data[i].sql_value().empty())
			{
				continue;
			}
			for(size_t j = 0; j < primary_keys.size(); ++j)
			{
				if(primary_keys[j].name != update_data[i].name())
				{
					continue;
				}
				if(!primary_keys[j].auto_increment || update_data[i].sql_value() != "0")
				{
					WhereField where_field;
					where_field.compare = "=";
					where_field.field = update_data[i];
					where_fields.push_back(where_field);
				}
				break;
			}
		}
	}while(0);
	return error_code;
}

void SQDB::set_owner_id(std::thread::id id)
{
    m_thread_id = id;
}

std::thread::id& SQDB::get_owner_id()
{
    return m_thread_id;
}

int SQDB::convert_binary_string(std::string& in_data)
{
    char* temp = new char[2*in_data.length()+2];

    if (0 != mysql_real_escape_string(m_mysql->m_mysql, temp, in_data.c_str(), in_data.length()))
    {
        in_data.assign(temp);
    }
    delete [] temp;
    return 0;
}
