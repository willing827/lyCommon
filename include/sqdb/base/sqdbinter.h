/*---------------------------------------------------------------------------*/
/*  sqdbinter.h                                                               */
/*                                                                           */
/*  History                                                                  */
/*      05/22/2017 create                                                    */
/*                                                                           */
/*  Author                                                                   */
/*      feng hao                                                             */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*---------------------------------------------------------------------------*/
#ifndef SQDB_INTER_H
#define SQDB_INTER_H

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>
#include <atomic>
#include <functional>
#include <memory>
#include "sqdbdataset.h"

namespace snqu{ namespace db{

    class SQDBInter
    {
    public:
        SQDBInter();
        ~SQDBInter();
		// 获取数据库连接的线程ID
        size_t get_owner_id();
		// 设置数据库连接的线程ID
        void set_owner_id();
		// 获取数据库错误
		std::pair<int, std::string> get_last_err();

		// 执行SQL文件 可跳过'--'开始的注释行 内部无事务
        int ExecSqlFile(const char* path);		
		// 执行事务
        int DoTransaction(std::function<int(void)> trans_func); 
    public:
		// 保持连接有效
        virtual int Ping() { return 0; } 
        virtual void Disconnect() = 0;

        virtual int Connect(const std::vector<std::string>& conn_param) = 0;
        virtual int Query(const std::string& sql_stmt, DBDataSet& result) = 0;
        virtual int Execute(const std::string& sql_stmt, __int64* last_insert_id = nullptr) = 0;
        
    protected:
		void ClearErr();
		int m_err;					// 数据库错误码
		std::string m_err_msg;		// 最后出错的sql语句或是错误信息

        virtual int TransactionBegin() = 0;
        virtual int TransactionCommit() = 0;
        virtual int TransactionRollback() = 0;

    private:
        std::atomic_uint m_owner_id;
        bool m_in_transation;
    };

    typedef std::shared_ptr<SQDBInter> SQDBInter_ptr;
}}

#endif // SQDB_INTER_H