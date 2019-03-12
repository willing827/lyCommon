/*-------------------------------------------------------------------------*/
/*  squeue.h																 */
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
#ifndef SQUEUE_H
#define SQUEUE_H

#include "sqtypes.h"
#include "sqindex.h"
#include <mutex>

typedef int32 (FUNTYPE_QueueCompare )(void * src, void * dest);


namespace snqu {namespace common{

//
// 动态链表
//
template <class T>
class SQQueue
{
public:
	struct QueueNode_t
	{
		T* m_data;
		QueueNode_t * m_next;
	};

	inline SQQueue(boolean is_del=TRUE)
	{
		m_count = 0;
		m_head = NULL;
		m_tail = NULL;
		m_is_del_data = is_del;
		m_fnCompare=NULL;
	}

	inline ~SQQueue()
	{
		clear();
	};

	inline int32 push_tail(T * value)
	{
		QueueNode_t * node= new QueueNode_t;
		if (NULL == node)
			return -1;
		node->m_next = NULL;
		node->m_data = value;
		if (m_head == NULL){//没有数据
			m_head = node;
			m_tail = node;
		}
		else{
			m_tail->m_next =node;
			m_tail = node;
		}
		m_count++;
		return m_count;
	};

	inline T * pop_header()
	{
		T* lstru;
		if (m_head == NULL)
			lstru = NULL;
		else{
			lstru = (T*)m_head->m_data;
			QueueNode_t * lpNode=NULL;
			lpNode = m_head->m_next; 
			delete m_head;
			m_head = lpNode;
			if (m_head == NULL)
				m_tail= NULL;
			m_count--;
		}
		return lstru;
	};
	inline T * get_header(){
		if (m_head == NULL)
			return NULL;
		return (T*)m_head->m_data;
	};

	inline boolean remove_header(){
		if (m_head == NULL)
			return false;
		else{
			QueueNode_t * lpNode=NULL;
			lpNode = m_head->m_next; 
			delete m_head;
			m_head = lpNode;
			if (m_head == NULL)
				m_tail= NULL;
			m_count--;
		}
		return true;
	};
	inline void clear(){
		QueueNode_t * lpNode=NULL;
		while (m_head != NULL){
			lpNode = m_head->m_next;
			if (true == m_is_del_data)
				delete (T*)m_head->m_data;
			delete m_head;
			m_head =lpNode;
		}
		m_tail=NULL;
		m_count = 0;
	}
	inline void clear(uint32 aiCount){
		QueueNode_t * lpNode=NULL;
		uint32 i=0;
		while (m_head != NULL){
			lpNode = m_head->m_next;
			if (true == m_is_del_data)
				delete (T*)m_head->m_data;
			delete m_head;
			m_head =lpNode;
			if (m_head == NULL)
				m_tail= NULL;
			i++;
			if (i == aiCount){
				break;
			}
		}
		m_count-=i;
	}
	inline int32 size(){return m_count;};

	inline int32 remove(T * value){
		QueueNode_t * new_node=NULL;
		QueueNode_t * old_node=NULL;
		new_node = m_head;
		old_node = new_node;
		while (  NULL != new_node){
			if (0 ==m_fnCompare(value,new_node->m_data))
			{
				m_count--;	
				if (new_node == m_head){//最前一个
					m_head=new_node->m_next;
					if (m_head == NULL)
						m_tail= NULL;
				}
				else if (new_node == m_tail){//最后一个
					m_tail=old_node;
					old_node->m_next=NULL;
					if (m_tail == NULL)
						m_head= NULL;
				}
				else
					old_node->m_next=new_node->m_next;

				if (true == m_is_del_data)
					delete (T*)new_node->m_data;
				delete  new_node;
				return 1;
			}
			old_node=new_node;
			new_node=new_node->m_next;
		}
		return 0;
	};

