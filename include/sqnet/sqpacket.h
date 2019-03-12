#pragma once
#include <stdint.h>


namespace snqu{ namespace net {

#define  SIGNATURE 0x87654321
#define  MAXLENGTH (10*1024*1024)
#define  HEADERSIZE sizeof(SQHeader)

#pragma pack(push, 1)
#pragma warning(push)
#pragma warning(disable: 4200)
    struct SQHeader
    {
        uint32_t signature;
        uint32_t message_id;
        uint32_t length;
        uint32_t pb_length;
        uint32_t option;
        uint64_t private_key;
        uint32_t reserved;
        char body[0];
        // uint32_t checksum;
    };
#pragma warning(pop)
#pragma pack(pop)

    template<typename PktHeader>
    class SQPacket
    {
    public:
        SQPacket() 
        {
            m_expire_time = 0;
            ::memset(&m_pkt_header, 0, sizeof(PktHeader));
        }

        ~SQPacket() {}

        PktHeader* GetHeader()
        {
            return &m_pkt_header;
        }

        void SetHeader(const PktHeader* pkt_header)
        {
            if (pkt_header) {
                ::memcpy(&m_pkt_header, pkt_header, sizeof(PktHeader));
            }
        }

        /*
        * �ͷ��Լ�
        */
        virtual void Free() {
            delete this;
        }

        /*
        * ��װ
        *
        * @param output: Ŀ��buffer
        * @return �Ƿ�ɹ�
        */
        virtual bool Encode(DataBuffer *output) = 0;

        /*
        * �⿪
        *
        * @param input: Դbuffer
        * @param header: ���ݰ�header
        * @return �Ƿ�ɹ�
        */
        virtual bool Decode(DataBuffer *input, PktHeader *header) = 0;

        /*
        * ��ʱʱ��
        */
        int64_t GetExpireTime() const {
            return m_expire_time;
        }

        /*
        * ���ù���ʱ��
        *
        * @param milliseconds ������, 0Ϊ��������
        */
        void SetExpireTime(int milliseconds)
        {
//             if (milliseconds == 0) {
//                 milliseconds = 1000 * 86400;
//             }
//             m_expire_time = getTime() + static_cast<int64_t>(milliseconds);
        }

    private:
        PktHeader m_pkt_header; // ��ͷ��Ϣ
        int64_t m_expire_time;  // ������ʱ��
    };

}}