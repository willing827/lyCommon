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
		// ��ȡ���ݿ����ӵ��߳�ID
        size_t get_owner_id();
		// �������ݿ����ӵ��߳�ID
        void set_owner_id();
		// ��ȡ���ݿ����
		std::pair<int, std::string> get_last_err();

		// ִ��SQL�ļ� ������'--'��ʼ��ע���� �ڲ�������
        int ExecSqlFile(const char* path);		
		// ִ������
        int DoTransaction(std::function<int(void)> trans_func); 
    public:
		// ����������Ч
        virtual int Ping() { return 0; } 
        virtual void Disconnect() = 0;

        virtual int Connect(const std::vector<std::string>& conn_param) = 0;
        virtual int Query(const std::string& sql_stmt, DBDataSet& result) = 0;
        virtual int Execute(const std::string& sql_stmt, __int64* last_insert_id = nullptr) = 0;
        
    protected:
		void ClearErr();
		int m_err;					// ���ݿ������
		std::string m_err_msg;		// �������sql�����Ǵ�����Ϣ

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