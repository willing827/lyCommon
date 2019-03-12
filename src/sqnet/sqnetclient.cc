#include <sqnet/sqnet2.h>
#include <sqnet/sqnetclient.h>
#include <sqnet/sqpacketparser.h>

#define ktLoggerId "CNet2Lib"
#include <sqstd/sqinc.h>

using namespace snqu::safe;
namespace snqu{ namespace net2 {
class NetClientWrapperImpl
{
public:
	NetClientWrapperImpl() 
	{
		static bool called = false;
		if (!called)
		{
			called = true;
			//SQInitLogger(ktLoggerId);
			//SLOG(kTrace) << "[x] NetClientWrapper constructed.";
		}
	}

	~NetClientWrapperImpl() {}

	std::vector<char> m_buffer;
};

NetClientWrapper::NetClientWrapper() : m_wrapperImpl(new NetClientWrapperImpl())
{
}

NetClientWrapper::~NetClientWrapper()
{
	SAFE_DELETE(m_wrapperImpl);
}

bool NetClientWrapper::send_data(
	CONNID connid, 
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
	uint16_t check_sum = 
		SQEncryptHelper::checksum((char *)&header->body, len - sizeof(SQHeader) - sizeof(uint32_t));
	*reinterpret_cast<uint16_t*>(((char*)header) + header->length - sizeof(uint32_t)) = check_sum;

	bool rel = false;
	rel = reinterpret_cast<IClient*>(connid)->Send(reinterpret_cast<const BYTE*>(buf), len) ? true: false;
	SAFE_DELETE(buf);
	
	_VMProtectEnd();
	return rel;
}

bool NetClientWrapper::send_raw_data(
	CONNID connid, 
	uint32_t message_id, 
	const string& xdata, 
	const string& session_key,
	bool encrypt)
{
	_VMProtectBegin(__FUNCTION__);
	int len = 0;
	char *buf = nullptr;
	uint32_t encrypt_size = 0;
	string output;
	uint32_t input_size = 0;

	if (encrypt && xdata.size() > 0)
	{
		// 加密处理
		input_size = xdata.size();
		input_size += AES_BLOCK_LEN;

		encrypt_size = input_size;
		len = sizeof(SQHeader) + encrypt_size + sizeof(uint32_t);
	}
	else
		len = (xdata.size() > 0) ? sizeof(SQHeader) + xdata.size() + sizeof(uint32_t) : sizeof(SQHeader) + sizeof(uint32_t);

	buf = new char[len];
	auto header = reinterpret_cast<SQHeader*>(buf);
	header->signature = SIGNATURE;
	header->message_id = message_id;
	header->length = len;
	header->reserved = 0;
	header->pb_length = 0;
	header->option = 0;

	if (xdata.size() > 0)
	{
		if (encrypt)
		{
			SET_PACKET_ENCRYPT_BIT(header->option);
			//string sbody = msg->SerializeAsString();
			string pkey("");
			string encode_body = ISQSafeModel::std_algo()->aes_encryptx(xdata, session_key, pkey);
			header->private_key = _strtoui64(pkey.c_str(), nullptr, 10);
			memcpy(&header->body, encode_body.c_str(), encode_body.size());
		}
		else
		{
			//msg->SerializeToArray(&header->body, msg->GetCachedSize());
			memcpy(&header->body, xdata.c_str(), xdata.size());
		}

		//header->pb_length = msg->GetCachedSize();
		header->pb_length = xdata.size();
	}

	// TODO: calc checksum
	uint16_t check_sum = 
		SQEncryptHelper::checksum((char *)&header->body, len - sizeof(SQHeader) - sizeof(uint32_t));
	*reinterpret_cast<uint16_t*>(((char*)header) + header->length - sizeof(uint32_t)) = check_sum;

	bool rel = false;
	rel = reinterpret_cast<IClient*>(connid)->Send(reinterpret_cast<const BYTE*>(buf), len) ? true: false;
	SAFE_DELETE(buf);

	_VMProtectEnd();
	return rel;
}

bool NetClientWrapper::send_data2(
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
	header->pb_length = 0;
	header->option = 0;
	header->reserved = 0;

	if (msg)
		memcpy(&header->body, msg, size);

	// TODO: calc checksum
	uint32_t check_sum = 
		SQEncryptHelper::checksum((char *)&header->body, len - sizeof(SQHeader) - sizeof(uint32_t));
	*reinterpret_cast<uint32_t*>(((char*)header) + header->length - sizeof(uint32_t)) = check_sum;

	bool rel = false;
	rel = reinterpret_cast<IClient*>(sock)->Send(reinterpret_cast<const BYTE*>(buf), len) ? true: false;
	SAFE_DELETE(buf);
	
	_VMProtectEnd();
	return rel;
}

string NetClientWrapper::decrypt_packet(
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

bool NetClientWrapper::encrypt_session_key(
	CONNID connid, 
	uint8* buf, 
	uint32 size,
	std::function<void(const string& session_key)> func)
{
	_VMProtectBegin(__FUNCTION__);
	bool rel = false;
	if (size != AES_KEY_LEN*2)
		rel = false;
	else
	{
		string session_key;
		session_key.resize(AES_KEY_LEN);
		for (uint32 i = 0; i < AES_KEY_LEN; i++)
			*reinterpret_cast<uint8*>(&session_key[i]) = buf[i];

		// 保存通讯KEY
		net2::SQEncryptHelper::simple_encrypt_key(session_key, AES_KEY_LEN);
		func(session_key);
		rel = true;
	}

	_VMProtectEnd();
	return rel;
}

void NetClientWrapper::get_session_key(
	CONNID connid)
{
	_VMProtectBegin(__FUNCTION__);
	string noneused = "~~~~$$";
	send_data2(connid, net2::InternalQueryKeyMsg, nullptr, 0);
	_VMProtectEnd();
}

EnHandleResult NetClientWrapper::receive_packet(
	IClient* pClient, 
	const BYTE* pData, 
	int iLength,
	std::function<void(CONNID connid, uint8* buf, uint32 size)> save_key_func,
	std::function<string(CONNID connid, uint64_t ulpkey, uint8* data, uint32 size)> decrypt_func,
	std::function<void(IClient* client, uint32 message_id, google::protobuf::Message* Message)> add_packet_func,
	std::function<google::protobuf::Message*(uint32 message_id)> get_msg_func
	)
{
	#define m_buffer m_wrapperImpl->m_buffer
	_VMProtectBegin(__FUNCTION__);
	EnHandleResult rel = HR_OK;
	uint32_t message_id = 0;
	uint32_t pb_size = 0;
	snqu::buffer::write(m_buffer, m_buffer.size(), (const char*)pData, iLength);

	int packet_size(0);
	int remain =iLength;
	size_t packet_head_len = sizeof(SQHeader);

	while (m_buffer.size() > packet_head_len)
	{
		// 数据太少, 不足包头数据
		if (m_buffer.size() < packet_head_len)
			break;

		//  检查信令头的标识是否正确
		SQHeader* header = reinterpret_cast<SQHeader*>(&m_buffer[0]);
		if (header->signature != SIGNATURE) 
		{
			// 返回HR_ERROR表示要终止此连接
			rel = HR_ERROR;
			m_buffer.clear();
		    //SNLOG(kError) << " packet SIGNATURE error.";
			break;
		}

		// 计算包长
		// 数据负载大小，包括checksum
		packet_size = header->length - packet_head_len;// - sizeof(uint32_t) //checksum;
		message_id = header->message_id;

		// 包长度超过最大长度, 收到非法数据, 解析异常
		if (packet_size > 1024*1024*50)
		{
		    //SNLOG(kError) << " too long packet, invalidate.";
			rel = HR_ERROR;
			m_buffer.clear();
			break;
		}

		// 数据太少, 不足一个包的数据
		if (header->length > (int)m_buffer.size())
		{
			break;
		}

		if (net2::InternalQueryKeyMsg == message_id)
		{
			// 保存此次通讯的key
			save_key_func(reinterpret_cast<CONNID>(pClient), (uint8*)&header->body, 
				packet_size - sizeof(uint32_t));
			m_buffer.erase(m_buffer.begin(), m_buffer.begin() + header->length);
			rel = HR_OK;
			//SNLOG(kTrace, "received session key.";
			continue;
		}

		// TODO:
		// Add checksum logic
		char* packet = &m_buffer[0];
		uint16_t checksum = *reinterpret_cast<const uint16_t*>(packet + (header->length - sizeof(uint32_t)));
		uint8 *body = reinterpret_cast<uint8*>(packet + sizeof(SQHeader));
		uint32_t required = header->length - sizeof(SQHeader);
		CONNID conn_id = reinterpret_cast<CONNID>(pClient);

		auto msg = get_msg_func(message_id);
		if (msg != nullptr)
		{
			bool pbParseOk = false;
			uint8_t *pbArray = nullptr;
			auto message = msg ? msg->New() : nullptr;
			pb_size = header->pb_length;

			if (PACKET_HAS_ENCRYPT_BIT(header->option))
			{
				string plaint = decrypt_func(conn_id, header->private_key, body, required - sizeof(uint32_t));
				pbParseOk = message->ParseFromString(plaint);
			}
			else
				pbParseOk = message->ParseFromArray(body, required - sizeof(uint32_t));

			if (pbParseOk)
			{
				add_packet_func(pClient, message_id, message);
				m_buffer.erase(m_buffer.begin(), m_buffer.begin() + header->length);
			}
			else
			{
				// protoBuff 解析失败
			    //SNLOG(kFatal) << " receive_packet parse packet error, msg: " << message_id;
				if (header->length <= m_buffer.size())
					m_buffer.erase(m_buffer.begin(), m_buffer.begin() + header->length);
				else
				{
					// 后面的包暂时只有抛弃了
					m_buffer.clear();
					break;
				}
			}
		}
		else
		{
			// 没找到的包类型，暂时忽略掉
			m_buffer.erase(m_buffer.begin(), m_buffer.begin() + header->length);
		}
	}

	if (rel != HR_OK)
	{
	    //SNLOG(kFatal) << "receive_packet return HR_ERROR.";
	}

	_VMProtectEnd();
	return rel;
}

bool NetClientWrapper::send_hearbeat(
	CONNID connid)
{
	auto len = sizeof(SQHeader) + sizeof(uint32_t);
	char buf[HEADERSIZE + sizeof(uint32_t)];

	auto header = reinterpret_cast<SQHeader*>(buf);

	header->signature = SIGNATURE;
	header->message_id = net2::InternalHeartMsg;
	header->length = len;
	header->reserved = 0;
	header->pb_length = 0;

	// TODO: calc checksum
	*reinterpret_cast<uint32_t*>(((char*)header) + header->length - sizeof(uint32_t)) = 0;
	bool rel = reinterpret_cast<IClient*>(connid)->Send(reinterpret_cast<const BYTE*>(buf), len) ? true: false;

	return rel;
}
}}