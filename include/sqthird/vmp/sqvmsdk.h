/*--------------------------------------------------------------------------*/
/*  sqvmsdk.h                                                                */
/*                                                                           */
/*  History                                                                  */
/*      01/30/2016                                                           */
/*                                                                           */
/*  Author                                                                   */
/*      GUO LEI																 */
/*                                                                           */
/*  Copyright (C) 2016 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*--------------------------------------------------------------------------*/
#ifndef __VM_SDK_H__
#define __VM_SDK_H__


#ifdef VMP_CODE
#include "VMProtectSDK.h"
#pragma message("##using VMP_CODE macros.")
#define _VMProtectBegin(p) VMProtectBegin(p)
#define _VMProtectBeginVirtualization(p)  VMProtectBeginVirtualization(p)
#define _VMProtectBeginMutation(p) VMProtectBeginMutation(p)
#define _VMProtectBeginUltra(p) VMProtectBeginUltra(p)
#define _VMProtectBeginVirtualizationLockByKey(p) VMProtectBeginVirtualizationLockByKey(p)
#define _VMProtectBeginUltraLockByKey(p) VMProtectBeginUltraLockByKey(p)
#define _VMProtectSetSerialNumber(p)	VMProtectSetSerialNumber(p)
#define _VMProtectEnd() VMProtectEnd()
#else
#define _VMProtectBegin(p)
#define _VMProtectBeginVirtualization(p)  
#define _VMProtectBeginMutation(p)
#define _VMProtectBeginUltra(p)
#define _VMProtectBeginVirtualizationLockByKey(p)
#define _VMProtectBeginUltraLockByKey(p)
#define _VMProtectSetSerialNumber(p)
#define _VMProtectEnd()
#endif // VMP_CODE
#endif //__VM_SDK_H__