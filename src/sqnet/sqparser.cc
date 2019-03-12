#include <sqnet/sqparser.h>
#include <codec/sqcodec.h>
#define ktLoggerId "RobotNetLib"
#include <sqnet/sqnetapi.h>

#define BUFF_SIZE_PART 10240

using namespace snqu::safe;
namespace snqu{ namespace net {


SqParser::SqParser(uint32_t packet_signature /*= SIGNATURE*/)
{
	m_signature = packet_signature;
	cur_size = BUFF_SIZE_PART;
	m_buffer.reserve(cur_size);
	clear_encry_key();
}

void SqParser::clear_encry_key()
{
    char dls[] = {'*','&','#',')','s','d','f','(','!','*','1','1','@',')','#','\0'};
    m_encry_key = dls;
}

SqParser::~SqParser()
{
    m_buffer.clear();
}

void SqParser::add_buffer(const char* buffer, unsigned int len)
{
    if (m_buffer.size() + len > cur_size)
    {
        m_buffer.reserve(cur_size + BUFF_SIZE_PART);
        cur_size += BUFF_SIZE_PART;
    }
    buffer::write(m_buffer, m_buffer.size(), buffer, len);
}

int SqParser::pop_packet(uint32_t& message_id, std::string& message)
{
    _VMProtectBegin(__FUNCTION__);
    int rel = 0;
    message_id = 0;
    uint32_t pb_size = 0;
    uint32_t body_len(0), total_len(0);
    size_t packet_head_len = sizeof(SQHeader);

    do
    {
        // 数据太少, 不足包头数据
        if (m_buffer.size() < packet_head_len)
        {
           //SNLOG((kDebug) << " packet not enough for packet_head_len.";
           //返回0继续读
            rel = 0;
            break;
        }

        //  检查信令头的标识是否正确
        SQHeader* header = reinterpret_cast<SQHeader*>(&m_buffer[0]);
        if (ntohl(header->signature) != m_signature)
        {
            //返回-1表示要终止此连接
            rel = -1;
            m_buffer.clear();
            //SNLOG((kError) << " packet SIGNATURE error.";
            break;
        }

        // 计算包长
        // 数据负载大小，包括checksum
        total_len = ntohl(header->length);
        body_len = total_len - packet_head_len - 4;
        message_id = ntohl(header->message_id);
        pb_size = ntohl(header->pb_length);

        // 包长度超过最大长度, 收到非法数据, 解析异常
        if (body_len > 1024*1024*50)
        {
           //SNLOG((kError) << " too long packet, invalidate.";
            rel = -1;
            m_buffer.clear();
            break;
        }

        // 数据太少, 不足一个包的数据
        if (total_len > (int)m_buffer.size())
        {
            rel = 0;
			break;
        }

        // Add checksum logic
        unsigned char* packet = (unsigned char*)&m_buffer[0];
//         uint32_t data_check_sum = ntohl(*reinterpret_cast<const uint32_t*>(packet + total_len - 4));
//         uint32_t local_check_sum = codec::Adler32((const char*)packet, total_len - 4);
//         if (data_check_sum != local_check_sum)
//         {
//             rel = -1;
//             //SNLOG(kError, "bad data check sum");
//             break;
//         }

        uint8 *body = reinterpret_cast<uint8*>(packet + packet_head_len);

        message.assign((char*)body, pb_size);

        m_buffer.erase(m_buffer.begin(), m_buffer.begin() + total_len);
        if (m_buffer.size() > packet_head_len)
        {
            rel = 1;
        }
        else
            rel = 0;


    }while (0);

#if 0
    if (rel < 0)
    {
       SNLOG((kError) << "parse packet return false.";
    }
#endif

    _VMProtectEnd();
    return rel;
}

std::string SqParser::encrypt_data(uint64_t ulpkey, char* data, uint32 size, string& send_key)
{
    _VMProtectBegin(__FUNCTION__);
    string send_data(data, size);
    return safe::ISQSafeModel::helper()->php_encryptx_stream(send_data, m_encry_key, send_key);
    _VMProtectEnd();
}

std::string SqParser::decrypt_data(uint64_t ulpkey, uint8* data, uint32 size, const string& private_key)
{
    _VMProtectBegin(__FUNCTION__);
    std::string cipher((char *)data, size);
    std::string send_key = std::to_string(ulpkey);
    return safe::ISQSafeModel::helper()->php_decryptx(cipher, private_key, send_key);
    _VMProtectEnd();
}

void SqParser::make_send_packet(uint32_t message_id, std::string& msg_data)
{
	_VMProtectBegin(__FUNCTION__);
	int len = 0;
	unsigned char *buf = nullptr;
	uint32_t encrypt_size = 0;
	string output;
	uint32_t input_size = 0;
    uint32_t check_sum = 0;

    // 加密处理
    input_size = msg_data.size();
    input_size += AES_BLOCK_LEN;
    encrypt_size = input_size;
    len = sizeof(SQHeader) + encrypt_size;

	buf = new unsigned char[len + 4];
	auto header = reinterpret_cast<SQHeader*>(buf);
	header->signature = htonl(m_signature);
	header->message_id = htonl(message_id);
    string pkey;
    header->private_key = net::hl64ton(_strtoui64(pkey.c_str(), nullptr, 10));
    len = sizeof(SQHeader) + msg_data.size();
    memcpy(&header->body, msg_data.c_str(), msg_data.size());
    header->length = htonl(len + 4);
    header->pb_length = htonl(msg_data.size());
    check_sum = htonl(codec::Adler32((const char*)buf, len));
    auto buf_pos = buf + len;
    memcpy(buf_pos, &check_sum, 4);
    msg_data.assign((char*)buf, len+4);
	SAFE_DELETE(buf);
	_VMProtectEnd();
}

}}