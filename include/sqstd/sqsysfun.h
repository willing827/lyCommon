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

	// ����С��maxsize��count�����ظ��������
	void generate_rand_numbers(int maxsize, int count, std::vector<int>& output);
	
	// �ͷ��ļ���ָ����Ŀ¼
	bool release_file(const string& path, const string& buffer, 
					  int32 length, bool create_always);

	// �����ļ����ݵ�string����
	string load_file(const string& path);

	// �����ļ�
	bool save_file(const string& path, const string& data);

	// ��������
	bool run_process(const string& app_name, const string& cmd_line,
		bool sw_show, uint32& pid, HANDLE& process);

	// ��ָ����������������
	bool run_process_as_desktop(const string& app_name, const string& cmd_line,
		bool sw_show, uint32& pid, HANDLE& process,
		const string& desktop);

	// ��������
	bool killprocess(const string& process);

	// ��ȡָ�����̵�·��
	std::string get_process_full_path(uint32_t pid);

	bool find_process(uint32_t pid);


}
#endif //SQSYSFUN_H