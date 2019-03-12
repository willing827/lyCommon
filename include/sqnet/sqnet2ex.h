#ifndef SQNET2EX_H
#define SQNET2EX_H

#include <sqstd/sqinc.h>
#include <HPSocket/HPSocket.h>
#include <HPSocket/SocketInterface.h>
#include <sqstd/sqtaskthread.h>
#include <protobuf/message.h>

#define SET_PACKET_ENCRYPT_BIT(option) option |= 0x80
#define PACKET_HAS_ENCRYPT_BIT(option) ((option & 0x80) != 0)

#define SQ_WSA_HEIBYTE         2
#define SQ_WSA_LOWBYTE         2
#define SQ_TEMP_BUF_SIZE       8192
#define SQ_UDP_APPSERVER_BROADCAST_PORT 13124
#define SQ_UDP_APPCLIENT_BROADCAST_PORT 13125

namespace snqu { namespace net2{ 
    typedef std::shared_ptr<google::protobuf::Message> proto_msg_ptr;
	typedef std::function<bool(CONNID, uint32_t, google::protobuf::Message*, bool)> send_func_t;
	typedef std::function<bool(CONNID, uint32_t, const string&, bool)> send_raw_func_t;
	typedef function<void (std::string, proto_msg_ptr)> callback_func_t1;
	
	class SQNet_Broadcast_Ex
	{
	public:
		SQNet_Broadcast_Ex();
		virtual ~SQNet_Broadcast_Ex();
		
	public:
		boolean start(uint16 port, bool broadcast);
		void stop();
		boolean send_data(uint32_t message_id, google::protobuf::Message *msg, bool encrypt, string toaddr);
		bool register_message(uint32_t message_id, const callback_func_t1& callback, google::protobuf::Message* factory = nullptr);

	protected:
		boolean make_broadcast_socket(uint16 port);
		boolean make_broadcast_recv_socket(uint16 port);
		boolean load_winsockdll();
		boolean send_broadcast_packet(ulong32 to, char *buf, uint32 len, uint16 port);
		void decrypt_packet(uint8* data, uint32 size);

		void net_proc();
		void deal_proc(const std::tuple<string/*ip_addr*/, uint32_t, proto_msg_ptr>& task);
		void add_user_packet(ulong32 ip, uint32 message_id, proto_msg_ptr Message);

	private:
		uint32_t	m_broadcast_sock;
		uint32_t    m_broadcast_recv;
		boolean     m_broadcast_running;
		boolean     m_ws2_inited;
		uint32_t    m_port;

		unordered_map<uint32_t, pair<callback_func_t1, google::protobuf::Message*> > m_message_map;
		snqu::TaskThread<std::tuple<string/*ip_addr*/, uint32_t, proto_msg_ptr> > m_task_thread;
	};
}}

#endif // SQNET2EX_H