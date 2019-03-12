#include "sqminiloger.h"
#include <sys/timeb.h>
#include <sqstd/sqfilerw.h>
#include <sqwin/win/sqwindows.h>
#include <sqstd/sqstringhelper.h>
#include <sqwin/sqwin.h>
#include <iostream>
#include <codec/sqcodec.h>
#include <sqwin/win/sqpath.h>
#include <mutex>

namespace snqu {

CriticalSection MiniLogger::m_printf_mtx;

#define BASE 65521 

#pragma warning(disable: 4018)

    inline uint32_t adler32(const char *buf)
    {
        size_t len = strlen(buf);

        unsigned long adler = 1;
        unsigned long s1 = adler & 0xffff;
        unsigned long s2 = (adler >> 16) & 0xffff;

        int i;
        for (i = 0; i < len; i++)
        {
            s1 = (s1 + buf[i]) % BASE;
            s2 = (s2 + s1) % BASE;
        }
        return (s2 << 16) + s1;
    }

	struct LogChannel
	{
		std::string file_full_path;
		std::string file_time_str;
		std::string cur_file_full_name_;
		FileRW cur_log_file;

		volatile int cur_index;
		CriticalSection m_lock;

		LogChannel()
            : cur_log_file(false)
		{
			cur_index = 0;
		}

		bool BuildFile(const std::string& module_name, const std::string& log_path, const std::string& ext_name)
		{
			if (file_full_path.empty())
			{
				file_full_path = log_path;
				if (module_name.empty())
					file_full_path.append("\\").append(path::get_module_filename());
				else
					file_full_path.append("\\").append(module_name);

				struct timeb now_t;
				ftime(&now_t);
				file_time_str.append(str::to_string(now_t.time, "%y%m%d_%H%M%S"))
					.append(std::to_string(now_t.millitm));
			}

			cur_file_full_name_ = "";
			cur_file_full_name_.append(file_full_path).append("_").append(file_time_str);

			if (cur_index > 0)
			{// 单个文件超过大小后分片
				cur_file_full_name_.append("_").append(std::to_string(cur_index));
			}
			cur_index++;
			cur_file_full_name_.append(ext_name);
			return cur_log_file.Open(cur_file_full_name_.c_str());
		}
	};


MiniLogger::MiniLogger()
{
	m_is_stop = true;
	m_flush_time = 0;
}

MiniLogger::~MiniLogger()
{
	Stop();
}

bool MiniLogger::Start(const LogInitParam& param)
{
	m_log_param = param;
    if (m_log_param.path.find(":") != std::string::npos)
    {//指定了完整路径
        m_log_dir = m_log_param.path;
    }
    else
    {
        m_log_dir = path::get_module_path();
        m_log_dir.append("\\").append(m_log_param.path);
    }

	m_is_stop = false;
	return true;
}

void MiniLogger::Stop()
{
	if (!m_is_stop)
	{
		m_is_stop = true;
		m_log_channel.foreach([&](SafeMap<uint32_t, LogChannelPtr>::value_type& item)->bool {
			if (nullptr != item.second)
			{
				item.second->cur_log_file.Close();
			}
			return false;
		});
	}
}

bool MiniLogger::AddFile(const char* log_file)
{
    auto ad32 = adler32(log_file);
	if (!m_log_channel.exist(ad32))
	{
		LogChannelPtr temp_ch = std::make_shared<LogChannel>();
		auto tmp_chl = std::make_pair(ad32, temp_ch);
		// 创建日志文件
		if (!temp_ch->BuildFile(log_file, m_log_dir, m_log_param.ext_name))
			return false;

		m_log_channel.SetVal(tmp_chl.first, tmp_chl.second);

        // 第一个日志文件把默认模块的日志加上
        if (1 == m_log_channel.size() && 0 == m_log_module.size())
            AddModule(KLogModuleId, log_file);

		return true;
	}

	return true;
}

bool MiniLogger::AddModule(const char* log_module, const char* log_file)
{
    int md_ad32 = adler32(log_module);
    if (!m_log_module.exist(md_ad32))
    {
        m_log_module.SetVal(md_ad32, adler32(log_file));
    }
    return true;
}

std::string MiniLogger::GetCurLogFileName(const char* log_module)
{
	int md_ad32 = adler32(log_module);
	LogChannelPtr temp_log = nullptr;
	m_log_channel.get(md_ad32, temp_log);

	if (nullptr != temp_log)
		return temp_log->cur_file_full_name_;

	return "";
}

inline void SetColor(int attribute)
{
    HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);  //获取缓冲区句柄
    SetConsoleTextAttribute(hCon, attribute);       //设置文本及背景色
}

