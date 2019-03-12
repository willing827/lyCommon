#include <sqnet/sqnet2.h>
#include <sqnet/sqnetservice.h>
#include <sqnet/sqpacketparser.h>
#include <vmp/sqvmsdk.h>
#include "netlog.h"

using namespace snqu::safe;
namespace snqu{ namespace net2 {
NetServiceWrapper::NetServiceWrapper()
{
	m_server_engine = nullptr;
	m_site_id = "0112!!~!{}$";
}

NetServiceWrapper::~NetServiceWrapper()
{
}

bool NetServiceWrapper::send_data(
	CONNID sock,
	uint32_t message_id, 
	google::protobuf::Message *msg, 
	const string& session_key,
	bool encrypt)
{
	_VMProtectBegin(__FUNCTION__);
	int len = 0;
	char *buf = nullptr;
	uint32_t encrypt_size = 0;
	string output;
	uint32_t input_size = 0;

	if (encrypt && msg != nullptr)
	{
		// 加密处理
		input_size = msg->ByteSize();
		input_size += AES_BLOCK_LEN;

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
			string sbody = msg->SerializeAsString();
			string pkey("");
			string encode_body = ISQSafeModel::std_algo()->aes_encryptx(sbody, session_key, pkey);
			header->private_key = _strtoui64(pkey.c_str(), nullptr, 10);
			memcpy(&header->body, encode_body.c_str(), encode_body.size());
		}
		else
			msg->SerializeToArray(&header->body, msg->GetCachedSize());

		header->pb_length = msg->GetCachedSize();
	}

	// TODO: calc checksum
	uint32_t check_sum = 
		SQEncryptHelper::checksum((char *)&header->body, len - sizeof(SQHeader) - sizeof(uint32_t));
	*reinterpret_cast<uint32_t*>(((char*)header) + header->length - sizeof(uint32_t)) = check_sum;

	bool rel = false;
	rel = m_server_engine->Send(sock, reinterpret_cast<const BYTE*>(buf), len) ? true: false;
	SAFE_DELETE(buf);

