#define WIN32_LEAN_AND_MEAN
#include "sqmysqlex.h"
#include <direct.h>
#include <sqwin/win/sqwindows.h>
#include <database/mysql/mysql.h>
#include <sqdb/sqdbuti.h>
#include <sqwin/win/sqpath.h>
#include "../sqdblog.h"

namespace snqu { namespace db {

#define BUILD_DB_ERR ClearErr(); m_impl->build_err(m_err, m_err_msg)

        struct SQMysql::impl
        {
            MYSQL* m_db;
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
                err = mysql_errno(m_db);
                err_msg = mysql_error(m_db);
            }

            bool ssl_set()
            {
                //ssl connect need 5.6.3 version
                std::string ssl_cert_path;
                char program_path[256] = { 0 };
                _getcwd(program_path, 255);
                ssl_cert_path = path::get_module_path();
                ssl_cert_path += "\\fiper";
                MDLOG(kInfo, "ssl_cert_path:{0}", ssl_cert_path.c_str());

                auto ret = mysql_ssl_set(m_db, (ssl_cert_path + "\\client-key.pem").c_str(),
                    (ssl_cert_path + "\\client-cert.pem").c_str(),
                    (ssl_cert_path + "\\ca.pem").c_str(), nullptr, nullptr);

                //mysql_options(m_mysql, MYSQL_OPT_SSL_CA, );
                //mysql_options(m_mysql, MYSQL_OPT_SSL_CERT, );
                //mysql_options(m_mysql, MYSQL_OPT_SSL_KEY, );

                if (ret != 0)
                {
                    MDLOG(kFatal, "ssl_set failed:{0}", mysql_error(m_db));
                    return false;
                }
                return true;
            }

            // 处理数据库返回结果
            void build_result(MYSQL_RES* sql_result, DBDataSet& results)
            {
                auto num_fields = mysql_num_fields(sql_result);
                MYSQL_FIELD *fields = mysql_fetch_fields(sql_result);
                MYSQL_ROW row;
                std::string temp_str;
                char *row_name;
                
                while ((row = mysql_fetch_row(sql_result)))
                {
                    results.AddRow();
                    for (unsigned int i = 0; i < num_fields; ++i)
                    {
                        // 列名
                        if (strlen(fields[i].name) > 0)
                        {
                            if (fields[i].name[0] == '@')
                                row_name = fields[i].name + 1;
                            else
                                row_name = fields[i].name;
                        }
                        else
                            row_name = "";

                        unsigned long *lengths = mysql_fetch_lengths(sql_result);

                        switch (fields[i].type)
                        {
                        case MYSQL_TYPE_DOUBLE:
                            results.SetVal(row_name, str::ToDouble(row[i]));
                            break;
                        case MYSQL_TYPE_FLOAT:
                            results.SetVal(row_name, str::ToFloat(row[i]));
                            break;
                        case MYSQL_TYPE_TINY:
                        case MYSQL_TYPE_SHORT:
                        case MYSQL_TYPE_LONG:
                            if (fields[i].flags & UNSIGNED_FLAG)
                                results.SetVal(row_name, str::ToUInt32(row[i]));
                            else
                                results.SetVal(row_name, str::ToInt32(row[i]));
                            break;
                        case MYSQL_TYPE_LONGLONG:
                            if (fields[i].flags & UNSIGNED_FLAG)
                                results.SetVal(row_name, str::ToUInt64(row[i]));
                            else
                                results.SetVal(row_name, str::ToInt64(row[i]));
                            break;
                        case MYSQL_TYPE_LONG_BLOB:
                        case MYSQL_TYPE_BLOB:
                            results.SetVal(row_name, (const char*)row[i], lengths[i]);
                            break;
                        case MYSQL_TYPE_VAR_STRING:
                            results.SetVal(row_name, (const char*)row[i], lengths[i]);
                            break;
                        default:
                            results.SetVal(row_name, "", 0);
                            MDLOG(kError, "build_row_property unknown data type, type:%d\n", fields[i].type);
                            break;
                        }
                    }
                }
            }
        };

        SQMysql::SQMysql(void)
            : m_impl(new impl)
        {
            m_impl->m_db = nullptr;
        }


        SQMysql::~SQMysql(void)
        {
            Disconnect();
        }

        int SQMysql::Connect(const std::vector<std::string>& conn_param)
        {
            int error_code = ERR_DB_CONN;

            if (conn_param.size() < DBConn_Mysql_size)
                return ERR_DB_PARAM;

            do {

                Disconnect();

                m_impl->m_db = mysql_init(nullptr);
                if (!m_impl->m_db)
                {
                    BUILD_DB_ERR;
                    MDLOG(kError, "mysql_init fail");
                    break;
                }

//                 if (!m_impl->ssl_set())
//                 {
//                     BUILD_DB_ERR;
//                     error_code = ERR_DB_SSL;
//                     break;
//                 }

                int ml_outtime = 10;
                mysql_options(m_impl->m_db, MYSQL_OPT_CONNECT_TIMEOUT, &ml_outtime);

                int re = 0;
                my_bool reconnect = true;
                re = mysql_options(m_impl->m_db, MYSQL_OPT_RECONNECT, &reconnect);
                if (re)
                {
                    MDLOG(kError, "mysql_options fail");
                    BUILD_DB_ERR;
                    break;
                }

                if (!mysql_real_connect(m_impl->m_db, conn_param[DBConn_host].c_str(), conn_param[DBConn_user_name].c_str(),
                    conn_param[DBConn_password].c_str(), conn_param[DBConn_db_name].c_str(),
                    atoi(conn_param[DBConn_port].c_str()), NULL, 0))
                {
                    MDLOG(kError, "mysql_real_connect failed mysql_errno code[{0}] mysql_err message[{1}]",
                        mysql_errno(m_impl->m_db), mysql_error(m_impl->m_db));
                    BUILD_DB_ERR;
                    m_impl->m_db = nullptr;
                    break;
                }
                re = mysql_set_character_set(m_impl->m_db, "utf8");
                if (re)
                {
                    MDLOG(kError, "mysql_set_character_set fail");
                    BUILD_DB_ERR;
                    break;
                }
                re = mysql_select_db(m_impl->m_db, conn_param[DBConn_db_name].c_str());
                if (re)
                {
                    MDLOG(kError, "mysql_select_db fail");
                    BUILD_DB_ERR;
                    break;
                }

                // determine whether SSL is in use 
                //const char* ssl_cipher = mysql_get_ssl_cipher(m_impl->m_mysql);
                error_code = 0;
            } while (0);

            return error_code;
        }

