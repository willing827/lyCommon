/*-------------------------------------------------------------------------*/
/*  sqindex.h																 */
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
#ifndef SQINDEX_H
#define SQINDEX_H

#include "sqtypes.h"


using namespace snqu;
inline int32 INC_CIRCLE_INDEX(int32 index, int32 count, int32 size)
{
	return (index+count)%size;
}


inline int32 DEC_CIRCLE_INDEX(int32 index,int32 count,int32 size)
{
	int32 temp=(index+size);
	return (int32)(temp-count)%size;
}


inline int32 GetCircleIndexCount(int32 start_index,int32 end_index,int32 size)
{
	int32 len = 0;
	if(end_index>=start_index)//环形索引定义。重新计算偏移
		len=end_index-start_index+1;
	else
		len=size-start_index+end_index+1;
	return len;
}


inline int32 GetCircleIndexDistance(int32 start_index,int32 end_index,int32 size)
{
	int32 len =0;
	if(end_index>=start_index)//环形索引定义。重新计算偏移
		len=end_index-start_index;
	else
		len=size-start_index+end_index;
	return len;
}


inline boolean IsCircleIndexRange(int32 index, int32 start_index, int32 end_index, int32 size)
{
	return (boolean)(GetCircleIndexCount(start_index, index, size)
		<= GetCircleIndexCount(start_index, end_index, size));
}


inline boolean IsCircleIndexRangebySize(int32 index, int32 start_index, int32 len, int32 size)
{
	return (boolean)(GetCircleIndexCount(start_index, index, size)<= len);
}
#endif //SQINDEX_H