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

// 得到程序名,完整路径
std::string get_module_filename();

// 得到程序所在路径 无\\结尾
std::string get_module_path();

// 得到当前工作目录 
std::string get_current_directory();

// 取得父目录的路径，不存在则返回空
std::string get_parent_path(const std::string& full_path);

// 去除最后一个目录 \\ 或 /
void remove_last_path(std::string& path);

// 系统目录
std::string system();

// windows目录
std::string windows();

// 临时目录 app_data\temp
std::string temp_dir();

//C:\Users\user\AppData\Local
std::string app_data_local(void);
std::string program_files(void);

// current_user用于获取 <user name> or All Users下的目录
std::string desktop(bool current_user); // 桌面路径
std::string start_menu(bool current_user);
std::string start_menu_program(bool current_user);
std::string start_menu_startup(bool current_user);
std::string start_menu_recently(void);
std::string application_data(bool current_user); //C:\Users\user\AppData\Roaming or C:\ProgramData
std::string my_documents(bool current_user);

// 相对路径转换绝对路径
std::string absolute_path(const std::string& source);

// 判断目录是否存在，可以自动创建
bool exist_folder(const std::string& destdir, bool create = false);
// 重命名文件
int  rename_file(const char* from, const char* to);
// 拷贝文件
int  copy_file(const char* from, const char* to);
// 移动文件
int  move_file(const char* from, const char* to);
// 删除文件
int  delete_file(const char* file);
// 创建目录 必须是windows路径 \的分隔
int  create_folder(const char* path_folder, SECURITY_ATTRIBUTES* psa = NULL);
// 删除目录
bool delete_folder(const std::string& szdir);

// 获取文件名或短路径名
// c:\\dir1\\file.txt -> file.txt
// c:\\dir1\\dir2 -> dir2
// c:\\dir1\\dir2\\ -> dir2\  
std::string strip_path(const std::string&);

// 获取扩展名
// c:\\dir1\\file.txt -> .txt
// c:\\dir1\\file.txt.zip -> .zip
std::string get_externtion(const std::string&);

//转换 \\ to /
std::string to_linux_path(const std::string&);

//转换 / to \\ 
std::string to_windows_path(const std::string&);

// 遍历目录所有文件 
// param:
// do_func 返回 false打断遍历
// is_subdir=true 遍历子目录 
// extern_filter 满足扩展名过滤条件的才回调处理 如 ".zip"
struct FileFDInfo
{
    std::string file_name;  // 短文件名
    std::string file_path;  // 所在完整目录
    time_t create_time;     // 创建时间
    time_t last_edit_time;  // 最后访问时间
    __int64 file_size;      // 文件大小
    bool is_dir;            // 是否为子目录
};
bool foreach_path(const std::string& path, std::function<bool(const FileFDInfo& file_fd)> do_func, 
                  bool is_subdir = false, const std::string& extern_filter = "");

bool dospath_to_ntpath(const std::string& DosPath, std::string& NtPath);

// 获取某个目录下的文件列表或目录列表
// is_dir  true 只返回目录 false 只返回文件 注:不遍历子目录
bool read_dir(std::list<std::string>& ret_list, const std::string& path, bool is_dir = false);

// 根据快捷方式获取原文件路径
bool get_path_by_link(const std::string& link_name, std::string& ret_path, std::string* ret_args = NULL);

}}
