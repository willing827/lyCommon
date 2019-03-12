#include <sqnet/sqnet2.h>
#include <sqnet/sqnethelper2.h>
#include <sqnet/sqcrypthelper.h>
#include "netlog.h"
#include <vmp/sqvmsdk.h>


HANDLE g_hNet2Log = nullptr;
namespace snqu { namespace net2{ 
SQNet_Broadcast::SQNet_Broadcast()
{
	m_broadcast_sock = SQ_INVALID_SOCKET;
	m_broadcast_recv = SQ_INVALID_SOCKET;
	m_broadcast_running = FALSE;
	m_ws2_inited = FALSE;
	m_port = 0;
}

SQNet_Broadcast::~SQNet_Broadcast()
{
}

boolean SQNet_Broadcast::send_broadcast_packet(ulong32 to, char *buf, uint32 len, uint16 port)
{
	boolean rel = FALSE;
	sockaddr_in sender;
	int addrlen = sizeof(sender);
	int ret = 0;

	if (!buf || len <= 0)
	{
		return rel;
	}

	if (SQ_INVALID_SOCKET == m_broadcast_sock)
	{
		if (!make_broadcast_socket(port))
		{
		    //SNLOG(kFatal) << ", make_broadcast_socket error!";
			return rel;
		}
	}

	sender.sin_family = AF_INET;
	sender.sin_addr.s_addr = to;
	sender.sin_port = htons(port);

	if (INADDR_BROADCAST == to)
		ret = sendto(m_broadcast_sock, (char *)buf, len, 0, (sockaddr *)&sender, addrlen);
	else 
		ret = sendto(m_broadcast_recv, (char *)buf, len, 0, (sockaddr *)&sender, addrlen);
	if (SOCKET_ERROR == ret)
	{
	    //SNLOG(kFatal) << __FUNCTION__ << ", sendto error " << __socket_error_code();
		return rel;
	}

	rel = TRUE;
	return rel;
}

boolean SQNet_Broadcast::send_data(uint32_t message_id, google::protobuf::Message *msg, bool encrypt, string toaddr)
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
	if (toaddr.empty())
		rel = send_broadcast_packet(INADDR_BROADCAST, buf, len, m_port);
	else
		rel = send_broadcast_packet(inet_addr(toaddr.c_str()), buf, len, m_port);

	SAFE_DELETE(buf);
	return rel;
}

boolean SQNet_Broadcast::start(uint16 port, bool broadcast)
{
	if (!m_ws2_inited)
	{
		if(!load_winsockdll())
		{
		    //SNLOG(kFatal, "初始化ws2_32失败");
			return FALSE;
		}
	}

	m_ws2_inited = TRUE;
	if (SQ_INVALID_SOCKET == m_broadcast_recv)
	{
		if (!make_broadcast_recv_socket(port))
		{
		    //SNLOG(kFatal) << __FUNCTION__ << ", make_broadcast_recv_socket error!";
			return FALSE;
		}
	}

	if (broadcast && SQ_INVALID_SOCKET == m_broadcast_sock)
	{
		if (!make_broadcast_socket(port))
		{
		    //SNLOG(kFatal) << __FUNCTION__ << ", make_broadcast_socket error!";
			return FALSE;
		}
	}

	m_port = port;
	m_broadcast_running = TRUE;
	m_task_thread.begin(std::bind(&SQNet_Broadcast::deal_proc, this, std::placeholders::_1));
	std::thread thd = std::thread(std::bind(&SQNet_Broadcast::net_proc, this));
	thd.detach();

	return TRUE;
}

void SQNet_Broadcast::stop()
{
	if (!m_broadcast_running)
		return;

	m_broadcast_running = FALSE;
	m_task_thread.end();
	
	if (m_broadcast_sock != SQ_INVALID_SOCKET)
	{
		closesocket(m_broadcast_sock);
		m_broadcast_sock = SQ_INVALID_SOCKET;
	}

	if (m_broadcast_recv != SQ_INVALID_SOCKET)
	{
		closesocket(m_broadcast_recv);
		m_broadcast_recv = SQ_INVALID_SOCKET;
	}

    for (auto& item : m_message_map)
    {
        SAFE_DELETE(item.second.second);
    }

    m_message_map.clear();
}

boolean SQNet_Broadcast::make_broadcast_socket(uint16 port)
{
	boolean rel = FALSE;
	SOCKET s = SQ_INVALID_SOCKET;
	BOOL optval;

	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); //IPPROTO_UDP
	if (s == SQ_INVALID_SOCKET)
	{
	    //SNLOG(kFatal) << __FUNCTION__ << " socket error " << __socket_error_code();
		return rel;
	}

	optval = TRUE;
	if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, (char FAR *)&optval, sizeof(optval))
		== SOCKET_ERROR)
	{
	    //SNLOG(kFatal) << __FUNCTION__ << " setsockopt error " << __socket_error_code();
		closesocket(s);
		return rel;
	}

	rel = TRUE;
	m_broadcast_sock = s;
	return rel;
}

