/*---------------------------------------------------------------------------*/
/*  sqwindows.h                                                              */
/*                                                                           */
/*  History                                                                  */
/*      05/26/2017   create                                                  */
/*                                                                           */
/*  Author                                                                   */
/*       feng hao                                                            */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*---------------------------------------------------------------------------*/
#pragma once

#include <string>
#include <functional>
#include <sqwin/sqwin.h>

// ����Ψһ��ʶ
std::string create_guid();

/********************************************
����: GetFileVersion
Ȩ��: public 
����ֵ: �ļ��汾
˵��: ��ȡ�ļ��汾
���� path: �ļ�����·��
********************************************/
std::string GetFileVersion(const std::string& path);

/********************************************
����: GetFileInfo
Ȩ��: public
����ֵ: ��Ʒ�汾����Ϣ
˵��: ��ȡ��Ʒ�汾
���� path: �ļ�����·��
********************************************/
struct FileInfo
{
	std::string file_version_;		// �ļ��汾
	std::string product_version_;	// ��Ʒ�汾
	std::string product_name;		// ��Ʒ����
	std::string file_copy_right_;	// ��Ȩ��Ϣ
	std::string file_description_;	// �ļ�˵��
};
FileInfo GetFileInfo(const std::string& path);


bool UnPackRes(WORD wResID, const std::string& file_type, const std::string& path, HMODULE hd = NULL);

struct _SYSTEMTIME;
namespace snqu{ namespace os{

time_t SysTmToCTime(const _SYSTEMTIME& st);

// ������ԴStringTable
std::string load_string(void *hInstance, unsigned int resID);

// ���̱���
void for_each_process(std::function<bool (const std::string& proc_name, unsigned long pid, unsigned long ppid)> func);

// ��ȡ��ǰdll��exe�ľ��
HMODULE GetCurrModuleHandle();
}}