	_VMProtectEnd();
	return rel;
}

bool NetServiceWrapper::send_data2(
	CONNID sock, 
	uint32_t message_id, 
	uint8* msg, 
	uint32 size)
{
	_VMProtectBegin(__FUNCTION__);
	auto len = msg ? sizeof(SQHeader) + size + sizeof(uint32_t) : sizeof(SQHeader) + sizeof(uint32_t);
	char *buf = new char[len];

	auto header = reinterpret_cast<SQHeader*>(buf);

	header->signature = SIGNATURE;
	header->message_id = message_id;
	header->length = len;
	header->reserved = 0;
	header->pb_length = 0;
	header->option = 0;

	if (msg)
		memcpy(&header->body, msg, size);

	// TODO: calc checksum
	uint32_t check_sum = 
		SQEncryptHelper::checksum((char *)&header->body, len - sizeof(SQHeader) - sizeof(uint32_t));
	*reinterpret_cast<uint32_t*>(((char*)header) + header->length - sizeof(uint32_t)) = check_sum;

	bool rel = false;
	rel = m_server_engine->Send(sock, reinterpret_cast<const BYTE*>(buf), len) ? true: false;
	SAFE_DELETE(buf);
	
	_VMProtectEnd();
	return rel;
}

string NetServiceWrapper::decrypt_packet(
	CONNID connid, 
	uint64_t ulpkey, 
	uint8* data, 
	uint32 size,
	const string& session_key)
{
	_VMProtectBegin(__FUNCTION__);
	string cipher((char *)data, size);
	string private_key = fmt::Format("{0}", ulpkey);
	return ISQSafeModel::std_algo()->aes_decryptx(cipher, session_key, private_key);
	_VMProtectEnd();
}

void NetServiceWrapper::query_internal_key(
	CONNID connid,
	std::function<void(CONNID connid, const string& local_key)> save_key_func,
    std::function<void(CONNID connid)> fire_conn_func)
{
	_VMProtectBegin(__FUNCTION__);
	string noneused_key;
	noneused_key.resize(AES_KEY_LEN);
	net2::SQEncryptHelper::generate_random_key(noneused_key, AES_KEY_LEN);

	// 生成16字节的AES通讯key
	string session_key;
	session_key.resize(AES_KEY_LEN);
	net2::SQEncryptHelper::generate_random_key(session_key, AES_KEY_LEN);

	// 保存
	save_key_func(connid, session_key);

	// 对key加密
	net2::SQEncryptHelper::simple_encrypt_key(session_key, session_key.size());

	session_key += noneused_key;
	send_data2(connid, net2::InternalQueryKeyMsg, (uint8*)session_key.c_str(), session_key.size());

    fire_conn_func(connid);
	_VMProtectEnd();
}

TPkgInfo* NetServiceWrapper::find_header(
	CONNID dwConnID)
{
	PVOID pInfo = nullptr;
	m_server_engine->GetConnectionExtra(dwConnID, &pInfo);
	return (TPkgInfo* )pInfo;
}

EnHandleResult NetServiceWrapper::receive_packet(
	CONNID connid, 
	int length,
	std::function<void(CONNID connid)> heartbeat_func,
	std::function<void(CONNID connid)> query_key_func,
	std::function<string(CONNID connid, uint64_t ulpkey, uint8* data, uint32 size)> decrypt_func,
	std::function<void(CONNID connid, uint32 message_id, proto_msg_ptr msg)> add_packet_func,
	std::function<google::protobuf::Message*(uint32 message_id)> get_msg_func)
{
	EnHandleResult rel = HR_OK;
	TPkgInfo *pkginfo = find_header(connid);
	if (pkginfo != nullptr)
	{
		int required = pkginfo->length;
		int remain = length;
		auto conHeader = pkginfo->_header;
		while(remain >= required)
		{
			remain -= required;
			CBufferPtr buffer(required);

			EnFetchResult result = m_server_engine->Fetch(connid, buffer, (int)buffer.Size());
			if (result == FR_OK)
			{
				if (buffer == NULL)
				{
					rel = HR_OK;
					break;
				}

				if (pkginfo->is_header)
				{
					SQHeader* pHeader = (SQHeader*)buffer.Ptr();
					if (SIGNATURE != pHeader->signature)
					{
					    //SNLOG(kFatal) << "数据包校验码失败，关闭连接, dwConnID = " << connid ;
						rel = HR_ERROR;
						break;
					}

					// 数据负载大小，包括checksum
					required = pHeader->length - sizeof(SQHeader);// - sizeof(uint32_t)/*checksum*/;
					conHeader->message_id = pHeader->message_id;
					conHeader->pb_length = pHeader->pb_length;
					conHeader->option = pHeader->option;
					conHeader->private_key = pHeader->private_key;
				}
				else
				{
					if (conHeader->message_id == net2::InternalHeartMsg)
					{
						pkginfo->is_header = !pkginfo->is_header;
						pkginfo->length	 = sizeof(SQHeader);
						heartbeat_func(connid);
						break;
					}

					if (conHeader->message_id == net2::InternalQueryKeyMsg)
					{
						pkginfo->is_header = !pkginfo->is_header;
						pkginfo->length	 = sizeof(SQHeader);
						query_key_func(connid);
						break;
					}

					BYTE *body = (BYTE *)buffer;
					uint16_t checksum = *reinterpret_cast<const uint16_t*>(body + required - sizeof(uint32_t));

					// TODO:
					// Add checksum logic
					//uint16_t calced_checksum = SQEncryptHelper::checksum((char *)body, required - sizeof(uint32_t));

					auto factory = get_msg_func(conHeader->message_id);
					if (factory != nullptr)
					{
						bool pbParseOk = false;
						uint8_t *pbArray = nullptr;
						auto message = factory ? factory->New() : nullptr;
						proto_msg_ptr msg(message, [](google::protobuf::Message *p){ delete p;});

						if (PACKET_HAS_ENCRYPT_BIT(conHeader->option))
						{
							string plaint("");
							plaint = decrypt_func(connid, conHeader->private_key, body, required - sizeof(uint32_t));
							pbParseOk = msg->ParseFromString(plaint);
						}
						else
							pbParseOk = msg->ParseFromArray(body, required - sizeof(uint32_t));

						if (pbParseOk)
						{
							add_packet_func(connid, conHeader->message_id, msg);
							//SNLOG(kTrace, "receive " << required << " bytes with msg_id " << message_id << ", connid: " 
							//	<< dwConnID;
						}
						else 
						{
							CHAR szAddress[40];
							int iAddressLen = sizeof(szAddress) / sizeof(CHAR);
							USHORT usPort = 0;

							m_server_engine->GetRemoteAddress(connid, szAddress, iAddressLen, usPort);
						    //SNLOG(kFatal) << "message->ParseFromString error, ipaddr = " << szAddress << ":" << usPort << ", message_id = " << conHeader->message_id;
							rel = HR_ERROR;
						}
					}
					else
					{
                        CHAR szAddress[40];
                        int iAddressLen = sizeof(szAddress) / sizeof(CHAR);
                        USHORT usPort = 0;

                        m_server_engine->GetRemoteAddress(connid, szAddress, iAddressLen, usPort);
						// 没找到的包类型，暂时忽略掉
                       //SNLOG((kFatal) << "not regist packet type ip:" << szAddress << ":" << usPort << ", message_id = " << conHeader->message_id;
					}

					// 下一个包要取header
					required = sizeof(SQHeader);
				}

				pkginfo->is_header = !pkginfo->is_header;
				pkginfo->length	 = required;
			}// if (result == FR_OK)
			else 
				rel = HR_ERROR;

			if (rel != HR_OK)
				break;
		} // while
	}

	return rel;
}

void record_log(int conn, int msg_id, int start, int end)
{
    //if ((end - start) > 100 && msg_id > 11000 && msg_id < 15000)
       //SNLOG((kTrace) << fmt::Format("job do slow conn[{0}] msg_id[{1}] cost_time[{2}]", conn, msg_id, end - start);
}

}}