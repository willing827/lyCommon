#define WIN32_LEAN_AND_MEAN
#include "sqmysql.h"
#include <direct.h>
#include <sqwin/win/sqwindows.h>
#include <sqdb/sqdberr.h>
#include <database/mysql/errmsg.h>
#include "../sqdblog.h"

SQMysql::SQMysql(void)
{
	m_mysql = nullptr;
}


SQMysql::~SQMysql(void)
{
	disconnect();
}

SQMysql& SQMysql::instance()
{
	static SQMysql _instance;
	return _instance;
}

int SQMysql::connect(const string& __in host, int __in port, const string& __in user_name, const string& __in password, const string& __in db_name)
{
	int error_code = SQERROR_DB_CONNECT_ERR;
	do{
		disconnect();	
		m_mysql = mysql_init(nullptr);
		if(!m_mysql)
		{
			MDLOG(kError, "mysql_init fail");
			break;
		}

		int ml_outtime = 10;
		mysql_options(m_mysql, MYSQL_OPT_CONNECT_TIMEOUT, &ml_outtime);

		int re = 0;
		my_bool reconnect = true;
		re = mysql_options(m_mysql, MYSQL_OPT_RECONNECT, &reconnect);
		if(re)
		{
			MDLOG(kError, "mysql_options fail");
			break;
		}
		if(!mysql_real_connect(m_mysql, host.c_str(), user_name.c_str(), password.c_str(), db_name.c_str(), port, NULL, 0))
		{
			MDLOG(kError, "mysql_real_connect failed mysql_errno code[%d] mysql_err message[%s]",
                mysql_errno(m_mysql), mysql_error(m_mysql));
            error_code =  mysql_errno(m_mysql);
			m_mysql = nullptr;
			break;
		}
		re = mysql_set_character_set(m_mysql, "utf8");
		if(re)
		{
			MDLOG(kError, "mysql_set_character_set fail");
			break;
		}
		re = mysql_select_db(m_mysql, db_name.c_str()); 
		if(re)
		{
			MDLOG(kError, "mysql_select_db fail");
			break;
		}
		error_code = SQERROR_OK;
	}while(0);	
	return error_code;
}

int SQMysql::disconnect()
{
	if(m_mysql)
	{
		mysql_close(m_mysql);		
		m_mysql = nullptr;
	}
	return 0;
}

int SQMysql::sql_execute(const string& __in query, __int64* __out insert_auto_id,bool is_convert_to_utf8)
{	
	int error_code = 0;
	string utf8_query = query;
	if(is_convert_to_utf8)  //转换成utf8格式，如果是图片文件则不转换
	{
		if(!utf8_check_string(utf8_query.c_str(), utf8_query.size()))
		{
			utf8_query = codec::A2U(utf8_query);
		}
	}
	
	if(m_mysql)
	{
		error_code = mysql_real_query(m_mysql, utf8_query.c_str(), utf8_query.size());  
		if(error_code)
		{
			int code = mysql_errno(m_mysql);
			MDLOG(kTrace, "sql_execute error, mysql_errno[%d], error_code[%d], sql_query[%s] mysql_error[%s]", code, 
                          error_code, utf8_to_asc(utf8_query).c_str(), mysql_error(m_mysql));
			error_code = code;
		}
		else if(error_code == 0 && insert_auto_id != nullptr)
		{
			*insert_auto_id = mysql_insert_id(m_mysql);
		}
	}
    else
        error_code = SQERROR_DB_ERR;
	return error_code;
}

int SQMysql::sql_query(const string& __in query, MYSQL_RES*& __out result)
{	
	int error_code = 0;
	string utf8_query = query;
	if(!utf8_check_string(utf8_query.c_str(), utf8_query.size()))
	{
		utf8_query = asc_to_utf8(utf8_query);
	}
	if(m_mysql)
	{
		error_code = mysql_real_query(m_mysql, utf8_query.c_str(), utf8_query.size()); 
		if(error_code == 0)
		{
			result = mysql_use_result(m_mysql);
			if(result == nullptr)
			{
                MDLOG(kWarning, "sql_query error ret data is empty, mysql_errno[%d] mysql_error[%s], sql_query[%s]",
                    mysql_errno(m_mysql), mysql_error(m_mysql), utf8_to_asc(utf8_query).c_str());
			}
		}
		else if(error_code)
		{
            auto code = mysql_errno(m_mysql);
            MDLOG(kWarning, "sql_query error, error_code[%d] mysql_errno[%d] mysql_error[%s], sql_query[%s]",
               error_code, code, mysql_error(m_mysql), utf8_to_asc(utf8_query).c_str());
			error_code = code;
		}
	}
	return error_code;
}

