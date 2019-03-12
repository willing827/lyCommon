#include <sqnet/sqpacketparser.h>
#include <codec/sqcodec.h>
#define ktLoggerId "ViewNetLib"

#define BUFF_SIZE_PART 10240

using namespace snqu::safe;
namespace snqu{ namespace net2 {

PacketParser::PacketParser()
{
    cur_size = BUFF_SIZE_PART;
    m_buffer.reserve(cur_size);
    clear_encry_key();
}

void PacketParser::clear_encry_key()
{
    char dls[] = {'*','&','#',')','s','d','f','(','!','*','1','1','@',')','#','\0'};
    m_encry_key = dls;
}

PacketParser::~PacketParser()
{
    m_buffer.clear();

    for (auto& item : m_message_map)
    {
        SAFE_DELETE(item.second.second);
    }

    m_message_map.clear();
}

void PacketParser::add_buffer(const char* buffer, unsigned int len)
{
    if (m_buffer.size() + len > cur_size)
    {
        m_buffer.reserve(cur_size + BUFF_SIZE_PART);
        cur_size += BUFF_SIZE_PART;
    }
    buffer::write(m_buffer, m_buffer.size(), buffer, len);
}

google::protobuf::Message* PacketParser::get_message_factory(uint32_t message_id)
{
    auto pos = m_message_map.find(message_id);
    if (pos != m_message_map.end())
        return pos->second.second;

    return nullptr;
}

int PacketParser::pop_packet(uint32_t& message_id, proto_msg_ptr& message)
{
    _VMProtectBegin(__FUNCTION__);
    int rel = 0;
    message_id = 0;
    uint32_t pb_size = 0;
    uint32_t body_len(0), total_len(0);
    size_t packet_head_len = sizeof(SQHeader);

    do
    {
        // ����̫��, �����ͷ����
        if (m_buffer.size() < packet_head_len)
        {
           //SNLOG((kDebug) << " packet not enough for packet_head_len.";
            break;
        }

        //  �������ͷ�ı�ʶ�Ƿ���ȷ
        SQHeader* header = reinterpret_cast<SQHeader*>(&m_buffer[0]);
        if (header->signature != SIGNATURE) 
        {
            // ����HR_ERROR��ʾҪ��ֹ������
            rel = -1;
            m_buffer.clear();
           //SNLOG((kError) << " packet SIGNATURE error.";
            break;
        }

        // �������
        // ���ݸ��ش�С������checksum
        total_len = header->length;
        body_len = total_len - packet_head_len - 4;
        message_id = header->message_id;
        pb_size = header->pb_length;

        // �����ȳ�����󳤶�, �յ��Ƿ�����, �����쳣
        if (body_len > 1024*1024*50)
        {
           //SNLOG((kError) << " too long packet, invalidate.";
            rel = -1;
            m_buffer.clear();
            break;
        }

        // ����̫��, ����һ����������
        if (total_len > (int)m_buffer.size())
        {
			break;
        }

        // Add checksum logic
        unsigned char* packet = (unsigned char*)&m_buffer[0];
//         uint32_t data_check_sum = *reinterpret_cast<const uint32_t*>(packet + total_len - 4);
//         uint32_t local_check_sum = adler32(packet, total_len - 4);
//         if (data_check_sum != local_check_sum)
//         {
//             rel = false;
//            //SNLOG(kError, "bad data check sum");
//             break;
//         }
        
        uint8 *body = reinterpret_cast<uint8*>(packet + packet_head_len);
        if (net2::InternalQueryKeyMsg == message_id)
        {
            // ����˴�ͨѶ��key
            if (body_len != AES_KEY_LEN*2)
            {
                rel = -1;
                break;
            }
            m_encry_key = "";
            m_encry_key.resize(AES_KEY_LEN);
            for (uint32 i = 0; i < AES_KEY_LEN; i++)
                *reinterpret_cast<uint8*>(&m_encry_key[i]) = body[i];

           //SNLOG(kDebug, "received encry_key is [%x]", body);
            // ����ͨѶKEY
            net2::SQEncryptHelper::simple_encrypt_key(m_encry_key, AES_KEY_LEN);
            m_buffer.erase(m_buffer.begin(), m_buffer.begin() + total_len);
            if (!m_encry_key.empty())
            {
                m_on_get_seesion_cb();
               //SNLOG(kInfo, "received encry_key");
                if (m_buffer.size() > packet_head_len)
                {
                    rel = 1;
                    break;
                }
            }
            else
            {
                rel = -1;
               //SNLOG(kError, "received encry_key empty");
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
                std::string proto_data = decrypt_data(header->private_key, body, body_len, m_encry_key);
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
                    break;
		        }
            }
            else
            {
                // protoBuff ����ʧ��
               //SNLOG((kError) << " receive_packet parse packet error, msg: " << message_id;
                if (total_len <= m_buffer.size())
                    m_buffer.erase(m_buffer.begin(), m_buffer.begin() + total_len);
                else
                {
                    // ����İ���ʱֻ��������
                    m_buffer.clear();
                    break;
                }
            }
        }
        else
        {
           //SNLOG(kWarning, "ignored message id:[%d]", message_id);
            // û�ҵ��İ����ͣ���ʱ���Ե�
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

std::string PacketParser::encrypt_data(uint64_t& ulpkey, const std::string& en_data)
{
    _VMProtectBegin(__FUNCTION__);
    std::string temp_key, ret_data;
    ret_data = safe::ISQSafeModel::std_algo()->aes_encryptx(en_data, m_encry_key, temp_key);
    ulpkey= _strtoui64(temp_key.c_str(), nullptr, 10);
    return ret_data;
    _VMProtectEnd();
}

std::string PacketParser::decrypt_data(uint64_t ulpkey, uint8* data, uint32 size, const string& private_key)
       {
    _VMProtectBegin(__FUNCTION__);
    std::string cipher((char *)data, size);
    std::string send_key = std::to_string(ulpkey);
    return safe::ISQSafeModel::std_algo()->aes_decryptx(cipher, private_key, send_key);
    _VMProtectEnd();
}

void PacketParser::make_send_packet(uint32_t message_id, std::string& msg_data)
{
	_VMProtectBegin(__FUNCTION__);
	int len = 0;
	unsigned char *buf = nullptr;
	uint32_t encrypt_size = 0;
	string output;
	uint32_t input_size = 0;
    uint32_t check_sum = 0;

    // ���ܴ���
    input_size = msg_data.size();
    input_size += AES_BLOCK_LEN;
    encrypt_size = input_size;
    len = sizeof(SQHeader) + encrypt_size;

	buf = new unsigned char[len + 4];
	auto header = reinterpret_cast<SQHeader*>(buf);
	header->signature = SIGNATURE;
	header->message_id = message_id;
	header->reserved = 0;
    SET_PACKET_ENCRYPT_BIT(header->option);
    std::string encode_body = encrypt_data(header->private_key, msg_data);
    len = sizeof(SQHeader) + encode_body.size();
    memcpy(&header->body, encode_body.c_str(), encode_body.size());
    header->length = len + 4;
    header->pb_length = msg_data.size();
    check_sum = codec::Adler32((const char*)buf, len);
    auto buf_pos = buf + len;
    memcpy(buf_pos, &check_sum, 4);
    msg_data.assign((char*)buf, len+4);
	SAFE_DELETE(buf);
	_VMProtectEnd();
}

}}