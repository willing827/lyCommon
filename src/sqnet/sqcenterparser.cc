#include <sqnet/sqcenterparser.h>
#include <codec/sqcodec.h>
#include <sqnet/sqnetapi.h>
#define ktLoggerId "CenterNetLib"

#define BUFF_SIZE_PART 10240

using namespace snqu::safe;
namespace snqu{ namespace net2 {

    ULONG64  hl64ton(ULONG64  host)   
    {   
        ULONG64   ret = 0;   
        ULONG   high,low;
        low   =   host & 0xFFFFFFFF;
        high   =  (host >> 32) & 0xFFFFFFFF;
        low   =   htonl(low);   
        high   =   htonl(high);   
        ret   =   low;
        ret   <<= 32;   
        ret   |=   high;   
        return   ret;   
    }

    ULONG64  ntohl64(ULONG64  host)   
    {   
        ULONG64   ret = 0;   
        ULONG   high,low;
        low    =   host & 0xFFFFFFFF;
        high   =  (host >> 32) & 0xFFFFFFFF;
        low    =   ntohl(low);   
        high   =   ntohl(high);   
        ret    =   low;
        ret   <<= 32;   
        ret   |=   high;   
        return   ret;   
    }

CenterParser::CenterParser(uint32_t packet_signature /*= SIGNATURE*/)
{
	m_signature = packet_signature;
	cur_size = BUFF_SIZE_PART;
	m_buffer.reserve(cur_size);
	clear_encry_key();
    is_v6_ver = false;
}

void CenterParser::clear_encry_key()
{
    char dls[] = {'*','&','#',')','s','d','f','(','!','*','1','1','@',')','#','\0'};
    m_encry_key = dls;
}

CenterParser::~CenterParser()
{
    m_buffer.clear();

    for (auto& item : m_message_map)
    {
        SAFE_DELETE(item.second.second);
    }

    m_message_map.clear();
}

void CenterParser::add_buffer(const char* buffer, unsigned int len)
{
    if (m_buffer.size() + len > cur_size)
    {
        m_buffer.reserve(cur_size + BUFF_SIZE_PART);
        cur_size += BUFF_SIZE_PART;
    }
    buffer::write(m_buffer, m_buffer.size(), buffer, len);
}

google::protobuf::Message* CenterParser::get_message_factory(uint32_t message_id)
{
    auto pos = m_message_map.find(message_id);
    if (pos != m_message_map.end())
        return pos->second.second;

    return nullptr;
}

int CenterParser::pop_packet(uint32_t& message_id, proto_msg_ptr& message)
{
    _VMProtectBegin(__FUNCTION__);
    int rel = 0;
    message_id = 0;
    uint32_t pb_size = 0;
    uint32_t body_len(0), total_len(0);
    size_t packet_head_len = sizeof(SQHeaderEx);

    do
    {
        // 数据太少, 不足包头数据
        if (m_buffer.size() < packet_head_len)
        {
           //SNLOG((kDebug) << " packet not enough for packet_head_len.";
            rel = 0;
            break;
        }

        //  检查信令头的标识是否正确
        SQHeaderEx* header = reinterpret_cast<SQHeaderEx*>(&m_buffer[0]);
        if (ntohl(header->signature) != m_signature)
        {
            // 返回HR_ERROR表示要终止此连接
            rel = false;
            m_buffer.clear();
           //SNLOG((kError) << " packet SIGNATURE error.";
            break;
        }

        // 计算包长
        // 数据负载大小，包括checksum
        total_len = ntohs(header->length);
        body_len = total_len - packet_head_len - 4;
        message_id = ntohs(header->message_id);
        pb_size = ntohs(header->pb_length);

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
        uint32_t data_check_sum = ntohl(*reinterpret_cast<const uint32_t*>(packet + total_len - 4));
        uint32_t local_check_sum = codec::Adler32((const char*)packet, total_len - 4);
        if (data_check_sum != local_check_sum)
        {
            rel = false;
           //SNLOG(kError, "bad data check sum");
            break;
        }

        uint8 *body = reinterpret_cast<uint8*>(packet + packet_head_len);

        if (net2::InternalQueryKeyMsg == message_id)
        {
            // 保存此次通讯的key
            m_encry_key.assign(decrypt_data(ntohl64(header->private_key), body, body_len, std::string(m_encry_key)).c_str(), pb_size);
            m_buffer.erase(m_buffer.begin(), m_buffer.begin() + total_len);
            if (!m_encry_key.empty())
            {
                m_on_get_seesion_cb();
               //SNLOG(kInfo, "received encry_key");
               //SNLOG(kDebug, "received encry_key is [%s]", m_encry_key.c_str());
            }
            else
            {
                rel = -1;
               //SNLOG(kError, "received encry_key empty");
                break;
            }
            break;
        }

        auto msg = get_message_factory(message_id);
        if (msg != nullptr)
        {
            bool pbParseOk = false;
            uint8_t *pbArray = nullptr;
            auto pb_msg = msg ? msg->New() : nullptr;
            proto_msg_ptr pro_msg(pb_msg, [](google::protobuf::Message *p){ delete p;});

            if (0 != pb_size)
            {
                std::string proto_data(decrypt_data(ntohl64(header->private_key), body, body_len, m_encry_key).c_str(), pb_size);
                pbParseOk = pro_msg->ParseFromString(proto_data);
            }
            else
                pbParseOk = pro_msg->ParseFromString("");


            if (pbParseOk)
            {
                message = pro_msg;
                m_buffer.erase(m_buffer.begin(), m_buffer.begin() + total_len);
				if (m_buffer.size() > packet_head_len)
		        {
		            rel = 1;
		        }
            }
            else
            {
                // protoBuff 解析失败
               //SNLOG((kError) << " receive_packet parse packet error, msg: " << message_id;
                if (total_len <= m_buffer.size())
                    m_buffer.erase(m_buffer.begin(), m_buffer.begin() + total_len);
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
           //SNLOG(kWarning, "ignored message id:[%d]", message_id);
            // 没找到的包类型，暂时忽略掉
            m_buffer.erase(m_buffer.begin(), m_buffer.begin() + total_len);
        }
    }while (0);

    if (rel < 0)
    {
       //SNLOG((kError) << "parse packet return false.";
    }

    _VMProtectEnd();
    return rel;
}

std::string CenterParser::encrypt_data(uint64_t ulpkey, char* data, uint32 size, string& send_key)
{
    _VMProtectBegin(__FUNCTION__);
    string send_data(data, size);
    return safe::ISQSafeModel::helper()->php_encryptx_stream(send_data, m_encry_key, send_key);
    _VMProtectEnd();
}

std::string CenterParser::decrypt_data(uint64_t ulpkey, uint8* data, uint32 size, const string& private_key)
{
    _VMProtectBegin(__FUNCTION__);
    std::string cipher((char *)data, size);
    std::string send_key = std::to_string(ulpkey);
    return safe::ISQSafeModel::helper()->php_decryptx(cipher, private_key, send_key);
    _VMProtectEnd();
}

void CenterParser::make_send_packet(uint32_t message_id, std::string& msg_data)
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
    len = sizeof(SQHeaderEx) + encrypt_size;

	buf = new unsigned char[len + 4];
	auto header = reinterpret_cast<SQHeaderEx*>(buf);
	header->signature = htonl(m_signature);
	header->message_id = htons(message_id);
    if (is_v6_ver)
        header->site_id = hl64ton(codec::Adler32(m_site_id));
    else
        header->site_id = hl64ton(_strtoui64(m_site_id.c_str(), nullptr, 10));
    string pkey;
    std::string encode_body = safe::ISQSafeModel::helper()->php_encryptx_stream(msg_data, m_encry_key, pkey);
    header->private_key = hl64ton(_strtoui64(pkey.c_str(), nullptr, 10));
    len = sizeof(SQHeaderEx) + encode_body.size();
    memcpy(&header->body, encode_body.c_str(), encode_body.size());
    header->length = uint16_t(htons(len + 4));
    header->pb_length = uint16_t(htons(msg_data.size()));
    /*std::string pb_data = net::BinToHex(msg_data);
    printf("m_encry_key:%s private_key:%s pkt_len:%d\r\n", m_encry_key.c_str(), pkey.c_str(), len + 4);
    printf("pb_data %s len:%d\r\n", pb_data.c_str(), msg_data.size());
    std::string ency_data = net::BinToHex(encode_body);
    printf("ency_data %s len:%d\r\n", ency_data.c_str(), encode_body.size());*/
    header->main_ver = 1;
    header->sub_ver = 1;
    check_sum = htonl(codec::Adler32((const char*)buf, len));
    auto buf_pos = buf + len;
    memcpy(buf_pos, &check_sum, 4);
    msg_data.assign((char*)buf, len+4);
	SAFE_DELETE(buf);
	_VMProtectEnd();
}

}}