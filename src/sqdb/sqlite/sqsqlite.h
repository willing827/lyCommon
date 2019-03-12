/*---------------------------------------------------------------------------*/
/*  sqsqlite.h                                                               */
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
#ifndef SQ_SQLITE_H_
#define SQ_SQLITE_H_

#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <functional>
#include <memory>
#include <sqdb/base/sqdbinter.h>

namespace snqu{ namespace db{

class SQLite3 : public SQDBInter
{
public:
    SQLite3();
    ~SQLite3();

public:
    int Connect(const std::vector<std::string>& conn_param);
    void Disconnect();

public:
    int Execute(const std::string& sql_stmt, __int64* last_insert_id = nullptr);
    int Query(const std::string& sql_stmt, DBDataSet& results);

protected:
    int TransactionBegin() override;
    int TransactionCommit() override;
    int TransactionRollback() override;

private:
    struct impl;
    std::unique_ptr<impl> m_impl;
};

}}



#endif