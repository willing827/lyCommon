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
		����: snqu::db::SQDBPool::Create
		Ȩ��: public 
		����ֵ: bool
		˵��:
		���� db_type: ���ݿ�����
		���� conn_param: ���Ӳ���
		���� pool_size:  ���ӳش�С ����С��1
		********************************************/
		bool Create(DBType db_type, const std::vector<std::string>& conn_param, int pool_size = 1);
		// �ر���������
		void Destroy();
        // ��ʱִ�б�֤�������ӵĻ���
        void KeepAlive();

		/********************************************
		����: snqu::db::SQDBPool::Query
		Ȩ��: public 
		����ֵ: int ������
		˵��: ִ��select����
		���� sql_stmt: ��ѯ��SQL���
		���� result:   �����
		********************************************/
		int Query(const std::string& sql_stmt, db::DBDataSet& result);

		/********************************************
		����: snqu::db::SQDBPool::Execute
		Ȩ��: public 
		����ֵ: int
		˵��: ִ��insert update�Ȳ���
		���� sql_stmt: ִ�е�SQL���
		���� last_insert_id: ��һ�� INSERT ��ѯ�в����� AUTO_INCREMENT �� ID ��
		********************************************/
		int Execute(const std::string& sql_stmt, __int64* last_insert_id = nullptr);
		
		/********************************************
		����: snqu::db::SQDBPool::DoTransaction
		Ȩ��: public 
		����ֵ: int
		˵��: ִ������
		���� trans_func: ��������
		********************************************/
		int DoTransaction(std::function<int(void)> trans_func);

        /********************************************
        ����: snqu::db::SQDBPool::BindTable
        Ȩ��: public
        ����ֵ: int
        ˵��: ͨ�����ݼ���Ŀ���������ݣ��ڲ�����������������
        ����ִ�����Ч��
        ���� table_name: ����
        ���� key_set:    Ψһ������֧�ֶ����������Ψһ��Ӧһ����¼
        ********************************************/
        int Insert(const std::string& table_name, const DBDataSet& result, __int64* last_insert_id = nullptr);

        /********************************************
        ����: snqu::db::SQDBPool::BindTable
        Ȩ��: public
        ����ֵ: int
        ˵��: ִ�б�󶨣�������Ψһ��Ӧ��ֵʱ�����Խ��б�
        �󶨣������������ݼ�ʱ�����Զ�ƴװ����SQL������������ݣ�
        �������л��档�ڲ���������������������ִ�����Ч��
        ���� table_name: ����
        ���� key_set:    Ψһ������֧�ֶ����������Ψһ��Ӧһ����¼
        ********************************************/
        void BindTable(const std::string& table_name, std::set<std::string>& key_set);

        /********************************************
        ����: snqu::db::SQDBPool::Update
        Ȩ��: public
        ����ֵ: int
        ˵��: ���±�����
        DBDataSet�󶨣������������ݼ�ʱ�����Զ�ƴװ����SQL������������ݣ�
        �������л��档�ڲ���������������������ִ�����Ч��
        ���� table_name: ����
        ���� condition:   ������Ϊ��ʱʹ��bindtable �󶨵����� ���ô�where
        ********************************************/
        int Update(const std::string& table_name, const DBDataSet& result, const std::string& condition = "");
    private:
		struct impl;
		std::unique_ptr<impl> m_impl;
    };
}
}


#endif // SQDBEX_H 