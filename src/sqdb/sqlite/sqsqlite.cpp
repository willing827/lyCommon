#include "sqsqlite.h"
#include <database/sqlite3/sqlite3.h>
#include "../sqdblog.h"
#include "sqlitestmt.h"
#include <sqdb/sqdbuti.h>

namespace snqu{ namespace db{

#define BUILD_DB_ERR ClearErr(); m_impl->build_err(m_err, m_err_msg)

    struct SQLite3::impl 
    {
        sqlite3* m_db;
        std::vector<std::string> m_db_conn_param;
        size_t m_thread_hash;

        impl()
            : m_db(nullptr)
        {
        }

        ~impl()
        {
        }

		void build_err(int& err, std::string& err_msg)
		{
			err = sqlite3_errcode(m_db);
			err_msg = sqlite3_errmsg(m_db);
		}
    };


    SQLite3::SQLite3(void)
        : m_impl(new impl)
    {
        m_impl->m_db = nullptr;
    }

    SQLite3::~SQLite3(void)
    {
        Disconnect();
    }

    int SQLite3::Connect(const std::vector<std::string>& conn_param)
    {
        int error_code = ERR_DB_CONN;

        if (conn_param.size() < DBConn_Sqlite3_size)
            return ERR_DB_PARAM;

        if (sqlite3_open(conn_param[db::Conn_Sqlite_Path].c_str(), &m_impl->m_db))
        {
			BUILD_DB_ERR;
			MDLOG(kError, "sqlite3_open fail err[{0}]", error_code);
            return ERR_DB_CONN;
        }

        sqlite3_exec(m_impl->m_db, "PRAGMA synchronous = OFF", 0, 0, 0);		//提高性能
        sqlite3_exec(m_impl->m_db, "PRAGMA cache_size = 8000", 0, 0, 0);		//加大缓存
        sqlite3_exec(m_impl->m_db, "PRAGMA count_changes = 1", 0, 0, 0);		//返回改变记录数
        sqlite3_exec(m_impl->m_db, "PRAGMA case_sensitive_like = 1", 0, 0, 0);  //支持中文LIKE查询

        if (!conn_param[db::Conn_Sqlite_Aeskey].empty())
        {
			if (sqlite3_key(m_impl->m_db, conn_param[db::Conn_Sqlite_Aeskey].c_str(),
				conn_param[db::Conn_Sqlite_Aeskey].length()))
            {
				BUILD_DB_ERR;
                MDLOG(kError, "sqlite3_key fail sql_err[{0}]", m_err);
                return ERR_DB_ENCYPT;
            }
        }
        return ERR_OK;
    }

    void SQLite3::Disconnect()
    {
        if (nullptr != m_impl->m_db)
        {
            sqlite3_close(m_impl->m_db);
            m_impl->m_db = nullptr;
        }
    }

    int SQLite3::Execute(const std::string& sql_stmt, __int64* last_insert_id)
    {	
        int error_code = ERR_DB_CONN;

        if(NULL != m_impl->m_db)
        {
            char *errmsg;
            if(sqlite3_exec(m_impl->m_db, sql_stmt.c_str(), 0, 0, &errmsg))
            {
				BUILD_DB_ERR;
				error_code = ERR_DB_EXEC;
				MDLOG(kError, "sql_execute error, sql_err[{0}], sql_stmt[{1}] sql_error[{2}]",
					m_err, sql_stmt.c_str(), m_err_msg.c_str());
            }
            else
            {
				if (last_insert_id != nullptr)
					*last_insert_id = sqlite3_last_insert_rowid(m_impl->m_db);
				error_code = ERR_OK;
            }
        }

        return error_code;
    }

    int SQLite3::Query(const std::string& sql_stmt, DBDataSet& results)
    {	
        int error_code = ERR_DB_CONN;

        if(NULL == m_impl->m_db) return error_code;

        SQLiteStatement_ptr stmt(new SQLiteStatement(sql_stmt, m_impl->m_db));
        if (!stmt->IsOk())
        {
            return ERR_DB_EXEC;
        }

        while (stmt->NextRow())
        {
            int num_fields = stmt->ColumnCount();
            using namespace rapidjson;
            results.AddRow();
            for (int i = 0; i < num_fields; i++)
            {
                switch (stmt->DataType(i))
                {
                case SQLiteStatement::INT:
                    results.SetVal(stmt->ColumnName(i).c_str(), stmt->ValueInt64(i));
                    break;
                case SQLiteStatement::FLT:
                    results.SetVal(stmt->ColumnName(i).c_str(), stmt->ValueDouble(i));
                    break;
                case SQLiteStatement::BLB:
                {
                    auto ret = stmt->ValueBlob(i);
                    results.SetVal(stmt->ColumnName(i).c_str(), ret.c_str(), ret.size());
                    break;
                }
                case SQLiteStatement::TXT:
                {
                    auto ret = stmt->ValueString(i);
                    results.SetVal(stmt->ColumnName(i).c_str(), ret.c_str(), ret.size());
                    break;
                }
                case SQLiteStatement::NUL:
                    results.SetVal(stmt->ColumnName(i).c_str(), "", 0);
                    break;
                default:
                    MDLOG(kError, "build_row_property unknown data type, type:[{0}]", stmt->DataType(i));
                    break;
                }
            }
        }

        return 0;
    }

    int SQLite3::TransactionBegin()
    {
		if (Execute("begin"))
		{
			BUILD_DB_ERR;
			return ERR_DB_TRANS;
		}

		return ERR_OK;
    }

    int SQLite3::TransactionCommit()
    {	
		if (Execute("commit"))
		{
			BUILD_DB_ERR;
			return ERR_DB_TRANS;
		}

		return ERR_OK;
    }

    int SQLite3::TransactionRollback()
    {
		if (Execute("rollback"))
		{
			BUILD_DB_ERR;
			return ERR_DB_TRANS;
		}

        return ERR_OK;
    }
}}