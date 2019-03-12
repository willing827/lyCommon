/*---------------------------------------------------------------------------*/
/*  sqos.h                                                                   */
/*                                                                           */
/*  History                                                                  */
/*      08/29/2018   create                                                  */
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

namespace snqu{ namespace os{

enum Ms_Os_Types{
    MS_UNKNOWN = -1,
	MS_WIN_XP = 0,
    MS_WIN_2000 = 1,
    MS_WIN_2003 = 2,
    MS_WIN_2008 = 3,
    MS_WIN_VISTA = 4,
	MS_WIN_WIN7 = 5,
    MS_WIN_WIN8 = 6,
    MS_WIN_2012 = 7,
    MS_WIN_WIN8_1 = 8,
    MS_WIN_2012_R2 = 9,
    MS_WIN_10 = 10,
    MS_WIN_2016 = 11,
    MS_WIN_2003_R2 = 12,
    MS_WIN_2008_R2 = 13,
};

// 设置系统时间
bool SetTime(__int64 utc_time);

// 获取系统版本类型
bool IsWinXP();

// 判断系统是否为64bit
bool IsWow64();

// 获取操作系统版本号
std::string get_os_version();
Ms_Os_Types get_os_type();

std::string get_host_name();

}}
