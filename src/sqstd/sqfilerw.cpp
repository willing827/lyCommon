#include <sqstd/sqfilerw.h>
#include <sqstd/sqvecbuffhelper.h>
#include <sqwin/win/sqpath.h>
#ifdef _WIN32
#include <io.h>
#include <stdint.h>
#else
#include <dirent.h>
#include <unistd.h>
#endif // WIN32


namespace snqu {

    static const unsigned int BUF_SIZE = 4096; // 4K对齐写入
	
	bool FileRW::Exist(const std::string& full_path)
	{
#ifdef _WIN32
		return _access(full_path.c_str(), 0) == 0;
#else
		return access(full_path.c_str(), 0) == 0;
#endif // WIN32
	}

	bool FileRW::InUse(const std::string& full_path)
	{
		if (Exist(full_path))
		{
			FILE* file = NULL;
			auto ret = fopen_s(&file, full_path.c_str(), "ab");
			if (ret == EACCES)
				return true;
			else if (ret == 0)
			{
				fclose(file);
				return false;
			}
		}
		return false;
	}

	bool FileRW::Read(const std::string& full_path, std::string& file_data, bool is_denyrw)
	{
		std::fstream file_stream;
		// 读的时候禁止写
        file_stream.open(full_path, std::ios::binary | std::ios::in, is_denyrw ? SH_DENYRW : SH_DENYNO);

		if (!file_stream) return false;
			
		file_stream.seekg(0, file_stream.end);
		size_t file_len = size_t(file_stream.tellg());

		// 分配内存空间  
		file_data.resize(file_len, 0);
		file_stream.seekg(0, file_stream.beg);

		// 获取文件内容  
		size_t cur_pos = 0;

		do
		{
			// 这里可以分片读然后就支持打断
			file_stream.read(&file_data[cur_pos], (file_len - cur_pos));

			if (file_stream.fail())
			{
				file_stream.close();
				return false;
			}
			cur_pos += (size_t)file_stream.gcount();

		} while (0 != cur_pos && file_len != cur_pos);

		file_stream.close();
		return true;
	}

	size_t FileRW::Size(const std::string& full_path)
	{
		size_t ret_size = 0;

		std::fstream file_stream;
		// 读大小的时候禁止写
		file_stream.open(full_path.c_str(), std::ios::binary | std::ios::in, SH_DENYWR);
		if (file_stream)
		{
			file_stream.seekg(0, file_stream.end);
			ret_size = size_t(file_stream.tellg());
			file_stream.close();
		}

		return ret_size;
	}

	bool FileRW::OverWrite(const std::string& full_path, const char* file_data, int len)
	{
		path::exist_folder(path::get_parent_path(full_path), true);

		// 清空文件内容
		std::fstream file_stream;
		// 写的时候禁止其它人操作
		file_stream.open(full_path.c_str(), std::ios::out | std::ios::trunc | std::ios::binary, SH_DENYRW);
		if (!file_stream)
			return false;

		file_stream.write(file_data, len);
		file_stream.flush();

		if (file_stream.fail())
		{
			file_stream.clear();
			file_stream.close();
			return false;
		}

		file_stream.close();
		return true;
	}

	bool FileRW::ForEachLine(const std::string& full_path, EachLienCallBack call_back)
	{
		size_t buf_len = 1024;
		std::fstream file_stream;
		std::string ret_str(buf_len + 1, '\0');
		file_stream.open(full_path.c_str(), std::ios::in | std::ios::binary, SH_DENYWR);
		if (file_stream)
		{
			file_stream.seekg(0, file_stream.end);
			size_t total_size = size_t(file_stream.tellg());

			file_stream.seekg(0, file_stream.beg);
			auto len = buf_len;
			size_t cur_pos(0), percent(0);
			while (!file_stream.eof())
			{
				getline(file_stream, ret_str);

				if (!file_stream.eof())
				{
					cur_pos += ret_str.length() + 2;
					percent = cur_pos * 100 / total_size;
				}
				else
				{
					percent = 100;
                    break;
				}

                if (ret_str.size() > 0)
                    ret_str.resize(ret_str.length() - 1);

				if (!call_back(ret_str, percent))
				{
					break;
				}

				if (file_stream.bad())
				{
					file_stream.close();
					return false;
				}

				ret_str = "";
			}
			file_stream.close();
			return true;
		}
		return false;
	}

	FileRW::FileRW(bool is_denyrw)
	{
        m_denies = is_denyrw ? _SH_DENYRW : _SH_DENYNO;
        m_buffer.reserve(BUF_SIZE);
	}

	FileRW::~FileRW()
	{
		Close();
	}

	bool FileRW::Open(const char* full_name)
	{
		m_buff_size = 0;

		if (nullptr == full_name) return false;

        path::exist_folder(path::get_parent_path(full_name), true);

		m_file_name = full_name;

		m_file_stream.open(m_file_name.c_str(), std::ios::binary | std::ios::in 
							| std::ios::app | std::ios::out , m_denies);
		if (!m_file_stream)
			return false;

		return true;
	}

	void FileRW::Close()
	{
		if (m_file_stream.is_open())
		{
			Flush();
			m_file_stream.flush();
			m_file_stream.close();
			m_buff_size = 0;
		}
	}

	size_t FileRW::Size()
	{
		if (!m_file_stream || !m_file_stream.is_open())
		{
			return 0;
		}

		m_file_stream.seekg(0, m_file_stream.end);
		auto srcSize = m_file_stream.tellg();

		return size_t(srcSize) + m_buff_size;
	}

	bool FileRW::Append(const char* file_data, int len, bool is_add_line)
	{
		if (!m_file_stream || !m_file_stream.is_open())
			return false;

		m_file_stream.write(file_data, len);
		if (m_file_stream.fail())
		{
			m_file_stream.clear();
			return false;
		}

		if (is_add_line)
		{
			#ifdef _WIN32
			m_file_stream.write("\r\n", 2);
			#else
			m_file_stream.write("\n", 1);
			#endif
		}

		return true;
	}

	void FileRW::Flush()
	{
		if (m_buff_size == 0)
			return;

		m_file_stream.write(&m_buffer[0], m_buff_size);
		m_file_stream.flush();
		m_buff_size = 0;
        m_buffer.clear();
	}

	bool FileRW::AlignWrite(const char* file_data, unsigned int len)
	{
		auto p_cur = file_data;
		int buff_cpy = len;
        // 有缓存部分先凑齐写入
		if (0 != m_buff_size && m_buff_size + len > BUF_SIZE)
		{
			// 凑齐后写入
			int tail_len = BUF_SIZE - m_buff_size;
            buffer::write(m_buffer, m_buff_size, file_data, tail_len);
            m_buff_size = BUF_SIZE;
			p_cur += tail_len;
			this->Flush();
			buff_cpy -= tail_len;
		}

		while (buff_cpy > BUF_SIZE)
		{
			m_file_stream.write(p_cur, BUF_SIZE);
			p_cur += BUF_SIZE;
			buff_cpy -= BUF_SIZE;
		}

        buffer::write(m_buffer, m_buff_size, p_cur, buff_cpy);
		m_buff_size += buff_cpy;

		return true;
	}

}