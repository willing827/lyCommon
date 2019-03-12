#include <sqdb/base/sqdbinter.h>
#include <sqstd/sqfilerw.h>
#include <sqdb/sqdbuti.h>
#include <sqstd/sqthreadhelper.h>

namespace snqu{ namespace db{

    SQDBInter::SQDBInter()
        : m_in_transation(false)
    {
        m_owner_id = 0;
		m_err = 0;
    }

    SQDBInter::~SQDBInter()
    {

    }

	void SQDBInter::ClearErr()
	{
		m_err = ERR_OK;
		m_err_msg = "";
	}

	std::pair<int, std::string> SQDBInter::get_last_err()
	{
		return std::make_pair(m_err, m_err_msg);
	}

    int SQDBInter::DoTransaction(std::function<int(void)> trans_func)
    {
        int error_code(-1);

        if (m_in_transation)
        {// 已经在事务中
            return trans_func();
        }

        do
        {
            error_code = TransactionBegin();
            if(ERR_OK != error_code)
            {
                break;
            }
            m_in_transation = true;
            error_code = trans_func();

            if(ERR_OK == error_code)
                error_code = TransactionCommit();

            if(ERR_OK != error_code)
            {
                TransactionRollback();
            }

            m_in_transation = false;

            error_code = ERR_OK;
            
        }while (0);

        return error_code;
    }

    size_t SQDBInter::get_owner_id()
    {
        return m_owner_id;
    }

    void SQDBInter::set_owner_id()
    {
        m_owner_id.store(GetCurThdID());
    }

    inline size_t get_new_line(const std::string& src_str, size_t find_start)
    {
        auto dot_pos = src_str.find('\r', find_start);
        if (std::string::npos == dot_pos)
            return dot_pos;

        dot_pos = src_str.find('\n', dot_pos);
        if (std::string::npos == dot_pos)
            return dot_pos;

        return dot_pos + 1;
    }

    inline size_t get_new_sql_len(const std::string& src_str, size_t find_start)
    {
        auto dot_pos = src_str.find(';', find_start);
        if (std::string::npos == dot_pos)
            return dot_pos;

        return dot_pos - find_start + 1;
    }

    int SQDBInter::ExecSqlFile(const char* path)
    {
        int err(0);

        std::string sql_stmt, file_str;
        // read file data
		if (!FileRW::Read(path, file_str))
			return ERR_DB_FILE_RW;

        auto file_len = file_str.length();
        size_t cur_pos = 0;

        while (1)
        {
            auto fd_pos = get_new_line(file_str, cur_pos);

            if (fd_pos >= file_len || std::string::npos == fd_pos)
                break;

            //跳过 '/'和'--'
            auto ch = file_str.at(cur_pos);
            if (ch == '/')
            {
                cur_pos = file_str.find('/', cur_pos + 1);
                if (std::string::npos == cur_pos)
                    break;
                cur_pos += 1;
                continue;
            }

            if (ch == '-' || ch == '\r' || ch == '\n')
            {
                cur_pos = fd_pos;
                continue;
            }

            auto new_stmt_len = get_new_sql_len(file_str, cur_pos);
            if (std::string::npos == new_stmt_len)
                break;

            sql_stmt.resize(new_stmt_len);

            file_str.copy(&sql_stmt[0], new_stmt_len, cur_pos);

            cur_pos += new_stmt_len;

            err = Execute(sql_stmt.c_str());

            if (err) break;
        }

        return err;
    }
}
}

