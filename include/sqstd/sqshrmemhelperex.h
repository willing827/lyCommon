
/*  sqshrmemhelper.h																*/
/*	A shared memory class                                                   */
/*  History                                                                 */
/*      07/06/2015															*/
/*                                                                          */
/*  Copyright (C) 2015 by SNQU network technology Inc.                      */
/*  All rights reserved                                                     */
/*--------------------------------------------------------------------------*/
#ifndef __SQSHRMEMHELPER_H__
#define __SQSHRMEMHELPER_H__

#include <sqstd/sqshrmem.h>
#include <sqevent.h>
#include <sqstd/winsechelper.h>
#include <sqstd/sqsignal.h>
#include <sqstd/sqmutex.h>

namespace snqu { namespace common { 
	typedef std::shared_ptr<SQSharedMemSvr> SQSharedMemSvrPtr;
	typedef std::shared_ptr<SQSharedMemUser> SQSharedMemUserPtr;
    typedef std::shared_ptr<SQSharedMem> SQSharedMemPtr;
	
	template  <typename _T>
	class SQSharedMemHelperex
	{
	public:
		inline SQSharedMemHelperex()
			: m_master_data(nullptr)
			, m_stop(false), m_client_data(nullptr)
		{
		}

		inline virtual ~SQSharedMemHelperex() {};

	protected:
		virtual void on_notify() = 0;
        void init_strs(const std::string& _MemName)
        {
            m_master_mem = fmt::Format("Global\\{0}_M_MEM", _MemName);
            m_client_mem = fmt::Format("Global\\{0}_C_MEM", _MemName);
            m_mater_signal_string = fmt::Format("Global\\{0}_M_SIGNAL", _MemName);
            m_client_signal_string = fmt::Format("Global\\{0}_C_SIGNAL", _MemName);
            m_mater_mutex_string = fmt::Format("Global\\{0}_M_MUTEX", _MemName);
            m_client_mutex_string = fmt::Format("Global\\{0}_C_MUTEX", _MemName);
        }

	public:
		inline bool create(bool is_master, string _MemName)
		{
            m_is_master = is_master;
			init_strs(_MemName);

			bool created = false;
			if (m_is_master)
            {
                created = open_memory();
                if (!created)
				    created = create_memory();
            }
			else
				created = open_memory();
			
			if (created)
			{
				m_thread = std::thread(std::bind(&SQSharedMemHelperex<_T>::notify_proc, this));
			}

			return created;
		}

		void destory()
		{
            m_master_data = NULL;
            m_client_data = NULL;
			m_stop = true;
            m_client_signal.release();
            m_master_signal.release();
            m_master_mutex.release();
            m_client_mutex.release();
            m_thread.join();

			if (m_is_master)
			{
				if (m_master != nullptr)
					m_master->destory();
			}
			else
			{
				if (m_client != nullptr)
					m_client->destory();
			}
		}

		inline _T* get_data()
		{
            if (m_is_master)
			    return m_master_data;
            return m_client_data;
		}

		inline void notify_proc()
		{
			while (!m_stop)
			{
                ::Sleep(10);
				if (m_is_master)
				{
                    if (0 == m_master_signal.wait())
                    {
                        if (m_stop) break;
                        m_master_mutex.wait();
                        on_notify();
                        m_master_mutex.release();
                    }
				}
                else
                {
                    if (0 == m_client_signal.wait())
                    {
                        if (m_stop) break;
                        m_client_mutex.wait();
                        on_notify();
                        m_client_mutex.release();
                    }
                }
			}
		}

		inline bool notify()
		{
            if (m_is_master)
            {
                m_client_signal.release();
            }
            else
            {
                m_master_signal.release();
            }
            return true;
		}

