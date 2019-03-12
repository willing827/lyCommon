/*---------------------------------------------------------------------------*/
/*  sqpath.h                                                                 */
/*                                                                           */
/*  History                                                                  */
/*      08/292018   create                                                   */
/*                                                                           */
/*  Author                                                                   */
/*       feng hao                                                            */
/*                                                                           */
/*  Copyright (C) 2018 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*---------------------------------------------------------------------------*/
#pragma once
#include <string>
#include <functional>
#include <sqwin/sqwin.h>
#include <list>

namespace snqu{ namespace path{

// �õ�������,����·��
std::string get_module_filename();

// �õ���������·�� ��\\��β
std::string get_module_path();

// �õ���ǰ����Ŀ¼ 
std::string get_current_directory();

// ȡ�ø�Ŀ¼��·�����������򷵻ؿ�
std::string get_parent_path(const std::string& full_path);

// ȥ�����һ��Ŀ¼ \\ �� /
void remove_last_path(std::string& path);

// ϵͳĿ¼
std::string system();

// windowsĿ¼
std::string windows();

// ��ʱĿ¼ app_data\temp
std::string temp_dir();

//C:\Users\user\AppData\Local
std::string app_data_local(void);
std::string program_files(void);

// current_user���ڻ�ȡ <user name> or All Users�µ�Ŀ¼
std::string desktop(bool current_user); // ����·��
std::string start_menu(bool current_user);
std::string start_menu_program(bool current_user);
std::string start_menu_startup(bool current_user);
std::string start_menu_recently(void);
std::string application_data(bool current_user); //C:\Users\user\AppData\Roaming or C:\ProgramData
std::string my_documents(bool current_user);

// ���·��ת������·��
std::string absolute_path(const std::string& source);

// �ж�Ŀ¼�Ƿ���ڣ������Զ�����
bool exist_folder(const std::string& destdir, bool create = false);
// �������ļ�
int  rename_file(const char* from, const char* to);
// �����ļ�
int  copy_file(const char* from, const char* to);
// �ƶ��ļ�
int  move_file(const char* from, const char* to);
// ɾ���ļ�
int  delete_file(const char* file);
// ����Ŀ¼ ������windows·�� \�ķָ�
int  create_folder(const char* path_folder, SECURITY_ATTRIBUTES* psa = NULL);
// ɾ��Ŀ¼
bool delete_folder(const std::string& szdir);

// ��ȡ�ļ������·����
// c:\\dir1\\file.txt -> file.txt
// c:\\dir1\\dir2 -> dir2
// c:\\dir1\\dir2\\ -> dir2\  
std::string strip_path(const std::string&);

// ��ȡ��չ��
// c:\\dir1\\file.txt -> .txt
// c:\\dir1\\file.txt.zip -> .zip
std::string get_externtion(const std::string&);

//ת�� \\ to /
std::string to_linux_path(const std::string&);

//ת�� / to \\ 
std::string to_windows_path(const std::string&);

// ����Ŀ¼�����ļ� 
// param:
// do_func ���� false��ϱ���
// is_subdir=true ������Ŀ¼ 
// extern_filter ������չ�����������ĲŻص����� �� ".zip"
struct FileFDInfo
{
    std::string file_name;  // ���ļ���
    std::string file_path;  // ��������Ŀ¼
    time_t create_time;     // ����ʱ��
    time_t last_edit_time;  // ������ʱ��
    __int64 file_size;      // �ļ���С
    bool is_dir;            // �Ƿ�Ϊ��Ŀ¼
};
bool foreach_path(const std::string& path, std::function<bool(const FileFDInfo& file_fd)> do_func, 
                  bool is_subdir = false, const std::string& extern_filter = "");

bool dospath_to_ntpath(const std::string& DosPath, std::string& NtPath);

// ��ȡĳ��Ŀ¼�µ��ļ��б��Ŀ¼�б�
// is_dir  true ֻ����Ŀ¼ false ֻ�����ļ� ע:��������Ŀ¼
bool read_dir(std::list<std::string>& ret_list, const std::string& path, bool is_dir = false);

// ���ݿ�ݷ�ʽ��ȡԭ�ļ�·��
bool get_path_by_link(const std::string& link_name, std::string& ret_path, std::string* ret_args = NULL);

}}