	inline int32 remove_by_index(int32 index)
	{
		int32 i=0;
		if (index>=m_count&&index<0)
			return -1;
		QueueNode_t * lpNode=NULL;
		QueueNode_t * lpOldNode=NULL;
		lpNode = m_head;
		lpOldNode = lpNode;
		while (  NULL != lpNode){
			if (index == i)
			{
				m_count--;	
				if (lpNode == m_head){//最前一个
					m_head=lpNode->m_next;
					lpOldNode=m_head;
					if (m_head == NULL)
						m_tail= NULL;
				}
				else if (lpNode == m_tail){//最后一个
					m_tail=lpOldNode;
					lpOldNode->m_next=NULL;
					if (m_tail == NULL)
						m_head= NULL;
				}
				else
					lpOldNode->m_next=lpNode->m_next;

				if (true == m_is_del_data)
					delete (T*)lpNode->m_data;
				delete  lpNode;
				return 1;
			}
			lpOldNode=lpNode;
			lpNode=lpNode->m_next;
			i++;
		}
		return 0;
	};

	inline QueueNode_t * get_head_node(){return m_head;}
	inline void set_compare_func(FUNTYPE_QueueCompare * value){m_fnCompare=value;}
private:
	uint32 m_count;
	QueueNode_t *m_head;
	QueueNode_t *m_tail;
	boolean m_is_del_data;
	FUNTYPE_QueueCompare *m_fnCompare; 
};


//
// 动态链表 线程安全
//
template <class T>
class SQSafeQueue
{
public:
	struct QueueNode_t
	{
		T* mpData;
		QueueNode_t * mpNext;
	};
	
	inline SQSafeQueue(boolean is_deldata=true)
	{
		miCount = 0;
		mpHead = NULL;
		mpTail = NULL;
		m_is_del_data=is_deldata;
	}
	
	inline ~SQSafeQueue()
	{
		clear();
	};

	inline void set_is_del_data(boolean is_deldata)
	{m_is_del_data = is_deldata;	};

	inline int32 push_tail(T * value){
		QueueNode_t * lpNode= new QueueNode_t;
		if (NULL == lpNode)
			return -1;
		lpNode->mpNext = NULL;
		lpNode->mpData = value;
		m_mutex_lock.lock();
		if (mpHead == NULL)	{//没有数据
			mpHead = lpNode;
			mpTail = lpNode;
		}
		else{
			mpTail->mpNext =lpNode;
			mpTail = lpNode;
		}
		miCount++;
		m_mutex_lock.unlock();
		return miCount;
	};
	inline T * pop_header(){
		T* lstru;
		m_mutex_lock.lock();
		if (mpHead == NULL)
			lstru = NULL;
		else{
			lstru = (T*)mpHead->mpData;
			QueueNode_t * lpNode=NULL;
			lpNode = mpHead->mpNext; 
			delete mpHead;
			mpHead = lpNode;
			if (mpHead == NULL)
				mpTail= NULL;
			miCount--;
		}
		m_mutex_lock.unlock();
		return lstru;
	};
	inline T * get_header(){
		T* lstru;
		m_mutex_lock.lock();
		if (mpHead == NULL)
			lstru = NULL;
		else
			lstru = (T*)mpHead->mpData;
		m_mutex_lock.unlock();
		return lstru;
	};