    private:
        bool open_memory()
        {
            m_master = std::make_shared<SQSharedMemUser>();
            if (m_master != nullptr)
            {
                if(!m_master->open(FILE_MAP_READ|FILE_MAP_WRITE, m_master_mem.c_str()))
                    return false;

                m_master_data = static_cast<_T*>(m_master->get_buffer());
                if (NULL == m_master_data)
                    return false;
            }

            m_client = std::make_shared<SQSharedMemUser>();
            if (m_client != nullptr)
            {
                if(!m_client->open(FILE_MAP_READ|FILE_MAP_WRITE, m_client_mem.c_str()))
                    return false;

                m_client_data = static_cast<_T*>(m_client->get_buffer());
                if (NULL == m_client_data)
                    return false;
            }

            if (!m_master_signal.open(m_mater_signal_string.c_str()))
            {
                auto err = ::GetLastError();
                return false;
            }

            if (!m_client_signal.open(m_client_signal_string.c_str()))
            {
                auto err = ::GetLastError();
                return false;
            }

            if (!m_master_mutex.open(MUTEX_ALL_ACCESS, m_mater_mutex_string.c_str()))
            {
                auto err = ::GetLastError();
                return false;
            }

            if (!m_client_mutex.open(MUTEX_ALL_ACCESS, m_client_mutex_string.c_str()))
            {
                auto err = ::GetLastError();
                return false;
            }

            return true;
        }

        bool create_memory()
        {
            m_master = std::make_shared<SQSharedMemSvr>();
            if (m_master != nullptr)
            {
                if(!m_master->create(NULL, m_master_mem.c_str(), sizeof(_T)))
                    return false;

                m_master_data = static_cast<_T*>(m_master->get_buffer());
                if (NULL == m_master_data)
                    return false;
            }

            m_client = std::make_shared<SQSharedMemSvr>();
            if (m_client != nullptr)
            {
                if(!m_client->create(NULL, m_client_mem.c_str(), sizeof(_T)))
                    return false;

                m_client_data = static_cast<_T*>(m_client->get_buffer());
                if (NULL == m_client_data)
                    return false;
            }

            bool event_ok = true;
            SECURITY_ATTRIBUTES sa; //允许管理员访问
            sa.nLength=sizeof(SECURITY_ATTRIBUTES);
            sa.bInheritHandle=FALSE;
            LPCSTR szSD= "D:P(D;OICI;GA;;;BG)(A;OICI;GA;;;SY)(A;OICI;GA;;;BA)(A;OICI;GRGWGX;;;IU)";
            ConvertStringSecurityDescriptorToSecurityDescriptorA(szSD,SDDL_REVISION_1,&(
                sa.lpSecurityDescriptor),NULL);

            if (!m_master_signal.create(m_mater_signal_string.c_str()))
            {
                auto err = ::GetLastError();
                return false;
            }

            if (!m_client_signal.create(m_client_signal_string.c_str()))
            {
                auto err = ::GetLastError();
                return false;
            }

            if (!m_master_mutex.create(m_mater_mutex_string.c_str()))
            {
                if (!m_master_mutex.open(MUTEX_ALL_ACCESS, m_mater_mutex_string.c_str()))
                {
                    auto err = ::GetLastError();
                    return false;
                }
            }

            if (!m_client_mutex.create(m_client_mutex_string.c_str()))
            {
                if (!m_client_mutex.open(MUTEX_ALL_ACCESS, m_mater_mutex_string.c_str()))
                {
                    auto err = ::GetLastError();
                    return false;
                }
            }

            LocalFree(sa.lpSecurityDescriptor);
            if (!event_ok)
                return false;

            return true;
        }

    protected:
        _T* m_master_data;
        _T* m_client_data;
        bool m_is_master;
        Signal m_master_signal;
        Signal m_client_signal;
        Mutex m_master_mutex;
        Mutex m_client_mutex;

    private:
        bool m_stop;
        SQSharedMemPtr  m_master;
        SQSharedMemPtr  m_client;

        string m_master_mem;
        string m_client_mem;
        string m_mater_signal_string;
        string m_client_signal_string;
        string m_mater_mutex_string;
        string m_client_mutex_string;
        std::thread m_thread;
	};
}}
#endif //__SQSHRMEMHELPER_H__