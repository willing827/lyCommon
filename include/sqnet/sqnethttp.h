/*---------------------------------------------------------------------------*/
/*  sqnethttp.h                                                              */
/*                                                                           */
/*  History                                                                  */
/*      05/28/2015                                                           */
/*                                                                           */
/*  Author                                                                   */
/*      Tan Wuliang                                                          */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*---------------------------------------------------------------------------*/

#ifndef SNQU_NETWORK_HTTP_H
#define SNQU_NETWORK_HTTP_H

#include "sqnet.h"
#include <cstdint>

namespace snqu
{
	namespace net
	{
		class SNQU_API IHTTPRequest
		{
		public:
			virtual ~IHTTPRequest() = 0;

		public:
			virtual string get_method() const = 0;
			virtual string get_action() const = 0;
			virtual uint32_t get_header_count(const string& key) const = 0;
			virtual string get_header(const string& key, uint32_t index = 0) const = 0;
			virtual string get_headers() const = 0;
			virtual string get_body() const = 0;
			virtual bool has_query(const string& key) const = 0;
			virtual string get_query(const string& key) const = 0;
			virtual bool has_post(const string& key) const = 0;
			virtual string get_post(const string& key) const = 0;
		};

		class SNQU_API IHTTPResponse
		{
		public:
			virtual ~IHTTPResponse() = 0;

		public:
			virtual uint32_t get_status() const = 0;
			virtual uint32_t get_header_count(const string& key) const = 0;
			virtual string get_header(const string& key, uint32_t index = 0) const = 0;
			virtual string get_body() const = 0;
		};

		class SNQU_API IHTTPConnection
			: public IConnection
		{
		public:
			virtual ~IHTTPConnection() override = 0;

		public:
			virtual void send(const IHTTPRequest* request) = 0;
			virtual void send(const IHTTPResponse* response) = 0;

		protected:
			virtual IHTTPResponse* on_request(const IHTTPRequest* request) = 0;
			virtual void on_response(const IHTTPResponse* response) = 0;
		};
	}
}

#endif // SNQU_NETWORK_HTTP_H ///:~