int SQMysql::get_result(MYSQL_RES*& __out result)
{
	if(m_mysql)
	{
		result = mysql_use_result(m_mysql);
		if(result == nullptr)
		{
			unsigned int err = mysql_errno(m_mysql);
            MDLOG(kWarning, "mysql_use_result error, mysql_errno[%d] mysql_error[%s]",
                 err, mysql_error(m_mysql));
            return SQERROR_DB_QUERY_ERR;
		}
	}
	return SQERROR_OK;
}

void SQMysql::free_result(MYSQL_RES* __in result)
{
	mysql_free_result(result);
}

int SQMysql::transaction_begin()
{
    int ret(SQERROR_DB_CONNECT_ERR);
	if(m_mysql)
	{
		string start_transaction = "START TRANSACTION;";
		ret = sql_execute(start_transaction.c_str(), nullptr, false);
	}
	return ret;
}

int SQMysql::transaction_commit()
{	
    int ret = SQERROR_DB_CONNECT_ERR;
	if(m_mysql)
	{
		if (mysql_commit(m_mysql))
        {
            ret = SQERROR_DB_OPER_ERR;
            MDLOG(kError, "mysql_commit failed err[%d] err_msg[%s]", mysql_errno(m_mysql), mysql_error(m_mysql));
        }
        else
        {
            ret = SQERROR_OK;
        }
	}
	return ret;
}

int SQMysql::transaction_rollback()
{	
    int ret = SQERROR_DB_CONNECT_ERR;
	if(m_mysql)
	{
		if (mysql_rollback(m_mysql))
        {
            ret = SQERROR_DB_OPER_ERR;
            MDLOG(kError, "mysql_rollback failed err[%d] err_msg[%s]", mysql_errno(m_mysql), mysql_error(m_mysql));
        }
        else
        {
            ret = SQERROR_OK;
        }
	}
	return ret;
}

int SQMysql::ping()
{
	int re = 1;
	if(m_mysql)
	{
		re = mysql_ping(m_mysql);
		if(re)
		{
			int code = mysql_errno(m_mysql);
			MDLOG(kError, "ping mysql mysql_errno[%d] mysql_err[%s]", code, mysql_error(m_mysql));
			if(code != CR_SERVER_GONE_ERROR)
			{
				MDLOG(kTrace, "ping set re with 0");
				re = 0;
			}
		}
	}
	return re;
}

int SQMysql::next_result()
{
	int re = 0;
	if(m_mysql)
	{
		re = mysql_next_result(m_mysql);
	}
	return re;
}

string SQMysql::wstr_to_str(const wstring& wstr, int codepage)
{
	if (wstr.empty())
	{
		return "";
	}
	string dest_str;
	int buffer_size = 0;	
	char* pchar = nullptr;
	buffer_size = WideCharToMultiByte(codepage, 0, wstr.c_str(), -1, 0, 0, nullptr, FALSE);
	pchar = new char[buffer_size + 1];
	memset(pchar, 0, sizeof(char) * (buffer_size + 1));
	if (WideCharToMultiByte(codepage, 0, wstr.c_str(), -1, (char*)pchar, buffer_size, nullptr, FALSE))
	{		
		dest_str = pchar;
	}
	delete pchar;	
	return dest_str;
}

wstring  SQMysql::str_to_wstr(const string& str, int codepage)
{
	if (str.empty())
	{
		return L"";
	}	
	wstring dest_str;
	int buffer_size = 0;	
	wchar_t* ptchar = nullptr;
	buffer_size = MultiByteToWideChar(codepage, 0, str.c_str(), -1, 0, 0);
	ptchar = new wchar_t[buffer_size + 1];
	memset(ptchar, 0, sizeof(wchar_t) * (buffer_size + 1));
	if (MultiByteToWideChar(codepage, 0, str.c_str(), -1, ptchar, buffer_size))
	{		
		dest_str = ptchar;
	}
	delete ptchar;	
	return dest_str;
}

string  SQMysql::asc_to_utf8(const string& str)
{
	return wstr_to_str(str_to_wstr(str, CP_ACP), CP_UTF8);
}

string  SQMysql::utf8_to_asc(const string& str)
{
	return wstr_to_str(str_to_wstr(str, CP_UTF8), CP_ACP);
}

