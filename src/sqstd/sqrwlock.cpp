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
        // ==时为独占写状态,不需要加锁
        if (std::this_thread::get_id() != this->m_write_thread_id) 
        {
            int count;
            if (WRITE_FIRST)//写优先模式下,要检测等待写的线程数为0(m_writeWaitCount==0)
            {
                do 
                {
                    while ((count = m_lockCount) == WRITE_LOCK_STATUS || m_writeWaitCount > 0);//写锁定时等待
                } while (!m_lockCount.compare_exchange_weak(count, count + 1));
            }
            else
            {
                do 
                {
                    while ((count = m_lockCount) == WRITE_LOCK_STATUS); //写锁定时等待
                } while (!m_lockCount.compare_exchange_weak(count, count + 1));
            }
        }
        return m_lockCount;
    }

    int RWLock::readUnlock() 
    {
        // ==时为独占写状态,不需要加锁
        if (std::this_thread::get_id() != this->m_write_thread_id)
            --m_lockCount;
        return m_lockCount;
    }
    int RWLock::writeLock()
    {
        // ==时为独占写状态,避免重复加锁
        if (std::this_thread::get_id() != this->m_write_thread_id)
        {
            ++m_writeWaitCount;//写等待计数器加1
            // 没有线程读取时(加锁计数器为0)，置为-1加写入锁，否则等待
            for(int zero=FREE_STATUS; !this->m_lockCount.compare_exchange_weak(zero, WRITE_LOCK_STATUS); zero=FREE_STATUS);
            --m_writeWaitCount;//获取锁后,计数器减1
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

