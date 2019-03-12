/*---------------------------------------------------------------------------*/
/*  filerw.h                                                                 */
/*                                                                           */
/*  History                                                                  */
/*      05/25/2017  create                                                   */
/*                                                                           */
/*  Author                                                                   */
/*       feng hao                                                            */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*---------------------------------------------------------------------------*/
#ifndef FILERW_H
#define FILERW_H


#include <string>
#include <fstream>
#include <functional>
#include <atomic>
#include <vector>

namespace snqu {
    class FileRW
    {

    public:
        /********************************************
        名称: snqu::FileRW::Exist
        权限: public static
        返回值: bool
        说明: 判断文件是否存在
        参数 full_path: 文件完整路径
        ********************************************/
        static bool Exist(const std::string& full_path);

        /********************************************
        名称: snqu::FileRW::InUse
        权限: public static
        返回值: bool
        说明: 判断文件是否正在使用
        参数 full_path: 文件完整路径
        ********************************************/
        static bool InUse(const std::string& full_path);

        /********************************************
        名称: snqu::FileRW::Read
        权限: public static
        返回值: bool
        说明: 读取指定文件的二进制内容
        参数 full_path: 文件完整路径
        参数 file_data: 返回读取的文件二进制数据
        ********************************************/
        static bool Read(const std::string& full_path, std::string& file_data, bool is_denyrw = false);

        /********************************************
        名称: snqu::FileRW::Size
        权限: public static
        返回值: size_t 文件大小
        说明: 返回指定文件的大小
        参数 full_path: 文件完整路径
        ********************************************/
        static size_t Size(const std::string& full_path);

        /********************************************
        名称: snqu::FileRW::Write
        权限: public
        返回值: bool
        说明: 用传入数据覆盖指定目录的文件，如果不存在则创建
        参数 full_path: 文件完整路径
        参数 file_data: 写入数据的首指针
        参数 len:		写入数据的长度
        ********************************************/
        static bool OverWrite(const std::string& full_path, const char* file_data, int len);

        /********************************************
        名称: snqu::FileRW::ForEachLine
        权限: public static
        返回值: bool
        说明:
        参数 full_path: 文件完整路径
        参数 call_back: 回调函数 @line_data 一行的数据内容 @percent 读取的百分比
        ********************************************/
        typedef std::function<bool(const std::string& line_data, int percent)> EachLienCallBack;
        static bool ForEachLine(const std::string& full_path, EachLienCallBack call_back);

    public:
        /********************************************
        名称: snqu::FileRW::FileRW
        权限: public
        参数 is_denyrw: true 禁止其它程序读写操作的文件 false 禁止其它程序写,可以读
        ********************************************/
        explicit FileRW(bool is_denyrw = false);
        ~FileRW();

        /********************************************
        名称: snqu::FileRW::Open
        权限: public
        返回值: bool
        说明: 打开一个文本文件,如果不存在则创建
        参数 full_path: 文件完整路径 '\0'结尾
        参数 is_denyrw: true 禁止其它程序读写此文件
        ********************************************/
        bool Open(const char* full_path);
        void Close();

        /********************************************
        名称: snqu::FileRW::size
        权限: public
        返回值: size_t 文件大小
        说明:	返回Open的文件大小
        ********************************************/
        size_t Size();

        /********************************************
        名称: snqu::FileRW::Append
        权限: public
        返回值: bool
        说明: 用传入数据追加在Open接口打开的文件后面
        参数 file_data: 传入数据的首指针
        参数 len:		写入数据的长度
        参数 is_add_lien:	自动添加行
        ********************************************/
        bool Append(const char* file_data, int len, bool is_add_line = false);
        inline bool AppendLine(const char* file_data, int len) { return Append(file_data, len, true); }

        /********************************************
        名称: snqu::FileRW::WriteByCache
        权限: public
        返回值: bool
        说明: 用传入数据追加在Open接口打开的文件后面
        利用缓存进行4K对齐写入
        参数 file_data: 传入数据的首指针
        参数 len:		写入数据的长度
        ********************************************/
        bool AlignWrite(const char* file_data, unsigned int len);
        void Flush();

    private:
        std::fstream m_file_stream;
        std::string m_file_name;
        int m_denies;

        std::vector<char> m_buffer;
        std::atomic_int m_buff_size;
    };

}

#endif // FILERW_H