int SQMysql::utf8_encode(int32_t codepoint, char *buffer, int *size)
{
	if(codepoint < 0)
		return -1;
	else if(codepoint < 0x80)
	{
		buffer[0] = (char)codepoint;
		*size = 1;
	}
	else if(codepoint < 0x800)
	{
		buffer[0] = 0xC0 + ((codepoint & 0x7C0) >> 6);
		buffer[1] = 0x80 + ((codepoint & 0x03F));
		*size = 2;
	}
	else if(codepoint < 0x10000)
	{
		buffer[0] = 0xE0 + ((codepoint & 0xF000) >> 12);
		buffer[1] = 0x80 + ((codepoint & 0x0FC0) >> 6);
		buffer[2] = 0x80 + ((codepoint & 0x003F));
		*size = 3;
	}
	else if(codepoint <= 0x10FFFF)
	{
		buffer[0] = 0xF0 + ((codepoint & 0x1C0000) >> 18);
		buffer[1] = 0x80 + ((codepoint & 0x03F000) >> 12);
		buffer[2] = 0x80 + ((codepoint & 0x000FC0) >> 6);
		buffer[3] = 0x80 + ((codepoint & 0x00003F));
		*size = 4;
	}
	else
		return -1;

	return 0;
}

int SQMysql::utf8_check_first(char byte)
{
	unsigned char u = (unsigned char)byte;

	if(u < 0x80)
		return 1;

	if(0x80 <= u && u <= 0xBF) {
		/* second, third or fourth byte of a multi-byte
		sequence, i.e. a "continuation byte" */
		return 0;
	}
	else if(u == 0xC0 || u == 0xC1) {
		/* overlong encoding of an ASCII byte */
		return 0;
	}
	else if(0xC2 <= u && u <= 0xDF) {
		/* 2-byte sequence */
		return 2;
	}

	else if(0xE0 <= u && u <= 0xEF) {
		/* 3-byte sequence */
		return 3;
	}
	else if(0xF0 <= u && u <= 0xF4) {
		/* 4-byte sequence */
		return 4;
	}
	else { /* u >= 0xF5 */
		/* Restricted (start of 4-, 5- or 6-byte sequence) or invalid
		UTF-8 */
		return 0;
	}
}

int SQMysql::utf8_check_full(const char *buffer, int size, int32_t *codepoint)
{
	int i;
	int32_t value = 0;
	unsigned char u = (unsigned char)buffer[0];

	if(size == 2)
	{
		value = u & 0x1F;
	}
	else if(size == 3)
	{
		value = u & 0xF;
	}
	else if(size == 4)
	{
		value = u & 0x7;
	}
	else
		return 0;

	for(i = 1; i < size; i++)
	{
		u = (unsigned char)buffer[i];

		if(u < 0x80 || u > 0xBF) {
			/* not a continuation byte */
			return 0;
		}

		value = (value << 6) + (u & 0x3F);
	}

	if(value > 0x10FFFF) {
		/* not in Unicode range */
		return 0;
	}

	else if(0xD800 <= value && value <= 0xDFFF) {
		/* invalid code point (UTF-16 surrogate halves) */
		return 0;
	}

	else if((size == 2 && value < 0x80) ||
		(size == 3 && value < 0x800) ||
		(size == 4 && value < 0x10000)) {
			/* overlong encoding */
			return 0;
	}

	if(codepoint)
		*codepoint = value;

	return 1;
}

const char *SQMysql::utf8_iterate(const char *buffer, int32_t *codepoint)
{
	int count;
	int32_t value;

	if(!*buffer)
		return buffer;

	count = utf8_check_first(buffer[0]);
	if(count <= 0)
		return NULL;

	if(count == 1)
		value = (unsigned char)buffer[0];
	else
	{
		if(!utf8_check_full(buffer, count, &value))
			return NULL;
	}

	if(codepoint)
		*codepoint = value;

	return buffer + count;
}

int SQMysql::utf8_check_string(const char *string, int length)
{
	int i;

	if(length == -1)
		length = strlen(string);

	for(i = 0; i < length; i++)
	{
		int count = utf8_check_first(string[i]);
		if(count == 0)
			return 0;
		else if(count > 1)
		{
			if(i + count > length)
				return 0;

			if(!utf8_check_full(&string[i], count, NULL))
				return 0;

			i += count - 1;
		}
	}

	return 1;
}
