/*-------------------------------------------------------------------------*/
/*  sqassert.h																 */
/*                                                                           */
/*  History                                                                  */
/*      04/21/2015															 */
/*                                                                           */
/*  Author                                                                   */
/*      Guolei                                                               */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*-------------------------------------------------------------------------*/

#ifndef __JFASSERT_H__
#define __JFASSERT_H__


#if defined(_DEBUG)
#include <assert.h>
#define SQ_ASSERT(x)	assert(x)
#define SQ_ASSERTX(x)	assert(x)
#else
#define SQ_ASSERT(x)
#define SQ_ASSERTX(x)
#endif

#ifdef SHARING_CORE
#undef SQ_ASSERT
#define SQ_ASSERT(x)	
#undef SQ_ASSERTX
#define SQ_ASSERTX(x)	
#endif

#endif //__JFASSERT_H__