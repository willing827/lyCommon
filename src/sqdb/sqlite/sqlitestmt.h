/*---------------------------------------------------------------------------*/
/*  sqlitestmt.h                                                             */
/*                                                                           */
/*  History                                                                  */
/*      05/22/2017 create                                                    */
/*                                                                           */
/*  Author                                                                   */
/*       feng hao                                                            */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*---------------------------------------------------------------------------*/
#ifndef SQ_SQLITE_STMT_H_
#define SQ_SQLITE_STMT_H_

#include <string>
#include <memory>

#pragma warning (disable:4290)

struct sqlite3;
struct sqlite3_stmt;

namespace snqu{ namespace db{

class SQLiteStatement 
{
private:
    // SQLiteStatement's ctor only to be called by SQLiteWrapper
    friend class SQLite3;
    SQLiteStatement(std::string const& statement, sqlite3*);

public:
    SQLiteStatement();

    enum dataType {
        INT = 1, //SQLITE_INTEGER,
        FLT = 2, //SQLITE_FLOAT  ,
        TXT = 3, //SQLITE_TEXT   ,
        BLB = 4, //SQLITE_BLOB   ,
        NUL = 5, //SQLITE_NULL   ,
    };

    dataType DataType(int pos_zero_indexed);

    int            ValueInt(int pos_zero_indexed);
    __int64      ValueInt64(int pos_zero_indexed);
    std::string ValueString(int pos_zero_indexed);
    double      ValueDouble(int pos_zero_indexed);
    std::string   ValueBlob(int pos_zero_indexed);
    std::string  ColumnName(int pos_zero_indexed);

    //    SQLiteStatement(const SQLiteStatement&);
    ~SQLiteStatement();

    //SQLiteStatement& operator=(SQLiteStatement const&);

    bool Bind(int pos_zero_indexed, std::string const& value);
    bool Bind(int pos_zero_indexed, double             value);
    bool Bind(int pos_zero_indexed, int                value);
    bool BindNull(int pos_zero_indexed/*Index of wildcard*/);

    bool Execute();
    bool NextRow();
    int  ColumnCount();

    /*   Call Reset if not depending on the NextRow cleaning up.
		 For example for select count(*) statements*/
    bool Reset();
    bool RestartSelect();

    bool IsOk();

private:
    sqlite3_stmt* m_stmt;
};

typedef std::shared_ptr<SQLiteStatement> SQLiteStatement_ptr;

}}



#endif