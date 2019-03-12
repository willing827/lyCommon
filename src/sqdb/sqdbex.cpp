#include <sqdb/sqdbex.h>
#include <sqstd/sqstringhelper.h>
#include <sqstd/sqthreadhelper.h>
#include <sstream>
#include <mutex>
#include <queue>
#include <sqdb/base/sqjsontosql.h>

#include "mysql/sqmysqlex.h"
#include "sqlite/sqsqlite.h"

namespace snqu{ namespace db{

	struct SQDBPool::impl
	{
		int m_db_type;
		int m_pool_size;
		std::mutex m_pool_mutex;
		std::queue<SQDBInter_ptr> m_idle_conn;
		std::map<size_t/*thd id*/, SQDBInter_ptr> m_using_conn;
		std::vector<std::string> m_conn_param;
        std::map<std::string, std::set<std::string>> m_table_key_bind;

		impl()
		{}
		~impl()
		{}

		bool NewConn()
		{
			SQDBInter_ptr db_ptr = nullptr;
			if (DB_Mysql == m_db_type)
				db_ptr = std::make_shared<SQMysql>();
			else if (DB_Sqlite3 == m_db_type)
				db_ptr = std::make_shared<SQLite3>();
			else
				return false;

			if (db_ptr->Connect(m_conn_param))
			{
				return false;
			}

			m_idle_conn.push(db_ptr);
			return true;
		}

		SQDBInter_ptr GetConn()
		{
			std::lock_guard<std::mutex> lock(m_pool_mutex);
			auto item = m_using_conn.find(GetCurThdID());
			if (item != m_using_conn.end())
				return item->second;

			if (m_idle_conn.size() == 0)
				return nullptr;

			auto ret_conn = m_idle_conn.front();
			m_idle_conn.pop();
			ret_conn->set_owner_id();
			m_using_conn.insert(std::make_pair(ret_conn->get_owner_id(), ret_conn));

			return ret_conn;
		}

		void PutConn(SQDBInter_ptr conn_ptr)
		{
			std::lock_guard<std::mutex> lock(m_pool_mutex);
			m_using_conn.erase(conn_ptr->get_owner_id());
			m_idle_conn.push(conn_ptr);
		}
	};

    SQDBPool::SQDBPool()
		: m_impl(new impl)
    {
    }

    SQDBPool::~SQDBPool()
    {
		Destroy();
    }

    bool SQDBPool::Create(DBType db_type, const std::vector<std::string>& conn_param, int pool_size)
    {
		Destroy();

		if (conn_param.size() == 0 || pool_size <= 0)
		{
			return false;
		}

		m_impl->m_db_type = db_type;
		m_impl->m_pool_size = pool_size;
		m_impl->m_conn_param = conn_param;

		for (int i = 0; i < pool_size; i++)
		{
			m_impl->NewConn();
		}

		if (m_impl->m_idle_conn.empty())
			return false;

		return true;
    }

    void SQDBPool::Destroy()
    {
		std::lock_guard<std::mutex> lock(m_impl->m_pool_mutex);
		while(!m_impl->m_idle_conn.empty())
		{
			auto conn = m_impl->m_idle_conn.front();
			m_impl->m_idle_conn.pop();
			conn->Disconnect();
		}

		for (auto& item : m_impl->m_using_conn)
		{
			item.second->Disconnect();
		}

		m_impl->m_using_conn.clear();
    }

    void SQDBPool::KeepAlive()
    {
        std::lock_guard<std::mutex> lock(m_impl->m_pool_mutex);
        int size = m_impl->m_idle_conn.size();
        for (int i = 0; i < size; i++)
        {
            auto item = m_impl->m_idle_conn.front();
            m_impl->m_idle_conn.pop();
            item->Ping();
            m_impl->m_idle_conn.push(item);
        }
    }

	int SQDBPool::Query(const std::string& sql_stmt, db::DBDataSet& result)
    {
		auto db_ptr = m_impl->GetConn();

		if (nullptr == db_ptr)
		{
			return ERR_DB_POOL_MAX;
		}

		auto ret = db_ptr->Query(sql_stmt, result);

		m_impl->PutConn(db_ptr);

		return ret;
    }

	int SQDBPool::Execute(const std::string& sql_stmt, __int64* last_insert_id)
	{
		auto db_ptr = m_impl->GetConn();

		if (nullptr == db_ptr)
		{
			return ERR_DB_POOL_MAX;
		}

		auto ret = db_ptr->Execute(sql_stmt, last_insert_id);

		m_impl->PutConn(db_ptr);

		return ret;
	}

	int SQDBPool::DoTransaction(std::function<int(void)> trans_func)
	{
		auto db_ptr = m_impl->GetConn();

		if (nullptr == db_ptr)
		{
			return ERR_DB_POOL_MAX;
		}

		auto ret = db_ptr->DoTransaction(trans_func);

		m_impl->PutConn(db_ptr);

		return ret;
	}

    int SQDBPool::Insert(const std::string& table_name, const db::DBDataSet& dtset, __int64* last_insert_id)
    {
        // 传入参数错误
        if (0 == dtset.GetRowCount()) return ERR_DB_PARAM;

        std::string sql_stmt = snqu::JsonToInsertSql(table_name, dtset.m_records);
        return Execute(sql_stmt, last_insert_id);
    }

    void SQDBPool::BindTable(const std::string& table_name, std::set<std::string>& key_set)
    {
        m_impl->m_table_key_bind[table_name] = key_set;
    }

    int SQDBPool::Update(const std::string& table_name, const DBDataSet& dtset, const std::string& condition)
    {
        // 传入参数错误
        if (0 == dtset.GetRowCount()) return ERR_DB_PARAM;
        // 未绑定表数据
        auto bind_iter = m_impl->m_table_key_bind.find(table_name);
        if (bind_iter == m_impl->m_table_key_bind.end() && condition.empty())
            return ERR_DB_NO_COND;

        int err = ERR_OK;
        std::string sql_stmt, cond;
        while (dtset.NextRow())
        {
            if (!condition.empty())
            {
                sql_stmt = snqu::JsonToUpdateSql(table_name, dtset.m_records[dtset.m_cur_row_id]);
                sql_stmt.append(" WHERE ").append(condition);
            }
            else
            {
                sql_stmt = snqu::JsonToUpdateSql(table_name, dtset.m_records[dtset.m_cur_row_id], &bind_iter->second, &cond);
                sql_stmt.append(" WHERE ").append(cond);
            }
                
            err = Execute(sql_stmt);
            if (err != ERR_OK)
            {
                break;
            }
        }

        return err;
    }

}}