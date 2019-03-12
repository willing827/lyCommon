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

// 创建唯一标识
std::string create_guid();

/********************************************
名称: GetFileVersion
权限: public 
返回值: 文件版本
说明: 获取文件版本
参数 path: 文件完整路径
********************************************/
std::string GetFileVersion(const std::string& path);

/********************************************
名称: GetFileInfo
权限: public
返回值: 产品版本等信息
说明: 获取产品版本
参数 path: 文件完整路径
********************************************/
struct FileInfo
{
	std::string file_version_;		// 文件版本
	std::string product_version_;	// 产品版本
	std::string product_name;		// 产品名称
	std::string file_copy_right_;	// 版权信息
	std::string file_description_;	// 文件说明
};
FileInfo GetFileInfo(const std::string& path);


bool UnPackRes(WORD wResID, const std::string& file_type, const std::string& path, HMODULE hd = NULL);

struct _SYSTEMTIME;
namespace snqu{ namespace os{

time_t SysTmToCTime(const _SYSTEMTIME& st);

// 加载资源StringTable
std::string load_string(void *hInstance, unsigned int resID);

// 进程遍历
void for_each_process(std::function<bool (const std::string& proc_name, unsigned long pid, unsigned long ppid)> func);

// 获取当前dll、exe的句柄
HMODULE GetCurrModuleHandle();
}}
