/*---------------------------------------------------------------------------*/
/*  sqapi.h                                                                  */
/*                                                                           */
/*  History                                                                  */
/*      07/15/2017                                                           */
/*                                                                           */
/*  Author                                                                   */
/*      feng hao                                                             */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*---------------------------------------------------------------------------*/

#ifndef SNQU_API_H
#define SNQU_API_H

#if defined(_MSC_VER)

#ifdef API_EXPORTS
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

#else
#define DLL_API
#endif

#define DLL_CALL __stdcall

#endif // SNQU_API_H
