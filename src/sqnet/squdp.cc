#include <sqnet/squdp.h>
#include <sqnet/sqnethelper2.h>
#include <sqnet/sqcrypthelper.h>

namespace snqu { namespace net2{ 
SQNetUdp::SQNetUdp() 
{
	m_send_sock = SQ_INVALID_SOCKET;
	m_recv_sock = SQ_INVALID_SOCKET;
	m_port = 0;
	m_running = false;
}

SQNetUdp::~SQNetUdp() {}


boolean SQNetUdp::start(uint16 port)
{
	m_port = port;
	if (SQ_INVALID_SOCKET == m_send_sock)
	{
		m_send_sock = make_send_socket();
		if (SQ_INVALID_SOCKET == m_send_sock)
		{
			return FALSE;
		}
	}

	if (SQ_INVALID_SOCKET == m_recv_sock)
	{
		m_recv_sock = make_recv_socket();
		if (SQ_INVALID_SOCKET == m_recv_sock)
		{
			return FALSE;
		}
	}

	m_running = true;
	m_task_thread.begin(std::bind(&SQNetUdp::deal_proc, this, std::placeholders::_1));
	std::thread thd = std::thread(std::bind(&SQNetUdp::net_proc, this));
	thd.detach();

	return TRUE;
}

void SQNetUdp::stop()
{
	if (!m_running)
		return;

	m_running = FALSE;
	m_task_thread.end();

	if (m_send_sock != SQ_INVALID_SOCKET)
	{
		closesocket(m_send_sock);
		m_send_sock = SQ_INVALID_SOCKET;
	}

	if (m_recv_sock != SQ_INVALID_SOCKET)
	{
		closesocket(m_recv_sock);
		m_recv_sock = SQ_INVALID_SOCKET;
	}

	for (auto& item : m_message_map)
	{
		SAFE_DELETE(item.second.second);
	}

	m_message_map.clear();
}

boolean SQNetUdp::send_udp_packet(ulong32 to, char *buf, uint32 len, uint16 port)
{
	boolean rel = FALSE;
	sockaddr_in sender;
	int addrlen = sizeof(sender);
	int ret = 0;

	if (!buf || len <= 0)
	{
		return rel;
	}
	
	sender.sin_family = AF_INET;
	sender.sin_addr.s_addr = to;
	sender.sin_port = htons(port);

	ret = sendto(m_send_sock, (char *)buf, len, 0, (sockaddr *)&sender, addrlen);
	if (SOCKET_ERROR == ret)
	{
		//SNLOG(kFatal, << __FUNCTION__ << ", sendto error " << __socket_error_code();
		return rel;
	}

	rel = TRUE;
	return rel;
}

boolean SQNetUdp::send_data(uint32_t message_id, google::protobuf::Message *msg, bool encrypt, string toaddr)
{
	int len = 0;
	char *buf = nullptr;
	uint32_t encrypt_size = 0;
	string output;
	uint32_t input_size = 0;

	if (encrypt && msg != nullptr)
	{
		// 加密处理
		input_size = msg->ByteSize();
		encrypt_size = input_size;
		len = sizeof(SQHeader) + encrypt_size + sizeof(uint32_t);
	}
	else
		len = msg ? sizeof(SQHeader) + msg->ByteSize() + sizeof(uint32_t) : sizeof(SQHeader) + sizeof(uint32_t);

	buf = new char[len];
	auto header = reinterpret_cast<SQHeader*>(buf);
	header->signature = SIGNATURE;
	header->message_id = message_id;
	header->length = len;
	header->reserved = 0;
	header->pb_length = 0;
	header->option = 0;

	if (msg != nullptr)
	{
		if (encrypt)
		{
			SET_PACKET_ENCRYPT_BIT(header->option);
			msg->SerializeToArray(&header->body, msg->GetCachedSize());

			string crypt_key;
			SQEncryptHelper::get_simple_encrypt_key(crypt_key);
			SQEncryptHelper::simple_encrypt((char *)&header->body, (uint32)msg->GetCachedSize(), 
				crypt_key, crypt_key.size());
		}
		else
			msg->SerializeToArray(&header->body, msg->GetCachedSize());

		header->pb_length = msg->GetCachedSize();
	}

	// TODO: calc checksum
	uint32_t check_sum = 
		SQEncryptHelper::checksum((char *)&header->body, len - sizeof(SQHeader) - sizeof(uint32_t));
	*reinterpret_cast<uint32_t*>(((char*)header) + header->length - sizeof(uint32_t)) = check_sum;

	boolean rel = FALSE;
	rel = send_udp_packet(inet_addr(toaddr.c_str()), buf, len, m_port);

	SAFE_DELETE(buf);
	return rel;
}

uint32_t SQNetUdp::make_send_socket()
{
	SOCKET s = SQ_INVALID_SOCKET;
	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == SOCKET_ERROR)
	{
		//SNLOG(kFatal, << __FUNCTION__ << ", socket error " << __socket_error_code();
		return SQ_INVALID_SOCKET;
	}

