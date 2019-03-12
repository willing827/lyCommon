
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

namespace snqu { namespace common { 
	typedef std::shared_ptr<SQSharedMemSvr> SQSharedMemSvrPtr;
	typedef std::shared_ptr<SQSharedMemUser> SQSharedMemUserPtr;
	
	template  <typename _T>
	class SQSharedMemHelper
	{
	public:
		inline SQSharedMemHelper()
			: m_data(nullptr)
			, m_stop(false)
		{
		}

		inline virtual ~SQSharedMemHelper() {};

	protected:
		virtual void on_notify() = 0;

	public:
        bool createex(string _MemName)
        {
            m_mem_string = fmt::Format("Global\\{0}", _MemName);

            m_usr_event_read_string = fmt::Format("Global\\{0}READ_USR", _MemName);
            m_usr_event_write_string = fmt::Format("Global\\{0}WRITE_USR", _MemName);
            m_svr_event_read_string = fmt::Format("Global\\{0}READ_SVR", _MemName);
            m_svr_event_write_string = fmt::Format("Global\\{0}WRITE_SVR", _MemName);

            bool created;

            if (open_memory())
            {
                m_is_master = false;
                created = true;
            }
            else
            {
                m_is_master = true;
                created = create_memory();
            }

            if (created)
            {
                m_thread = std::thread(std::bind(&SQSharedMemHelper<_T>::notify_proc, this));
            }

            return created;
        }

		inline bool create(bool is_master, string _MemName)
		{
            m_is_master = is_master;
			m_mem_string = fmt::Format("Global\\{0}", _MemName);

            m_usr_event_read_string = fmt::Format("Global\\{0}READ_USR", _MemName);
            m_usr_event_write_string = fmt::Format("Global\\{0}WRITE_USR", _MemName);
            m_svr_event_read_string = fmt::Format("Global\\{0}READ_SVR", _MemName);
            m_svr_event_write_string = fmt::Format("Global\\{0}WRITE_SVR", _MemName);

			bool created = false;
			if (m_is_master)
			{
				created = create_memory();
			}
			else
			{
				created = open_memory();
			}
			
			if (created)
			{
				m_thread = std::thread(std::bind(&SQSharedMemHelper<_T>::notify_proc, this));
			}

			return created;
		}

		void destory()
		{
            m_data = NULL;
			m_stop = true;
			m_event_svr_read.set_event();
			m_event_usr_read.set_event();
            m_thread.join();

			if (m_is_master)
			{
                m_event_usr_write.destory();
                m_event_usr_read.destory();
                m_event_svr_write.destory();
                m_event_svr_read.destory();

				if (m_server != nullptr)
					m_server->destory();
			}
			else
			{
				if (m_user != nullptr)
					m_user->destory();
			}
		}

		inline _T* get_data()
		{
			return m_data;
		}

		inline void notify_proc()
		{
			while (!m_stop)
			{
				if (m_is_master)
				{
                    if (m_event_svr_read.wait(INFINITE))
                    {
                        if (m_stop)
                            break;
                        m_event_svr_read.reset_event();
                        on_notify();
                        m_event_svr_write.set_event();
                    }
				}
                else
                {
                    if (m_event_usr_read.wait(INFINITE))
                    {
                        if (m_stop)
                            break;

                        bool set;
                        set = m_event_usr_read.reset_event();
                        on_notify();
                        set = m_event_usr_write.set_event();
                    }
                    else
                    {
                        bool set = m_event_usr_read.reset_event();
                    }
                }
			}
		}

		inline bool notify()
		{
            if (m_is_master)
            {
                if(!m_event_usr_read.set_event())
                    return false;
                //if(!m_event_usr_write.wait(INFINITE))
				if(!m_event_usr_write.wait(5*1000)) // 设置为5秒超时，防止两个进程中，有一个挂掉后，会影响另外一个正常运行
                    return false;
                return m_event_usr_write.reset_event();
            }

            if (!m_event_svr_read.set_event())
                return false;
            //if(!m_event_usr_write.wait(INFINITE))
            if(!m_event_svr_write.wait(5*1000))
                return false;
			return m_event_svr_write.reset_event();
		}

	protected:
		_T* m_data;
        bool m_is_master;

	private:
		bool m_stop;
		SQSharedMemSvrPtr m_server;
		SQSharedMemUserPtr m_user;
		
        Event m_event_usr_write;
		Event m_event_usr_read;
        Event m_event_svr_write;
        Event m_event_svr_read;

		string m_mem_string;
		string m_usr_event_read_string;
        string m_usr_event_write_string;
        string m_svr_event_read_string;
        string m_svr_event_write_string;
        std::thread m_thread;

    private:
        bool open_memory()
        {
            m_user = std::make_shared<SQSharedMemUser>();
            if (m_user != nullptr)
            {
                if(!m_user->open(FILE_MAP_READ|FILE_MAP_WRITE, m_mem_string.c_str()))
                    return false;

                m_data = static_cast<_T*>(m_user->get_buffer());
                if (NULL == m_data)
                    return false;
            }

            if (!m_event_usr_read.open(EVENT_ALL_ACCESS, m_usr_event_read_string.c_str()))
            {
                auto err = ::GetLastError();
                return false;
            }
            else
                m_event_usr_read.reset_event();

            if (!m_event_usr_write.open(EVENT_ALL_ACCESS, m_usr_event_write_string.c_str()))
            {
                auto err = ::GetLastError();
                return false;
            }
            else
                m_event_usr_write.set_event();

            if (!m_event_svr_read.open(EVENT_ALL_ACCESS, m_svr_event_read_string.c_str()))
            {
                auto err = ::GetLastError();
                return false;
            }
            else
                m_event_svr_read.set_event();

            if (!m_event_svr_write.open(EVENT_ALL_ACCESS, m_svr_event_write_string.c_str()))
            {
                auto err = ::GetLastError();
                return false;
            }
            else
                m_event_svr_write.reset_event();
            return true;
        }

        bool create_memory()
        {
            m_server = std::make_shared<SQSharedMemSvr>();
            if (m_server != nullptr)
            {
                if(!m_server->create(NULL, m_mem_string.c_str(), sizeof(_T)))
                    return false;

                m_data = static_cast<_T*>(m_server->get_buffer());
                if (NULL == m_data)
                    return false;
            }

            bool event_ok = true;
            SECURITY_ATTRIBUTES sa; //允许管理员访问
            sa.nLength=sizeof(SECURITY_ATTRIBUTES);
            sa.bInheritHandle=FALSE;
            LPCSTR szSD= "D:P"
                "(D;OICI;GA;;;BG)"
                "(A;OICI;GA;;;SY)"
                "(A;OICI;GA;;;BA)"
                "(A;OICI;GRGWGX;;;IU)";

            ConvertStringSecurityDescriptorToSecurityDescriptorA(szSD,SDDL_REVISION_1,&(
                sa.lpSecurityDescriptor),NULL);

            if (!m_event_usr_read.create(&sa, true, false, m_usr_event_read_string.c_str()))
                event_ok = false;

            if (!m_event_usr_write.create(&sa, true, false, m_usr_event_write_string.c_str()))
                event_ok = false;

            if (!m_event_svr_read.create(&sa, true, false, m_svr_event_read_string.c_str()))
                event_ok = false;

            if (!m_event_svr_write.create(&sa, true, false, m_svr_event_write_string.c_str()))
                event_ok = false;

            LocalFree(sa.lpSecurityDescriptor);
            if (!event_ok)
                return false;

            return true;
        }
	};
}}
#endif //__SQSHRMEMHELPER_H__