	inline boolean remove_header(){
		if (mpHead == NULL)
			return false;
		QueueNode_t * lpNode=NULL;
		m_mutex_lock.lock();
		lpNode = mpHead->mpNext; 
		delete mpHead;
		mpHead = lpNode;
		if (mpHead == NULL)
			mpTail= NULL;
		miCount--;
		m_mutex_lock.unlock();
		return true;
	};
	inline void clear(){
		QueueNode_t * lpNode=NULL;
		m_mutex_lock.lock();
		while (mpHead != NULL){
			lpNode = mpHead->mpNext;
			if (true == m_is_del_data)
				delete (T*)mpHead->mpData;
			delete mpHead;
			mpHead =lpNode;
		}
		mpTail=NULL;
		miCount = 0;
		m_mutex_lock.unlock();
	}
	inline void clear(uint32 aiCount){
		QueueNode_t * lpNode=NULL;
		uint32 i=0;
		m_mutex_lock.lock();
		while (mpHead != NULL){
			lpNode = mpHead->mpNext;
			if (true == m_is_del_data)
				delete (T*)mpHead->mpData;
			delete mpHead;
			mpHead =lpNode;
			if (mpHead == NULL)
				mpTail= NULL;
			i++;
			if (i == aiCount){
				break;
			}
		}
		miCount-=i;
		m_mutex_lock.unlock();
	}
	inline int32 size(){return miCount;};
private:
	uint32 miCount;
	mutex		m_mutex_lock;	   
	QueueNode_t *mpHead;
	QueueNode_t *mpTail;
	boolean m_is_del_data;
};


//
// 静态数组模拟动态链表,方式
//
template<class T>
class SQStaticsQueue 
{
public:
	SQStaticsQueue()
	{
		init_member();
		mpDataList=NULL;
	};
	SQStaticsQueue(int32 aiMaxCount)
	{
		init_member();
		mpDataList=NULL;
		miMaxCount=aiMaxCount;
		mpDataList= new T*[miMaxCount];
		if(NULL == mpDataList)
			throw;
		memset(mpDataList,0,miMaxCount*sizeof(T*));
	};
	~SQStaticsQueue()
	{
		clear();
		if(NULL != mpDataList)
		{
			delete [] mpDataList;
			mpDataList=NULL;
		}
	}
	inline void init(int32 aiMaxCount)
	{
		clear();
		if(NULL != mpDataList)
		{
			delete [] mpDataList;
			mpDataList=NULL;
		}
		miMaxCount=aiMaxCount;
		mpDataList= new T*[miMaxCount];
		if(NULL == mpDataList)
		{
			throw;
		}
		memset(mpDataList,0,miMaxCount*sizeof(T*));

	}
	inline int32 size(){return miCount;}
	inline void init_member()
	{
		mbIsFrist=TRUE;
		miCount=0;
		miHeadIndex=0;
	}
	inline void clear()
	{
		if(NULL ==mpDataList)
			return ;
		int32 liIndex=miHeadIndex;
		T* lpValue=NULL;
		for(int32 i=0;i<miCount;i++)
		{
			lpValue	=mpDataList[liIndex];
			if(NULL != lpValue)
				delete lpValue;
			liIndex = INC_CIRCLE_INDEX(liIndex,1,miMaxCount);
		}
		memset(mpDataList,0,miMaxCount*sizeof(T*));		
		init_member();

	};

	inline T * pop_header()
	{
		T *lpValue =NULL;
		if(miCount>0)
		{
			lpValue = mpDataList[miHeadIndex];
			mpDataList[miHeadIndex]=NULL;
			miCount--;
			miHeadIndex = INC_CIRCLE_INDEX(miHeadIndex,1,miMaxCount);
		}
		return lpValue;
	};
	inline T * get_header()
	{
		T *lpValue =NULL;
		if(miCount>0)
			lpValue = mpDataList[miHeadIndex];
		return lpValue;
	};
	inline boolean push_tail(T *apValue)
	{
		if (miCount < miMaxCount)
		{
			mpDataList[INC_CIRCLE_INDEX(miHeadIndex,miCount,miMaxCount)]=apValue;
			miCount++;
			return TRUE;
		}
		return FALSE;
	}

protected:
	int32		miMaxCount;
	int32		miCount;
	T		**mpDataList;

	int32	miHeadIndex;
	boolean	mbIsFrist;
};


// 
// cache列表 静态列表方式 线程安全
//
template<class T>
class SQSafeStaticQueue
{
public:
	SQSafeStaticQueue()
	{
		init_member();
		m_data_list=NULL;
	}

	SQSafeStaticQueue(int32 max_count)
	{
		init_member();
		m_data_list=NULL;
		m_max_count=max_count;
		m_data_list= new T*[m_max_count];
		if(NULL == m_data_list)
			throw;
		memset(m_data_list,0,m_max_count*sizeof(T*));
	}


