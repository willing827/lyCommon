#include <sqstd/sqrwlock.h>
#include <stdexcept>
#include <thread>

namespace snqu{

    RWLock::RWLock(bool writeFirst)
        : WRITE_FIRST(writeFirst)
        , m_write_thread_id()
    {
        m_lockCount = 0;
        m_writeWaitCount = 0;
    }

    int RWLock::readLock() 
    {
        // ==ʱΪ��ռд״̬,����Ҫ����
        if (std::this_thread::get_id() != this->m_write_thread_id) 
        {
            int count;
            if (WRITE_FIRST)//д����ģʽ��,Ҫ���ȴ�д���߳���Ϊ0(m_writeWaitCount==0)
            {
                do 
                {
                    while ((count = m_lockCount) == WRITE_LOCK_STATUS || m_writeWaitCount > 0);//д����ʱ�ȴ�
                } while (!m_lockCount.compare_exchange_weak(count, count + 1));
            }
            else
            {
                do 
                {
                    while ((count = m_lockCount) == WRITE_LOCK_STATUS); //д����ʱ�ȴ�
                } while (!m_lockCount.compare_exchange_weak(count, count + 1));
            }
        }
        return m_lockCount;
    }

    int RWLock::readUnlock() 
    {
        // ==ʱΪ��ռд״̬,����Ҫ����
        if (std::this_thread::get_id() != this->m_write_thread_id)
            --m_lockCount;
        return m_lockCount;
    }
    int RWLock::writeLock()
    {
        // ==ʱΪ��ռд״̬,�����ظ�����
        if (std::this_thread::get_id() != this->m_write_thread_id)
        {
            ++m_writeWaitCount;//д�ȴ���������1
            // û���̶߳�ȡʱ(����������Ϊ0)����Ϊ-1��д����������ȴ�
            for(int zero=FREE_STATUS; !this->m_lockCount.compare_exchange_weak(zero, WRITE_LOCK_STATUS); zero=FREE_STATUS);
            --m_writeWaitCount;//��ȡ����,��������1
            m_write_thread_id = std::this_thread::get_id();
        }
        return m_lockCount;
    }
    int RWLock::writeUnlock()
    {
        if(std::this_thread::get_id() != this->m_write_thread_id)
        {
            throw std::runtime_error("writeLock/Unlock mismatch");
        }
        assert(WRITE_LOCK_STATUS==m_lockCount);
        m_write_thread_id=NULL_THEAD;
        m_lockCount.store(FREE_STATUS);
        return m_lockCount;
    }

    const std::thread::id RWLock::NULL_THEAD;

}

