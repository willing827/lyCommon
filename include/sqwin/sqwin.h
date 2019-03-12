/*---------------------------------------------------------------------------*/
/*  sqwin.h                                                                  */
/*                                                                           */
/*  History                                                                  */
/*      05/26/2017  create                                                   */
/*                                                                           */
/*  Author                                                                   */
/*       feng hao                                                            */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*---------------------------------------------------------------------------*/
// 仅WINDOWS可用 帮助解决头文件冲突，所有用到WINSOW的头文件的尽量使用此文件包含

#pragma once
#include <winsock2.h>
#include <Windows.h>
#include <Winbase.h>
#include <windef.h>
#include <SDKDDKVer.h>
#include <sqstd/sqtypes.h>
#include <tchar.h>

#include <sqwin/win/sqfilesignature.h>
#include <sqwin/win/sqfilevercheck.h>
#include <sqwin/win/sqwinftp.h>
#include <sqwin/win/sqos.h>
#include <sqwin/win/sqpath.h>
#include <sqwin/win/sqservice.h>
#include <sqwin/win/sqtools.h>
#include <sqwin/win/sqwindowmsg.h>
#include <sqwin/win/sqwindows.h>
#include <sqwin/win/sqwinnet.h>
#include <sqwin/win/sqwinsock.h>