	return s;
}

uint32_t SQNetUdp::make_recv_socket()
{
	SOCKET s = SQ_INVALID_SOCKET;
	sockaddr_in addrbind;

	addrbind.sin_family = AF_INET;
	addrbind.sin_addr.s_addr = INADDR_ANY;
	addrbind.sin_port = htons(m_port);

	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == SOCKET_ERROR)
	{
		//SNLOG(kFatal, << __FUNCTION__ << ", socket error " << __socket_error_code();
		return SQ_INVALID_SOCKET;
	}

	if (::bind(s, (sockaddr *)&addrbind, sizeof(sockaddr_in)) == SOCKET_ERROR)
	{
		//SNLOG(kFatal, << __FUNCTION__ << ", bind error " << __socket_error_code();
		closesocket(s);
		return SQ_INVALID_SOCKET;
	}

	return s;
}


void SQNetUdp::net_proc()
{
	sockaddr_in from;
	uint8 cbuf[SQ_TEMP_BUF_SIZE];
	int ret = 0;
	int fromlen = sizeof(from);
	boolean stopped = FALSE;

	while (m_running)
	{
		zero_memory(cbuf, sizeof(cbuf));
		ret = recvfrom(m_recv_sock, (char *)cbuf, sizeof(cbuf), 0, (sockaddr *)&from, &fromlen);
		if (SOCKET_ERROR == ret || ret <= 0)
		{
			break;
		}
		else
		{
			SQHeader* pHeader = (SQHeader*)cbuf;
			if (ret < (int)pHeader->length || pHeader->signature != SIGNATURE)
			{
				Sleep(10);
				continue;
			}

			// 数据负载大小，包括checksum
			int32 required = pHeader->length - sizeof(SQHeader);// - sizeof(uint32_t)/*checksum*/;
			uint32 message_id = pHeader->message_id;
			uint32 pb_size = pHeader->pb_length;
			uint32 options = pHeader->option;

			BYTE *body = (BYTE *)cbuf + sizeof(SQHeader);
			auto it = m_message_map.find(message_id);
			if (it != m_message_map.end())
			{
				bool pbParseOk = false;
				uint8_t *pbArray = nullptr;
				auto message = it->second.second ? it->second.second->New() : nullptr;
				proto_msg_ptr msg(message, [](google::protobuf::Message *p){ delete p;});

				if (PACKET_HAS_ENCRYPT_BIT(options))
				{
					decrypt_packet(body, required - sizeof(uint32_t));
					pbParseOk = msg->ParseFromArray(body, pb_size);
				}
				else
					pbParseOk = msg->ParseFromArray(body, required - sizeof(uint32_t));

				if (pbParseOk)
					add_user_packet(from.sin_addr.s_addr, message_id, msg);
			}
		}

		Sleep(10);
	}

	stop();
}

void SQNetUdp::decrypt_packet(uint8* data, uint32 size)
{
	string crypt_key;
	SQEncryptHelper::get_simple_encrypt_key(crypt_key);
	SQEncryptHelper::simple_encrypt((char *)data, size, crypt_key, crypt_key.size());
}

void SQNetUdp::add_user_packet(ulong32 ip, uint32 message_id, proto_msg_ptr Message)
{
	in_addr in;
	in.S_un.S_addr = ip;
	string ip_addr = inet_ntoa(in);
	m_task_thread.post_task(std::make_tuple(ip_addr, message_id, Message));
}

void SQNetUdp::deal_proc(const std::tuple<string/*ip_addr*/, uint32_t, proto_msg_ptr>& task)
{
	auto it = m_message_map.find(std::get<1>(task));
	if (it != m_message_map.end())
	{
		it->second.first(std::get<0>(task), std::get<2>(task));
	}
}

bool SQNetUdp::register_message(uint32_t message_id, 
									   const callback_func_t1& callback, 
									   google::protobuf::Message* factory /*= nullptr*/)
{
	if (m_message_map.find(message_id) != m_message_map.end())
		return false;

	m_message_map[message_id] = make_pair(callback, factory);
	return true;
}
}}

