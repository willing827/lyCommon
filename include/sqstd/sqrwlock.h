#ifndef _RW_LOCK_
#define _RW_LOCK_

#include <cstdlib>
#include <cassert>
#include <atomic>
#include <thread>
#include "sqraii.h"

namespace snqu {

/*
 * atomicʵ�ֶ�д��Դ��,��ռд,�����,��ֹ���ƹ��캯����'='��ֵ������
 * WRITE_FIRSTΪtrueʱΪд����ģʽ,������̵߳ȴ���ȡ(m_writeWaitCount>0)��ȴ�,������д�߳��Ȼ�ȡ��
 * ����Ƕ�׼���
 * readLock/Unlock ʵ�ֹ���Ķ�ȡ��/�������߳�������
 * writeLock/Unlock ʵ�ֶ�ռ��д���/����,ͬʱֻ����һ���߳�д�룬
 * �����߳��ڶ�ȡʱ��д���߳���������д���߳�ִ��ʱ�����еĶ�ȡ�̶߳���������
 */

class RWLock 
{

public:
    /* ��ʼΪ0���߳�id */
    static const  std::thread::id NULL_THEAD;
    const bool WRITE_FIRST;
    /* �����жϵ�ǰ�Ƿ���д�߳� */
    std::thread::id m_write_thread_id;
    /* ��Դ��������,����Ϊint��ԭ�ӳ�Ա����,-1Ϊд״̬��0Ϊ����״̬,>0Ϊ�����ȡ״̬ */
    std::atomic_int m_lockCount;
    /* �ȴ�д�̼߳�����,����Ϊunsigned int��ԭ�ӳ�Ա����*/
    std::atomic_uint m_writeWaitCount;

private:
    // ��ֹ���ƹ��캯��
    RWLock(const RWLock&);
    // ��ֹ����ֵ������
    RWLock& operator=(const RWLock&);

public:
#define WRITE_LOCK_STATUS -1
#define FREE_STATUS 0
    RWLock(bool writeFirst = false);//Ĭ��Ϊ������ģʽ
    virtual ~RWLock() {};

    int readLock();
    int readUnlock();
    int writeLock();
    int writeUnlock();

    // ����ȡ����������ͷŶ�����װΪraii�����Զ���ɼ����ͽ�������
    raii read_guard() 
    {
        return raii(std::bind(&RWLock::readUnlock, this), std::bind(&RWLock::readLock, this));
    }

    // ��д������������ͷŶ�����װΪraii�����Զ���ɼ����ͽ�������
    raii write_guard() 
    {
        return raii(std::bind(&RWLock::writeUnlock, this), std::bind(&RWLock::writeLock, this));
    }
};

}

#endif//_RW_LOCK_