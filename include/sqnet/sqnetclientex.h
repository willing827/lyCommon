#pragma once
#include <sqstd/sqinc.h>
#include <google/protobuf/message.h>
#include <memory>
#include <string>
#include <sqstd/sqtaskthread.h>
#include <sqnet/sqnethelper2.h>
#include <HPSocket/bufferptr.h>
#include <HPSocket/HPSocket.h>
#include <HPSocket/SocketInterface.h>
#include <sqnet/sqnet2.h>
#include <sqstd/sqtimerthread.h>
#include <sqstd/sqvecbuffhelper.h>
#include <codec/sqcodec.h>
#include <sqnet/sqcrypthelper.h>
#include <sqsafe/sqsafemodel.h>
#include <vmp/sqvmsdk.h>
#include <atomic>
#include <sqstd/sqsafeset.h>
#include "sqcenterparser.h"

using namespace snqu::safe;
namespace snqu{ namespace net2 {

	class NetClientEx : public CTcpClientListener
	{
	public:
		NetClientEx(uint32_t signature = SIGNATURE);
        ~NetClientEx();

        virtual EnHandleResult OnSend(IClient* pClient, const BYTE* pData, int iLength) override;
        virtual EnHandleResult OnReceive(IClient* pClient, const BYTE* pData, int iLength) override;
        virtual EnHandleResult OnClose(IClient* pClient) override;
        virtual EnHandleResult OnError(IClient* pClient, EnSocketOperation enOperation, int iErrorCode) override;
        virtual EnHandleResult OnConnect(IClient* pClient) override;

        typedef snqu::TimerThread task_thread_t;
	public:
        void set_session_data(const std::string& site_id, const std::string& session_id, const std::string& domain, bool is_v6_ver = false);
		bool start(int reconn_time = 15, int send_wait_time = 5);
		void stop();
        bool disconnect();
		bool connect(unsigned short port, bool async = false, bool is_auto_reconn = false);
		bool register_message(uint32_t message_id, const callback_func_center_t& callback, google::protobuf::Message* factory = nullptr);
        bool is_connected();

        template <typename protomsg_ptr>
        bool send_msg(uint32_t message_id, protomsg_ptr message)
        {
            if (!m_connected || NULL == message)
                return false;

            bool rel = false;
            if (m_p_connection != NULL)
            {
                if (0 == message->sequence())
                    message->set_sequence(m_sequence);
                message->set_session_id(m_session_id);
                std::string send_data = message->SerializeAsString();
                m_parser.make_send_packet(message_id, send_data);
                if (!send_data.empty())
                {
                    rel = m_p_connection->Send(reinterpret_cast<const BYTE*>(send_data.c_str()), send_data.size()) ? true: false;
                    if(rel)
                    {
                        m_sequence++;
                    }
                }
            }

            return rel;
        }

	protected:
		virtual void on_connected() = 0;
		virtual void on_close() = 0;
		virtual void on_error() = 0;
		virtual bool on_filter(uint32_t, proto_msg_ptr) { return true; }
        void reconnect();
        void stop_reconn();
        void clear_stop_client();
        bool connect(bool async = true);
        bool m_started;
        bool m_reconn;
        std::mutex m_unique_lock;
        std::condition_variable m_condition;
        int m_send_wait_second;
        int m_reconnect_time;
        task_thread_t m_task_thread;
        IClient* m_p_connection;

	private:
        void hearbeat_timer_proc(uint32_t id);
        void handle_packet_proc();
        void get_encry_key(IClient*);
		bool fire_hearbeat();
		void connected_notify_proc();
        void on_get_encry_key();

	private:
		atomic_bool m_connected;
		std::recursive_mutex m_lock;
		
        std::string m_session_id;
        std::string m_domain;
        CenterParser m_parser;
        std::string m_remote_ip;
        unsigned short m_remote_port;
        atomic_int m_sequence;
        snqu::SafeSet<IClient*> m_temp_conn;
	};
}}