	void init(int32 max_count)
	{
		clear();
		m_mutex_lock.lock();
		
		if(NULL != m_data_list)
		{
			delete [] m_data_list;
			m_data_list=NULL;
		}
		m_max_count=max_count;
		m_data_list= new T*[m_max_count];
		if(NULL == m_data_list)
		{
			m_mutex_lock.unlock();
			throw;
		}
		memset(m_data_list,0,m_max_count*sizeof(T*));
		m_mutex_lock.unlock();
	}


	~SQSafeStaticQueue()
	{
		clear();
		if(NULL != m_data_list)
		{
			delete [] m_data_list;
			m_data_list=NULL;
		}
	}
	
	inline int32 size(){return m_count;}
	
	inline void init_member()
	{
		m_is_first=TRUE;
		m_count=0;
		m_head_index=0;
	}


	inline void clear()
	{
		m_mutex_lock.lock();
		if(NULL ==m_data_list)
		{
			m_mutex_lock.unlock();
			return ;
		}
		int32 head_index=m_head_index;
		T* lpValue=NULL;
		for(int32 i=0;i<m_count;i++)
		{
			lpValue	=m_data_list[head_index];
			if(NULL != lpValue)
				delete lpValue;
			head_index = INC_CIRCLE_INDEX(head_index,1,m_max_count);
		}
		memset(m_data_list,0,m_max_count*sizeof(T*));			
		init_member();
		m_mutex_lock.unlock();
	}

	inline T * pop_header()
	{
		T *lpValue =NULL;
		m_mutex_lock.lock();
		if(m_count>0)
		{
			lpValue = m_data_list[m_head_index];
			m_data_list[m_head_index]=NULL;
			m_count--;
			//miHeadIndex = (miHeadIndex+1)%miMaxCount;
			m_head_index = INC_CIRCLE_INDEX(m_head_index,1,m_max_count);
		}
		m_mutex_lock.unlock();

		return lpValue;
	}

	inline T * pop()
	{
		T *lpValue =NULL;
		m_mutex_lock.lock();
		if(m_count>0)
		{
			lpValue = m_data_list[m_head_index];
		}
		m_mutex_lock.unlock();

		return lpValue;
	}

	inline boolean push_tail(T *value)
	{
		if(NULL == value)
			return FALSE;
		m_mutex_lock.lock();
		if (m_count < m_max_count)
		{
			m_data_list[INC_CIRCLE_INDEX(m_head_index,m_count,m_max_count)]=value;
			m_count++;
			m_mutex_lock.unlock();
			return TRUE;
		}
		m_mutex_lock.unlock();	
		return FALSE;
	}

private:
	inline SQSafeStaticQueue<T>& SQSafeStaticQueue<T>::operator =(const SQSafeStaticQueue<T> &)
	{
		return *this;
	}

private:
	mutex	m_mutex_lock;
	
protected:
	int32	m_max_count;
	int32	m_count;
	T		**m_data_list;
	int32	m_head_index;
	boolean	m_is_first;
};


//
// 静态数组模拟动态链表,T一般是WORD,int32,不能是带虚函数的类，因为要memset
//
class SQStaticIndexQueue
{
public:
	SQStaticIndexQueue()
	{
		init_member();
		mpDataList=NULL;
	};

	SQStaticIndexQueue(uint32 aiMaxCount)
	{
		init_member();
		mpDataList=NULL;
		miMaxCount=aiMaxCount;
		mpDataList= new int32[miMaxCount];
		if(NULL == mpDataList)
			throw;
		memset(mpDataList,0,miMaxCount*sizeof(int32));
	};
	~SQStaticIndexQueue()
	{
		clear();
		if(NULL != mpDataList)
		{
			delete [] mpDataList;
			mpDataList=NULL;
		}
	}
	inline void init(uint32 aiMaxCount)
	{
		clear();
		if(NULL != mpDataList)
		{
			delete [] mpDataList;
			mpDataList=NULL;
		}
		miMaxCount=aiMaxCount;
		mpDataList= new int32[miMaxCount];
		if(NULL == mpDataList)
		{
			throw;
		}
		memset(mpDataList,0,miMaxCount*sizeof(int32));

	}
	inline uint32 size(){return miCount;}
	inline void init_member()
	{
		mbIsFrist=TRUE;
		miCount=0;
		miHeadIndex=0;
	}
	inline void clear()
	{
		if(NULL ==mpDataList)
			return ;
		//		int32 liIndex=miHeadIndex;
		memset(mpDataList,0,miMaxCount*sizeof(int32));		
		init_member();
	};

