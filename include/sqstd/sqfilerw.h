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
        ����: snqu::FileRW::Exist
        Ȩ��: public static
        ����ֵ: bool
        ˵��: �ж��ļ��Ƿ����
        ���� full_path: �ļ�����·��
        ********************************************/
        static bool Exist(const std::string& full_path);

        /********************************************
        ����: snqu::FileRW::InUse
        Ȩ��: public static
        ����ֵ: bool
        ˵��: �ж��ļ��Ƿ�����ʹ��
        ���� full_path: �ļ�����·��
        ********************************************/
        static bool InUse(const std::string& full_path);

        /********************************************
        ����: snqu::FileRW::Read
        Ȩ��: public static
        ����ֵ: bool
        ˵��: ��ȡָ���ļ��Ķ���������
        ���� full_path: �ļ�����·��
        ���� file_data: ���ض�ȡ���ļ�����������
        ********************************************/
        static bool Read(const std::string& full_path, std::string& file_data, bool is_denyrw = false);

        /********************************************
        ����: snqu::FileRW::Size
        Ȩ��: public static
        ����ֵ: size_t �ļ���С
        ˵��: ����ָ���ļ��Ĵ�С
        ���� full_path: �ļ�����·��
        ********************************************/
        static size_t Size(const std::string& full_path);

        /********************************************
        ����: snqu::FileRW::Write
        Ȩ��: public
        ����ֵ: bool
        ˵��: �ô������ݸ���ָ��Ŀ¼���ļ�������������򴴽�
        ���� full_path: �ļ�����·��
        ���� file_data: д�����ݵ���ָ��
        ���� len:		д�����ݵĳ���
        ********************************************/
        static bool OverWrite(const std::string& full_path, const char* file_data, int len);

        /********************************************
        ����: snqu::FileRW::ForEachLine
        Ȩ��: public static
        ����ֵ: bool
        ˵��:
        ���� full_path: �ļ�����·��
        ���� call_back: �ص����� @line_data һ�е��������� @percent ��ȡ�İٷֱ�
        ********************************************/
        typedef std::function<bool(const std::string& line_data, int percent)> EachLienCallBack;
        static bool ForEachLine(const std::string& full_path, EachLienCallBack call_back);

    public:
        /********************************************
        ����: snqu::FileRW::FileRW
        Ȩ��: public
        ���� is_denyrw: true ��ֹ���������д�������ļ� false ��ֹ��������д,���Զ�
        ********************************************/
        explicit FileRW(bool is_denyrw = false);
        ~FileRW();

        /********************************************
        ����: snqu::FileRW::Open
        Ȩ��: public
        ����ֵ: bool
        ˵��: ��һ���ı��ļ�,����������򴴽�
        ���� full_path: �ļ�����·�� '\0'��β
        ���� is_denyrw: true ��ֹ���������д���ļ�
        ********************************************/
        bool Open(const char* full_path);
        void Close();

        /********************************************
        ����: snqu::FileRW::size
        Ȩ��: public
        ����ֵ: size_t �ļ���С
        ˵��:	����Open���ļ���С
        ********************************************/
        size_t Size();

        /********************************************
        ����: snqu::FileRW::Append
        Ȩ��: public
        ����ֵ: bool
        ˵��: �ô�������׷����Open�ӿڴ򿪵��ļ�����
        ���� file_data: �������ݵ���ָ��
        ���� len:		д�����ݵĳ���
        ���� is_add_lien:	�Զ������
        ********************************************/
        bool Append(const char* file_data, int len, bool is_add_line = false);
        inline bool AppendLine(const char* file_data, int len) { return Append(file_data, len, true); }

        /********************************************
        ����: snqu::FileRW::WriteByCache
        Ȩ��: public
        ����ֵ: bool
        ˵��: �ô�������׷����Open�ӿڴ򿪵��ļ�����
        ���û������4K����д��
        ���� file_data: �������ݵ���ָ��
        ���� len:		д�����ݵĳ���
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