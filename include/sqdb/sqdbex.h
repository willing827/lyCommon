/*------------------------------------------------------------------------- --*/
/*  sqdb.h                                                                   */
/*                                                                           */
/*  History                                                                  */
/*      05/21/2015  create                                                   */
/*                                                                           */
/*  Author                                                                   */
/*		feng hao															 */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*---------------------------------------------------------------------------*/

#ifndef SQDBEX_H
#define SQDBEX_H

#include <string>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <list>
#include <map>
#include <memory>
#include <thread>
#include <google/protobuf/message.h>
#include "sqdbuti.h"
#include "base/sqdbdataset.h"


namespace snqu{ namespace db{

    class DBDataSet;

    class SQDBPool
    {
    public:
        SQDBPool(void);
        virtual ~SQDBPool(void);

		/********************************************
		名称: snqu::db::SQDBPool::Create
		权限: public 
		返回值: bool
		说明:
		参数 db_type: 数据库类型
		参数 conn_param: 连接参数
		参数 pool_size:  连接池大小 不能小于1
		********************************************/
		bool Create(DBType db_type, const std::vector<std::string>& conn_param, int pool_size = 1);
		// 关闭所有连接
		void Destroy();
        // 定时执行保证空闲连接的活性
        void KeepAlive();

		/********************************************
		名称: snqu::db::SQDBPool::Query
		权限: public 
		返回值: int 错误码
		说明: 执行select操作
		参数 sql_stmt: 查询的SQL语句
		参数 result:   结果集
		********************************************/
		int Query(const std::string& sql_stmt, db::DBDataSet& result);

		/********************************************
		名称: snqu::db::SQDBPool::Execute
		权限: public 
		返回值: int
		说明: 执行insert update等操作
		参数 sql_stmt: 执行的SQL语句
		参数 last_insert_id: 上一步 INSERT 查询中产生的 AUTO_INCREMENT 的 ID 号
		********************************************/
		int Execute(const std::string& sql_stmt, __int64* last_insert_id = nullptr);
		
		/********************************************
		名称: snqu::db::SQDBPool::DoTransaction
		权限: public 
		返回值: int
		说明: 执行事务
		参数 trans_func: 事务函数体
		********************************************/
		int DoTransaction(std::function<int(void)> trans_func);

        /********************************************
        名称: snqu::db::SQDBPool::BindTable
        权限: public
        返回值: int
        说明: 通过数据集向目标表插入数据，内部无事务，数量多请用
        事务执行提高效率
        参数 table_name: 表名
        参数 key_set:    唯一键名，支持多键，但必须唯一对应一条记录
        ********************************************/
        int Insert(const std::string& table_name, const DBDataSet& result, __int64* last_insert_id = nullptr);

        /********************************************
        名称: snqu::db::SQDBPool::BindTable
        权限: public
        返回值: int
        说明: 执行表绑定，当表有唯一对应键值时，可以进行表
        绑定，这样传入数据集时，可自动拼装更新SQL语句来更新数据，
        此数据有缓存。内部无事务，数量多请用事务执行提高效率
        参数 table_name: 表名
        参数 key_set:    唯一键名，支持多键，但必须唯一对应一条记录
        ********************************************/
        void BindTable(const std::string& table_name, std::set<std::string>& key_set);

        /********************************************
        名称: snqu::db::SQDBPool::Update
        权限: public
        返回值: int
        说明: 更新表数据
        DBDataSet绑定，这样传入数据集时，可自动拼装更新SQL语句来更新数据，
        此数据有缓存。内部无事务，数量多请用事务执行提高效率
        参数 table_name: 表名
        参数 condition:   条件，为空时使用bindtable 绑定的主键 不用带where
        ********************************************/
        int Update(const std::string& table_name, const DBDataSet& result, const std::string& condition = "");
    private:
		struct impl;
		std::unique_ptr<impl> m_impl;
    };
}
}


#endif // SQDBEX_H 