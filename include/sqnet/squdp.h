#ifndef SQUDP_H
#define SQUDP_H

#include <sqstd/sqinc.h>
#include <sqstd/sqtaskthread.h>
#include <sqnet/sqnet2.h>

namespace snqu { namespace net2{ 
	class SQNetUdp
	{
	public:
		SQNetUdp();
		virtual ~SQNetUdp();

	public:
		boolean start(uint16 port);
		void stop();
		boolean send_data(uint32_t message_id, google::protobuf::Message *msg, bool encrypt, string toaddr);
		bool register_message(uint32_t message_id, const callback_func_t1& callback, google::protobuf::Message* factory = nullptr);

	private:
		uint32_t make_send_socket();
		uint32_t make_recv_socket();
		
		boolean send_udp_packet(ulong32 to, char *buf, uint32 len, uint16 port);
		void decrypt_packet(uint8* data, uint32 size);
		void net_proc();
		void deal_proc(const std::tuple<string/*ip_addr*/, uint32_t, proto_msg_ptr>& task);
		void add_user_packet(ulong32 ip, uint32 message_id, proto_msg_ptr Message);

	private:
		uint32_t m_send_sock;
		uint32_t m_recv_sock;
		uint16_t m_port;
		bool m_running;

		unordered_map<uint32_t, pair<callback_func_t1, google::protobuf::Message*> > m_message_map;
		snqu::TaskThread<std::tuple<string/*ip_addr*/, uint32_t, proto_msg_ptr> > m_task_thread;
	};
}}

#endif //SQUDP_H