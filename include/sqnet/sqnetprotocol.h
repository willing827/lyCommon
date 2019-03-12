/*---------------------------------------------------------------------------*/
/*  sqnetprotocol.h                                                          */
/*                                                                           */
/*  History                                                                  */
/*      05/29/2015                                                           */
/*                                                                           */
/*  Author                                                                   */
/*      Tan Wuliang                                                          */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*---------------------------------------------------------------------------*/

#ifndef SNQU_NETWORK_PROTOCOL_H
#define SNQU_NETWORK_PROTOCOL_H

#include "sqnethelper.h"
#include <functional>
#include <unordered_map>
#include <google/protobuf/message.h>

namespace snqu
{
	namespace net
	{
#define  SIGNATURE 0x87654321
#define  MAXLENGTH (10*1024*1024)
//#define  HEADERSIZE sizeof(Header)

#pragma pack(push, 1)

#pragma warning(push)
#pragma warning(disable: 4200)
		struct Header
		{
			uint32_t signature;
			uint32_t message_id;
			uint32_t length;
			uint32_t reserved;
			char body[0];
			// uint32_t checksum;
		};
#pragma warning(pop)

#pragma pack(pop)

		SNQU_API class Protocol
			: public helper::Connection
		{
		public:
			Protocol();
			virtual ~Protocol() override = 0;

		public:
			void send(uint32_t message_id, google::protobuf::Message* message = nullptr);
			virtual bool register_message(uint32_t message_id, const function<void (uint32_t, google::protobuf::Message*)>& callback, google::protobuf::Message* factory = nullptr);
		
		protected:
			virtual size_t on_received(const char* buf, size_t len, IService *user_service) override;
			size_t parse_packet(char *buf, size_t len, size_t& packet_len);

		private:
			unordered_map<uint32_t, pair<function<void (uint32_t, google::protobuf::Message*)>, google::protobuf::Message*>> m_message_map;
		};
	}
}

#endif // SNQU_NETWORK_PROTOCOL_H ///:~
