#include <sqdb/sqdbmanagerex.h>
#include <sqdb/sqdbex.h>
#include <thread>
#include <sqstd/sqtypes.h>
#include <sqdb/sqdberr.h>
#include "sqdblog.h"

namespace snqu{ namespace db{

SQDBManagerEx::SQDBManagerEx(void)
{
	m_pool_size = 5;
}


SQDBManagerEx::~SQDBManagerEx(void)
{
}

int SQDBManagerEx::init(std::vector<std::string>& conn_param, int db_type, int pool_size)
{
    static bool called = false;
    if (!called)
    {
        called = true;
        SQInitLogger("sqdbex");
        SLOG(kTrace) << "SQDBManagerEx create ";
    }

    if (db_type < 0 || pool_size < 0)
        return SQERROR_DB_ERR;

    m_pool_size = pool_size;
    m_db_type = db_type;
    m_dbconn_param = conn_param;

    auto error_code = init_connect_pool();

    return error_code;
}

void SQDBManagerEx::uninit()
{
    clear_connect_pool();
}

bool SQDBManagerEx::keep_alive()
{
    bool alive = false;
    m_connect_pool.foreach([&](SafeMap<SQDBEx*, bool>::value_type& item)->bool
    {
        if(item.first->keep_alive())
        {
            alive = true;
        }
        else
        {
            //item.first->reconn();
        }
        return false;
    });

    return alive;
}

int SQDBManagerEx::init_connect_pool()
{
    int error_code = SQERROR_OK;
    for (snqu::uint32 p_s = 0; p_s < m_pool_size; p_s++)
    {
        auto connect = new SQDBEx();
        if(!connect)
        {
            return SQERROR_DB_MEMEROY_ERR;
        }

        int reconnect_num = 0;
        do
        {
            error_code = connect->open(m_db_type, m_dbconn_param);
            if(error_code == 0 || reconnect_num++ >= 5 
                || error_code == 1049 || error_code == 1045 || error_code == 1046)				
            {
                break;
            }
            this_thread::sleep_for(chrono::seconds(5));
            SNLOG(kTrace, "reconnect db");
        }while(1);

        if(error_code)
        {
            delete connect;
            connect = nullptr;
            if (m_connect_pool.size() == 0)
                return SQERROR_DB_CONNECT_ERR;
        }
        else
        {
            m_connect_pool.insert(std::make_pair(connect, false));
        }
    }
	
	return SQERROR_OK;
}

int SQDBManagerEx::get_connect(SQDBEx*& connect)
{
	int error_code = 0;
	connect = nullptr;

    assert(m_connect_pool.size() > 0);

    // 找到已经分配的
    m_connect_pool.foreach([&](SafeMap<SQDBEx*, bool>::value_type& item)->bool
    {
        if(item.first->m_db_ptr->get_owner_id() == ::GetCurrentThreadId())
        {
            connect = item.first;
            return true;
        }

        return false;
    });

    if (nullptr != connect) 
        return SQERROR_OK;

    m_connect_pool.foreach([&](SafeMap<SQDBEx*, bool>::value_type& item)->bool
    {
        if(!item.second)
        {
            item.second = true;
            connect = item.first;
            if(!connect->keep_alive())
            {
                SNLOG(kTrace, "try db reconnect");
                if (!connect->open(m_db_type, m_dbconn_param))
                {
                    return false;
                }
            }

            connect->m_db_ptr->set_owner_id(::GetCurrentThreadId());
            return true;
        }
        return false;
    });

    if (nullptr != connect) 
        return SQERROR_OK;

	return SQERROR_DB_CONNECT_POOL_FULL_ERR;
}

int SQDBManagerEx::free_connect(SQDBEx* connect)
{
// 	auto ss_data = m_connect_pool.find(connect);
// 	if (ss_data != m_connect_pool.end())
// 	{
// 		if (ss_data->first->get_owner_id() != std::thread::id())
// 		{
// 			return SQERROR_OK;
// 		}
// 		else
// 		{
// 			ss_data->second = false;
// 		}
// 	}
    
	return SQERROR_OK;
}

void SQDBManagerEx::clear_connect_pool()
{
    m_connect_pool.foreach([](SafeMap<SQDBEx*, bool>::value_type& item)->bool
    {
        item.first->close();
        return false;
    });
	m_connect_pool.clear();
}

int SQDBManagerEx::sql_execute(const string& __in sql_statement, __int64* __out insert_auto_id)
{
	int error_code = 0;
	SQDBEx* connect = nullptr;
	error_code = get_connect(connect);
	if(connect)
	{
		error_code =connect->m_db_ptr->execute(sql_statement, insert_auto_id);
		free_connect(connect);
		connect = nullptr;
	}
	return error_code;
}

int SQDBManagerEx::sql_query(const string& __in sql_statement, IDataSet& __out result)
{
	int error_code = 0;
	SQDBEx* connect = nullptr;
	error_code = get_connect(connect);
	if(connect)
	{
		error_code =connect->query(sql_statement, result);
		free_connect(connect);
		connect = nullptr;
	}
	return error_code;
}


}}