	inline int32 pop_header()
	{
		int32 liValue=-1;
		if(miCount>0)
		{
			liValue = mpDataList[miHeadIndex];
			miCount--;
			miHeadIndex = INC_CIRCLE_INDEX(miHeadIndex,1,miMaxCount);
		}
		return liValue;
	};
	inline int32 get_header()
	{
		int32 liValue=-1;
		if(miCount>0)
			liValue = mpDataList[miHeadIndex];
		return liValue;
	};
	inline boolean push_tail(int32 aiValue)
	{
		if (miCount < miMaxCount)
		{
			mpDataList[INC_CIRCLE_INDEX(miHeadIndex,miCount,miMaxCount)]=aiValue;
			miCount++;
			return TRUE;
		}
		return FALSE;
	}

protected:
	uint32		miMaxCount;
	uint32		miCount;
	int32		 *mpDataList;

	uint32	miHeadIndex;//头
	boolean	mbIsFrist;
};


class SQSafeStaticIndexQueue
{
public:
	SQSafeStaticIndexQueue()
	{
		init_member();
		m_data_list=NULL;
	};

	SQSafeStaticIndexQueue(uint32 max_count)
	{
		init_member();
		m_data_list=NULL;
		m_max_count=max_count;
		m_data_list= new int32[m_max_count];
		if(NULL == m_data_list)
			throw;
		memset(m_data_list,0,m_max_count*sizeof(int32));
	};

	~SQSafeStaticIndexQueue()
	{
		clear();
		if(NULL != m_data_list)
		{
			delete [] m_data_list;
			m_data_list=NULL;
		}
	}

	inline void init(uint32 max_count)
	{
		m_mutex_lock.lock();
		clear();
		if(NULL != m_data_list)
		{
			delete [] m_data_list;
			m_data_list=NULL;
		}
		m_max_count=max_count;
		m_data_list= new int32[m_max_count];
		if(NULL == m_data_list)
		{
			m_mutex_lock.unlock();
			throw;
		}
		memset(m_data_list,0,m_max_count*sizeof(int32));
		m_mutex_lock.unlock();

	}

	inline uint32 size(){return m_count;}
	inline void init_member()
	{
		m_is_first=TRUE;
		m_count=0;
		m_head_index=0;
	}

	inline void clear()
	{
		m_mutex_lock.lock();
		if(NULL ==m_data_list)
		{
			m_mutex_lock.unlock();
			return;
		}
		
		memset(m_data_list,0,m_max_count*sizeof(int32));		
		init_member();
		m_mutex_lock.unlock();
	};

	inline int32 pop_header()
	{
		int32 value=-1;
		m_mutex_lock.lock();
		if(m_count>0)
		{
			value = m_data_list[m_head_index];
			m_count--;
			m_head_index = INC_CIRCLE_INDEX(m_head_index,1,m_max_count);
		}
		m_mutex_lock.unlock();
		return value;
	};
	
	inline int32 get_header()
	{
		int32 value=-1;
		m_mutex_lock.lock();
		if(m_count>0)
			value = m_data_list[m_head_index];
		m_mutex_lock.unlock();
		return value;
	};

	inline boolean push_tail(int32 value)
	{
		m_mutex_lock.lock();
		if (m_count < m_max_count)
		{
			m_data_list[INC_CIRCLE_INDEX(m_head_index,m_count,m_max_count)]=value;
			m_count++;
			m_mutex_lock.unlock();
			return TRUE;
		}
		m_mutex_lock.unlock();
		return FALSE;
	}

protected:
	uint32	m_max_count;
	uint32	m_count;
	int32	*m_data_list;
	mutex   m_mutex_lock;
	uint32	m_head_index;
	boolean	m_is_first;
};
}}
#endif //SQUEUE_H