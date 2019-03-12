/*-------------------------------------------------------------------------*/
/*  sqsysfun.h																 */
/*                                                                           */
/*  History                                                                  */
/*      04/29/2015															 */
/*                                                                           */
/*  Author                                                                   */
/*      Guolei                                                               */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*-------------------------------------------------------------------------*/
#ifndef SQSYSFUN_H
#define SQSYSFUN_H


#include <sqstd/sqbasictypes.h>
using namespace std;
namespace snqu {
    string get_file_data(const std::string& file);
	string generate_rand_token(std::string source);
	string get_execute_file();

	string generate_rand_string(bool ul_mix, int length);

	// 生成小于maxsize的count个不重复的随机数
	void generate_rand_numbers(int maxsize, int count, std::vector<int>& output);
	
	// 释放文件到指定的目录
	bool release_file(const string& path, const string& buffer, 
					  int32 length, bool create_always);

	// 加载文件内容到string里面
	string load_file(const string& path);

	// 保存文件
	bool save_file(const string& path, const string& data);

	// 启动进程
	bool run_process(const string& app_name, const string& cmd_line,
		bool sw_show, uint32& pid, HANDLE& process);

	// 在指定的桌面启动进程
	bool run_process_as_desktop(const string& app_name, const string& cmd_line,
		bool sw_show, uint32& pid, HANDLE& process,
		const string& desktop);

	// 结束进程
	bool killprocess(const string& process);

	// 获取指定进程的路径
	std::string get_process_full_path(uint32_t pid);

	bool find_process(uint32_t pid);


}
#endif //SQSYSFUN_H