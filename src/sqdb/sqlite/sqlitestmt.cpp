#include "sqlitestmt.h"
#include <database/sqlite3/sqlite3.h>
#include "../sqdblog.h"

namespace snqu{ namespace db{

// how to use
//SQLiteStatement_ptr stmt(new SQLiteStatement(statement, sqlite3*));

SQLiteStatement::SQLiteStatement(std::string const& statement, sqlite3* db_ptr) 
{
    auto rc = sqlite3_prepare(db_ptr, statement.c_str(), -1/* If than zero, then stmt is read up to the first nul terminator*/,
                              &m_stmt, 0/*Pointer to unused portion of stmt*/);         
    if (SQLITE_OK != rc)
    {
        MDLOG(kError, "sqlite3_prepare fail err[%d]", rc);
    }
}

SQLiteStatement::~SQLiteStatement() 
{
    if(m_stmt) 
    {
        sqlite3_finalize(m_stmt);
        m_stmt = nullptr;
    }
}

SQLiteStatement::SQLiteStatement()
    : m_stmt(nullptr)
{
}

bool SQLiteStatement::Bind(int pos_zero_indexed, std::string const& value) 
{
    if (sqlite3_bind_text(m_stmt, pos_zero_indexed+1, value.c_str(), value.length(),
                          SQLITE_TRANSIENT)) // SQLITE_TRANSIENT: SQLite makes its own copy
    {
        return false;
    }
    return true;
}

bool SQLiteStatement::Bind(int pos_zero_indexed, double value) 
{
    if (sqlite3_bind_double(m_stmt, pos_zero_indexed+1, value))
    {   
        return false;
    }
    return true;
}

bool SQLiteStatement::Bind(int pos_zero_indexed, int value) 
{
    if (SQLITE_OK != sqlite3_bind_int(m_stmt, pos_zero_indexed+1, value))
    {
            return false;
    }
    return true;
}

bool SQLiteStatement::BindNull(int pos_zero_indexed) 
{
    if (SQLITE_OK != sqlite3_bind_null(m_stmt, pos_zero_indexed+1))
    {  
        return false;
    }
    return true;
}

bool SQLiteStatement::IsOk()
{
    return nullptr != m_stmt;
}

bool SQLiteStatement::Execute() 
{
    int rc = sqlite3_step(m_stmt);

    if (SQLITE_BUSY == rc || SQLITE_ERROR == rc ||
        SQLITE_MISUSE == rc)
    {
        MDLOG(kError, "sqlite3_step fail err[%d]", rc);
        return false;
    }

    if (SQLITE_DONE != rc)
    {
        return false;
    }

    sqlite3_reset(m_stmt);
    return true;
}

SQLiteStatement::dataType SQLiteStatement::DataType(int pos_zero_indexed) 
{
    return dataType(sqlite3_column_type(m_stmt, pos_zero_indexed));
}

int SQLiteStatement::ValueInt(int pos_zero_indexed) 
{
    return sqlite3_column_int(m_stmt, pos_zero_indexed);
}

__int64 SQLiteStatement::ValueInt64(int pos_zero_indexed)
{
    return (__int64)sqlite3_column_int64(m_stmt, pos_zero_indexed);
}

std::string SQLiteStatement::ValueString(int pos_zero_indexed) 
{
    auto ret = sqlite3_column_text(m_stmt, pos_zero_indexed);
    if (ret)
        return std::string(reinterpret_cast<const char*>(ret));
    else
        return "";
}
std::string   SQLiteStatement::ValueBlob(int pos_zero_indexed)
{
    auto ret = sqlite3_column_blob(m_stmt, pos_zero_indexed);
    auto size = sqlite3_column_bytes(m_stmt, pos_zero_indexed);
    if (ret)
        return std::string(reinterpret_cast<const char*>(ret, size));
    else
        return "";
}

double SQLiteStatement::ValueDouble(int pos_zero_indexed)
{
    return sqlite3_column_double(m_stmt, pos_zero_indexed);
}

std::string SQLiteStatement::ColumnName(int pos_zero_indexed)
{
    return std::string(reinterpret_cast<const char*>(sqlite3_column_name(m_stmt, pos_zero_indexed)));
}

bool SQLiteStatement::RestartSelect() 
{
    sqlite3_reset(m_stmt);
    return true;
}

bool SQLiteStatement::Reset() 
{
    int rc = sqlite3_step(m_stmt);

    sqlite3_reset(m_stmt);

    if (rc == SQLITE_ROW) return true;
    return false;
}

bool SQLiteStatement::NextRow() 
{
    int rc = sqlite3_step(m_stmt);

    if (rc == SQLITE_ROW) 
    {
        return true;
    }
    if (rc == SQLITE_DONE) 
    {
        sqlite3_reset(m_stmt);
        return false;
    } 
    else if (SQLITE_BUSY == rc || SQLITE_ERROR == rc ||
        SQLITE_MISUSE == rc)
    {
        MDLOG(kError, "sqlite3_step fail err[%d]", rc);
    } 

    return false;
}

int SQLiteStatement::ColumnCount()
{
    return sqlite3_data_count(m_stmt);
}

}}