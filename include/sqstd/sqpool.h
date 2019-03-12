/*-------------------------------------------------------------------------*/
/*  sqpool.h																 */
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
#ifndef SQPOOL_H
#define SQPOOL_H

#include "sqtypes.h"
#include "sqindex.h"
#include "squeue.h"


namespace snqu {namespace common{
template<typename T>
class SQCachePool : private SQSafeStaticQueue<T>
{
public:
	inline void init(int max_count)
	{
		SQSafeStaticQueue<T>::init(max_count);
	}

	inline T* malloc()
	{
		T* item = SQSafeStaticQueue<T>::pop_header();
		if (NULL == item)
			item = new T;
		return item;
	}

	inline void free(T *p)
	{
		if(1 != SQSafeStaticQueue<T>::push_tail(p))
			delete p;
	}
	
	inline int32 size()
	{
		return SQSafeStaticQueue<T>::size();
	}
	
	void clear()
	{
		SQSafeStaticQueue<T>::clear();
	}
};
}}
#endif //SQPOOL_H