/*---------------------------------------------------------------------------*/
/*  sqdbencryptor.h                                                          */
/*                                                                           */
/*  History                                                                  */
/*      08/01/2017 create                                                    */
/*                                                                           */
/*  Author                                                                   */
/*      feng hao                                                             */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*---------------------------------------------------------------------------*/
#ifndef SQDB_DB_ENCRYPTOR_H
#define SQDB_DB_ENCRYPTOR_H

#include <functional>
#include <set>

namespace snqu{ namespace db{

    namespace detail {
        inline const char* ShortName(const char* colname)
        {
            auto ret = strchr(colname, '.');
            if (ret != nullptr)
            {
                colname = ret + 1;
            }
            return colname;
        }
    }

    typedef std::function<std::string(const std::string&)> CyptFunc;
    typedef std::set<std::string> EncryptFileds;
    class DBEncryptor
    {
    public:
        /********************************************
        ����: snqu::db::SQDBPool::BindEncryptFunc
        Ȩ��: public
        ����ֵ: void
        ˵��: ���ֶμӽ��ܺ���
        ����δ����
        ���� field_name:	 Ҫ���ܵ��ֶ�����ȫ��Ψһ����ͬ���ֶ�����ͬ�ͻ�һ�����
        ********************************************/
        void AddEncryptField(const char* col_name) { m_en_field.insert(col_name); };
        inline bool IsEnField(const char* col_name)
        {
            col_name = detail::ShortName(col_name);
            if (m_en_field.count(col_name))
            {
                return true;
            }
            return false;
        }

        CyptFunc m_encrypt_fun;     // ���ü��ܺ���
        CyptFunc m_decrypt_fun;     // ���ý��ܺ���
    private:
        EncryptFileds m_en_field;
    };

}}

#endif // SQDB_DB_ENCRYPTOR_H