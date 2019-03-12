/*---------------------------------------------------------------------------*/
/*  sqmysql.h                                                                */
/*                                                                           */
/*  History                                                                  */
/*      05/22/2017                                                           */
/*                                                                           */
/*  Author                                                                   */
/*       feng hao                                                            */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*---------------------------------------------------------------------------*/
#ifndef SQMYSQLEX_H
#define SQMYSQLEX_H

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <memory>
#include <sqdb/base/sqdbinter.h>
#include <sqdb/base/sqdbdataset.h>


namespace snqu{ namespace db{

    class SQMysql : public SQDBInter
    {
    public:
        SQMysql();
        ~SQMysql();

    public:
        int Ping() override;
        void Disconnect() override;
        int Connect(const std::vector<std::string>& conn_param) override;
        int Query(const std::string& sql_stmt, DBDataSet& result) override;
        int Execute(const std::string& sql_stmt, __int64* last_insert_id = nullptr) override;

    protected:
        int TransactionBegin() override;
        int TransactionCommit() override;
        int TransactionRollback() override;

    private:
        struct impl;
        std::unique_ptr<impl> m_impl;
    };
}
}

#endif // SQMYSQLEX_H