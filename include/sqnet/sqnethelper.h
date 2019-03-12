/*---------------------------------------------------------------------------*/
/*  sqnethelper.h                                                            */
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

#ifndef SNQU_NETWORK_HELPER_H
#define SNQU_NETWORK_HELPER_H

#include "sqnet.h"
#include "sqnethttp.h"
#include <unordered_map>
#include <deque>
#include <functional>
#include <memory>
#include <google/protobuf/message.h>
#include <sqstd/sqtaskthread.h>



#pragma warning(push)
#pragma warning(disable: 4251)


using namespace snqu;
namespace snqu
{
	namespace net
	{
		namespace helper
		{
			struct MessageContext
			{
				uint32_t m_message_id;
				std::pair<function<void (uint32_t, google::protobuf::Message*)>, google::protobuf::Message*> m_message;

				inline MessageContext(std::pair<function<void (uint32_t, google::protobuf::Message*)>, google::protobuf::Message*> message)
				{
					m_message = message;
				}
			};

			class SNQU_API Connection
				: public IConnection
			{
			public:
				Connection();
				virtual ~Connection() override = 0;

			public:
				virtual void connect() override final;
				virtual void send(const char* buf, size_t len) override final;
				virtual void close() override final;

			protected:
				virtual void set_proxy(IConnection* proxy) override final;

			protected:
				virtual void on_error(int errid, const char* errmsg) override;
				virtual void on_connected(const char* addr, int port, uint64_t peer_id) override;
				virtual size_t on_received(const char* buf, size_t len, IService *user_service) override;
				virtual void on_sent() override;
				virtual void on_disconnected() override;

			public:
				void heart_beat();
				seconds get_heart_beat();

			private:
				IConnection* m_proxy;
				seconds m_last_heartbeated;
				
			};

			class SNQU_API Service
				: public IService
			{
			public:
				Service();
				virtual ~Service() override = 0;

			public:
				void init();
				void release();

			protected:
				virtual IConnection* create_connection() override;
				
				virtual bool register_message_logger(uint32_t message_id, const function<bool (uint32_t, google::protobuf::Message*)>& callback);

			protected:
				virtual void on_registered() override;
				virtual bool on_started() override;
				virtual void on_heartbeat(const milliseconds& now, const milliseconds& delta) override;
				virtual bool on_stopped() override;
				virtual void on_unregistered() override;

			public:
				void add_user_packet(uint32_t message_id, 
								     const function<void (uint32_t, google::protobuf::Message*)>& callback, 
									 google::protobuf::Message* factory = nullptr);


			private:
				typedef unordered_map<uint32_t, function<bool (uint32_t, google::protobuf::Message*)>> logger_map_t;
				typedef std::shared_ptr<MessageContext> MessageContext_Ptr;
 				
				snqu::TaskThread<MessageContext_Ptr> m_task_thread;
				logger_map_t m_logger_map;

			private:
				void detect_packet_proc(const MessageContext_Ptr& msg);

			private:
				bool m_started;
				
			};

			class SNQU_API HTTPRequest
				: public IHTTPRequest
			{
			public:
				HTTPRequest(const string& action);
				HTTPRequest(const string& action, const string& body);
				virtual ~HTTPRequest() override;

			public:
				bool set_action(const string& action);
				bool clear_headers();
				bool add_header(const string& key, const string& value);
				bool set_body(const string& body);
				bool set_query(const string& key, const string& value);
				bool set_post(const string& key, const string& value);

			public:
				virtual string get_method() const override;
				virtual string get_action() const override;
				virtual uint32_t get_header_count(const string& key) const override;
				virtual string get_header(const string& key, uint32_t index = 0) const override;
				virtual string get_headers() const override;
				virtual string get_body() const override;
				virtual bool has_query(const string& key) const override;
				virtual string get_query(const string& key) const override;
				virtual bool has_post(const string& key) const override;
				virtual string get_post(const string& key) const override;

			private:
				string m_action;
				unordered_map<string, deque<string>> m_headers;
				string m_body;
				unordered_map<string, string> m_queries;
				unordered_map<string, string> m_posts;
			};

			class SNQU_API HTTPResponse
				: public IHTTPResponse
			{
			public:
				HTTPResponse(uint32_t status);
				HTTPResponse(uint32_t status, const string& body);
				virtual ~HTTPResponse() override;

			public:
				bool set_status(uint32_t status);
				bool clear_headers();
				bool add_header(const string& key, const string& value);
				bool set_body(const string& body);

			public:
				virtual uint32_t get_status() const override;
				virtual uint32_t get_header_count(const string& key) const override;
				virtual string get_header(const string& key, uint32_t index = 0) const override;
				virtual string get_body() const override;

			private:
				uint32_t m_status;
				unordered_map<string, deque<string>> m_headers;
				string m_body;
			};

			class SNQU_API HTTPConnection
				: public IHTTPConnection
			{
			public:
				HTTPConnection();
				virtual ~HTTPConnection() override = 0;

			public:
				virtual void connect() override;
				virtual void send(const char* buf, size_t len) override;
				virtual void close() override;
				virtual void send(const IHTTPRequest* request) override;
				virtual void send(const IHTTPResponse* response) override;

			protected:
				virtual void set_proxy(IConnection* proxy) override final;

			protected:
				virtual void on_error(int errid, const char* errmsg) override;
				virtual void on_connected(const char* addr, int port, uint64_t peer_id) override;
				virtual size_t on_received(const char* buf, size_t len, IService *user_service) override;
				virtual void on_sent() override;
				virtual void on_disconnected() override;
				virtual IHTTPResponse* on_request(const IHTTPRequest* request) override;
				virtual void on_response(const IHTTPResponse* response) override;

			private:
				IHTTPConnection* m_proxy;
			};
		}
	}
}

#pragma warning(pop)

#endif // SNQU_NETWORK_HELPER_H ///:~