        void SQMysql::Disconnect()
        {
            if (m_impl->m_db)
            {
                mysql_close(m_impl->m_db);
                m_impl->m_db = nullptr;
            }
        }

        int SQMysql::Execute(const std::string& sql_stmt, __int64* last_insert_id)
        {
            int error_code = ERR_DB_CONN;

            if (m_impl->m_db)
            {
                if (mysql_real_query(m_impl->m_db, sql_stmt.c_str(), sql_stmt.size()))
                {
                    error_code = ERR_DB_EXEC;
                    BUILD_DB_ERR;
                    MDLOG(kError, "sql_execute error, mysql_errno[{0}], error_code[{1}], sql_query[{2}] mysql_error[{3}]", m_err,
                        error_code, sql_stmt.c_str(), m_err_msg.c_str());
                }
                else
                {
                    error_code = ERR_OK;
                    if (nullptr != last_insert_id)
                        *last_insert_id = mysql_insert_id(m_impl->m_db);
                }
            }

            return error_code;
        }

        int SQMysql::Query(const std::string& sql_stmt, DBDataSet& results)
        {
            int error_code = ERR_DB_CONN;

            if (m_impl->m_db)
            {
                if (mysql_real_query(m_impl->m_db, sql_stmt.c_str(), sql_stmt.size()))
                {
                    error_code = ERR_DB_EXEC;
                    BUILD_DB_ERR;
                    MDLOG(kError, "sql_query error, error_code[{0}] mysql_errno[{1}] mysql_error[{2]], sql_query[{3}]",
                        error_code, m_err, m_err_msg.c_str(), sql_stmt.c_str());
                }
                else
                {
                    auto mysql_res = mysql_use_result(m_impl->m_db);
                    if (mysql_res == nullptr)
                    {
                        error_code = ERR_DB_QUERY;
                        BUILD_DB_ERR;
                        MDLOG(kError, "sql_query error ret data is empty, error_code[{0}] mysql_errno[{1}] mysql_error[{2}], sql_query[{3}]",
                            error_code, m_err, m_err_msg.c_str(), sql_stmt.c_str());
                    }
                    else
                    {
                        m_impl->build_result(mysql_res, results);
                        error_code = ERR_OK;
                    }
                }
            }

            return error_code;
        }

        int SQMysql::TransactionBegin()
        {
            if (m_impl->m_db)
            {
                std::string start_transaction = "START TRANSACTION;";
                if (mysql_real_query(m_impl->m_db, start_transaction.c_str(), start_transaction.size()))
                {
                    BUILD_DB_ERR;
                    MDLOG(kError, "start transaction error, mysql_errno[{0}] mysql_error[{1}]", m_err, m_err_msg.c_str());
                    return ERR_DB_TRANS;
                }
                mysql_autocommit(m_impl->m_db, 0);
            }
            else
                return ERR_DB_CONN;

            return ERR_OK;
        }

        int SQMysql::TransactionCommit()
        {
            if (m_impl->m_db)
            {
                if (mysql_commit(m_impl->m_db))
                {
                    BUILD_DB_ERR;
                    MDLOG(kError, "commit transaction error, mysql_errno[{0}] mysql_error[{1}]", m_err, m_err_msg.c_str());
                    return ERR_DB_TRANS;
                }
                mysql_autocommit(m_impl->m_db, 1);
            }
            else
                return ERR_DB_CONN;

            return ERR_OK;
        }

        int SQMysql::TransactionRollback()
        {
            if (m_impl->m_db)
            {
                if (mysql_rollback(m_impl->m_db))
                {
                    BUILD_DB_ERR;
                    MDLOG(kError, "rollback transaction error, mysql_errno[{0}] mysql_error[{1}]", m_err, m_err_msg.c_str());
                    return ERR_DB_TRANS;
                }
                mysql_autocommit(m_impl->m_db, 1);
            }
            else
                return ERR_DB_CONN;

            return ERR_OK;
        }

        int SQMysql::Ping()
        {
            int re = ERR_OK;
            if (m_impl->m_db)
            {
                if (mysql_ping(m_impl->m_db))
                {
                    re = ERR_ERR;
                    BUILD_DB_ERR;
                    MDLOG(kError, "Ping error, mysql_errno[{0}] mysql_error[{1}]", m_err, m_err_msg.c_str());
                }
            }
            return re;
        }

    }
}

