#pragma once
#include <sqstd/sqinc.h>
#include <memory>
#include <sqstd/sqtaskthread.h>
#include <sqnet/sqnethelper2.h>
#include <HPSocket/bufferptr.h>
#include <HPSocket/HPSocket.h>
#include <HPSocket/SocketInterface.h>
#include <sqnet/sqnet2.h>
#include <sqstd/sqtimerthread.h>
#include <sqstd/sqvecbuffhelper.h>
#include <codec/sqcodec.h>
#include <sqnet/sqcrypthelper.h>
#include <sqsafe/sqsafemodel.h>
#include <vmp/sqvmsdk.h>
#include <codec/sqcodec.h>

using namespace snqu::safe;
namespace snqu{ namespace net {

    typedef function<void(uint32_t, std::string message)> callback_func_center_t;
    typedef unordered_map<uint32_t, callback_func_center_t> message_map_t;

	class SqParser
	{
	public:
		SqParser(uint32_t packet_signature = SIGNATURE);
		~SqParser();
		void add_buffer(const char* buffer, unsigned int len);
		int pop_packet(uint32_t& message_id, std::string& message);
        std::string encrypt_data(uint64_t ulpkey, char* data, uint32 size, string& send_key);
		std::string decrypt_data(uint64_t ulpkey, uint8* data, uint32 size, const string& private_key);
        void make_send_packet(uint32_t message_id, std::string& msg_data);
        void clear_encry_key();

        std::function<void(void)> m_on_get_seesion_cb;
        message_map_t m_message_map;
        std::string m_channel_id;
    private:
        std::vector<char> m_buffer;
        std::string m_encry_key;
        
        unsigned int cur_size;
		uint32_t m_signature;
	};

}}