void MiniLogger::OutPutStr(const char* log_module, int level, std::string& log_str)
{
    if (m_is_stop) return;

    auto log_ad32 = adler32(log_module);
    if (m_log_module.exist(log_ad32))
    {
        m_log_module.get(log_ad32, log_ad32);
    }
	else
	{//默认日志可以自动加上
		if (strcmp("default", log_module) == 0)
		{
			AddFile(log_module);
		}
	}

    LogChannelPtr temp_ch = nullptr;
    if (m_log_channel.get(log_ad32, temp_ch) && nullptr != temp_ch)
    {
        Printf(log_module, level, log_str.c_str(), m_log_param.is_out_to_console, 
			   m_log_param.is_out_put_dbg_str);

        const char* out_str = log_str.c_str();
        volatile size_t len = 0;
        std::string encrypt;
        int hdlen = 4;
        
        if (m_log_param.encrypt_func != nullptr)
        {
            encrypt = m_log_param.encrypt_func(log_str);
            len = htonl(u_long(encrypt.length())) - 9832;
            encrypt.insert(0, (char*)&len, hdlen);
            out_str = encrypt.c_str();
            len = encrypt.length();
        }
        else
        {
            len = log_str.length();
        }

        // 写入文件
        LockGuard<CriticalSection> lock(temp_ch->m_lock);
        temp_ch->cur_log_file.AlignWrite(out_str, int(len));

        if (temp_ch->cur_log_file.Size() > m_log_param.max_log_file_size)
        {
            temp_ch->cur_log_file.Flush();
			m_flush_time = GetTickCount();
            // 关闭当前文件
            temp_ch->cur_log_file.Close();
            // 创建日志文件
            if (!temp_ch->BuildFile(log_module, m_log_dir, m_log_param.ext_name))
            {
                // how to fuck it
            }
        }

		//是否实时写入
        if (m_log_param.is_no_delay)
            temp_ch->cur_log_file.Flush();
		else
		{
			//1秒必然要写入一次
			auto temp_tm = GetTickCount();
			if (temp_tm - m_flush_time > 1000)
			{
				temp_ch->cur_log_file.Flush();
				m_flush_time = temp_tm;
			}
		}
    }
}

void MiniLogger::Printf(const char* log_module, int level, const char* log_str, bool is_out_to_console, bool is_out_debug)
{
    if (is_out_to_console)
    {
        LockGuard<CriticalSection> lock(m_printf_mtx);
        switch (level)
        {
        case 0:
            SetColor(FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN); break;
        case 1:
            SetColor(FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY); break;
        case 2:
            SetColor(FOREGROUND_RED | FOREGROUND_INTENSITY); break;
        case 3:
            SetColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY); break;
        case 4:
            SetColor(FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY); break;
        default:
            SetColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY); break;
            break;
        }
        std::cout << log_str;
    }

#ifdef _WIN32
    if (is_out_debug)
    {
		std::string outstr = ktDebugSign;
		outstr.append("[").append(log_module).append("]").append(log_str);
        OutputDebugStringA(outstr.c_str());
    }
#endif // _WIN32
}

void MiniLogger::CheckLogSpace()
{
	std::vector<path::FileFDInfo> file_list;
    // 判断目录大小
    snqu::path::foreach_path(m_log_dir, [&](const path::FileFDInfo& file_fd)->bool
    {
		file_list.push_back(file_fd);
        return true;
    });

    // 删除最早的文件
	if (file_list.size() <= 1)
	{
		return;
	}

	std::sort(file_list.begin(), file_list.end(), [](const path::FileFDInfo& a, path::FileFDInfo& b)->bool {
		return a.create_time > b.create_time;
	});

	long long total_size = 0;
	std::for_each(file_list.begin(), file_list.end(), [&](const path::FileFDInfo& file_fd){
		if (total_size >= m_log_param.max_log_path_size)
		{//超出目录大小限制，清除之后的文件
			std::string tmp_file_name = m_log_dir + "\\" + file_fd.file_name;
			::DeleteFileA(tmp_file_name.c_str());
		}
		else
		{
			total_size += file_fd.file_size;
		}
	});
}

void MiniLogger::CheckLogOutDate()
{
    WIN32_FIND_DATAA wfd;
    auto dir = m_log_dir;
    dir.append("\\*");
    HANDLE hFind = FindFirstFileA(dir.c_str(), &wfd);
    if (INVALID_HANDLE_VALUE == hFind) {
        return;
    }

    do {
        if (wfd.cFileName[0] == '.') {
            continue;
        }

        if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;

        }
        else {
            SYSTEMTIME fSysTime;
            FileTimeToSystemTime(&wfd.ftCreationTime, &fSysTime);

            auto expireTimes = m_log_param.log_time_limit * 60 * 60;
            if (expireTimes <= (time(nullptr) - os::SysTmToCTime(fSysTime))) {
                auto fPath = m_log_dir;
                fPath.append("\\").append(wfd.cFileName);
                DeleteFileA(fPath.c_str());
            }
        }

    } while (FindNextFileA(hFind, &wfd));

    FindClose(hFind);
}

}