boolean SQNet_Broadcast::make_broadcast_recv_socket(uint16 port)
{
	SOCKET s = SQ_INVALID_SOCKET;
	sockaddr_in addrbind;

	addrbind.sin_family = AF_INET;
	addrbind.sin_addr.s_addr = INADDR_ANY;
	addrbind.sin_port = htons(port); 

	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == SOCKET_ERROR)
	{
	    //SNLOG(kFatal) << __FUNCTION__ << ", socket error " << __socket_error_code();
		return FALSE;
	}

	if (::bind(s, (sockaddr *)&addrbind, sizeof(sockaddr_in)) == SOCKET_ERROR)
	{
	    //SNLOG(kFatal) << __FUNCTION__ << ", bind error " << __socket_error_code();
		closesocket(s);
		return FALSE;
	}

	m_broadcast_recv = s;
	return TRUE;
}

boolean SQNet_Broadcast::load_winsockdll()
{
	int     liErr;
	WORD    liVerRequested;
	WSADATA wsadata;

	// 
	// 初始化WinSock库
	//
	liVerRequested = MAKEWORD(SQ_WSA_HEIBYTE, SQ_WSA_LOWBYTE);
	liErr = WSAStartup(liVerRequested, &wsadata);
	if (liErr!=0)
	{
		//SNLOG(kFatal, << __FUNCTION__ << "WSAStartup() failed code "<< __socket_error_code();
		return false;
	}

	/* Confirm that the WinSock DLL supports 2.2.*/
	/* Note that if the DLL supports versions greater    */
	/* than 2.2 in addition to 2.2, it will still return */
	/* 2.2 in wVersion since that is the version we      */
	/* requested.                                        */

	if ( LOBYTE( wsadata.wVersion ) != SQ_WSA_LOWBYTE ||
		HIBYTE( wsadata.wVersion ) != SQ_WSA_HEIBYTE )
	{
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		//SNLOG(kFatal, << __FUNCTION__ << ", Tell the user that we could not find a wVersion usable WinSock DLL!";
		return false; 
	}
	return true;
}

void SQNet_Broadcast::net_proc()
{
	sockaddr_in from;
	uint8 cbuf[SQ_TEMP_BUF_SIZE];
	int ret = 0;
	int fromlen = sizeof(from);
	boolean stopped = FALSE;

	while (m_broadcast_running)
	{
		zero_memory(cbuf, sizeof(cbuf));
		ret = recvfrom(m_broadcast_recv, (char *)cbuf, sizeof(cbuf), 0, (sockaddr *)&from, &fromlen);
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
				proto_msg_ptr msg(it->second.second->New());

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

void SQNet_Broadcast::decrypt_packet(uint8* data, uint32 size)
{
	_VMProtectBegin(__FUNCTION__);
	string crypt_key;
	SQEncryptHelper::get_simple_encrypt_key(crypt_key);
	SQEncryptHelper::simple_encrypt((char *)data, size, crypt_key, crypt_key.size());
	_VMProtectEnd();
}

void SQNet_Broadcast::add_user_packet(ulong32 ip, uint32 message_id, proto_msg_ptr Message)
{
	in_addr in;
	in.S_un.S_addr = ip;
	string ip_addr = inet_ntoa(in);
	m_task_thread.post_task(std::make_tuple(ip_addr, message_id, Message));
}

void SQNet_Broadcast::deal_proc(const std::tuple<string/*ip_addr*/, uint32_t, proto_msg_ptr>& task)
{
	auto it = m_message_map.find(std::get<1>(task));
	if (it != m_message_map.end())
	{
		it->second.first(std::get<0>(task), std::get<2>(task));
	}
}

bool SQNet_Broadcast::register_message(uint32_t message_id, 
					  const callback_func_t1& callback, 
					  google::protobuf::Message* factory /*= nullptr*/)
{
	if (m_message_map.find(message_id) != m_message_map.end())
		return false;

	m_message_map[message_id] = make_pair(callback, factory);
	return true;
}
}}

void init_sqnetlib()
{
	//SQInitLogger(ktLoggerId);
	int     liErr;
	WORD    liVerRequested;
	WSADATA wsadata;

	// 
	// 初始化WinSock库
	//
	liVerRequested = MAKEWORD(SQ_WSA_HEIBYTE, SQ_WSA_LOWBYTE);
	liErr = WSAStartup(liVerRequested, &wsadata);
	if (liErr!=0)
	{
		//SNLOG(kFatal, << __FUNCTION__ << "WSAStartup() failed code "<< __socket_error_code();
		return;
	}

	/* Confirm that the WinSock DLL supports 2.2.*/
	/* Note that if the DLL supports versions greater    */
	/* than 2.2 in addition to 2.2, it will still return */
	/* 2.2 in wVersion since that is the version we      */
	/* requested.                                        */

	if (LOBYTE( wsadata.wVersion ) != SQ_WSA_LOWBYTE ||
		HIBYTE( wsadata.wVersion ) != SQ_WSA_HEIBYTE )
	{
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		//SNLOG(kFatal, << __FUNCTION__ << ", Tell the user that we could not find a wVersion usable WinSock DLL!";
		return; 
	}
	return;
}