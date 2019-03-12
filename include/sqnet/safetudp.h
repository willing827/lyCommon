#ifndef SAFETUDP_H
#define SAFETUDP_H

#include <sqstd/sqinc.h>
#include <sqnet/sqnethelper2.h>
#include <Raknet/RakPeerInterface.h>
#include <Raknet/MessageIdentifiers.h>
#include <Raknet/RakSleep.h>
#include <Raknet/BitStream.h>
#include <Raknet/FileList.h>
#include <Raknet/FileListTransfer.h>
#include <Raknet/FileListTransferCBInterface.h>
#include <Raknet/IncrementalReadInterface.h>
#include <Raknet/FileListNodeContext.h>
#include <Raknet/RakNetTypes.h>
#include <sqstd/sqtimerthread.h>
#include <sqstd/sqtaskthread.h>
#include <protobuf/message.h>
#include <atomic>

typedef std::shared_ptr<google::protobuf::Message> proto_msg_ptr;

namespace snqu { namespace net2 {
	typedef struct _MiniUdpPacket
	{
		uint8 packetIdentifier;
		SQHeader sqpacket;
	} MiniUdpPacket;

	typedef RakNet::SystemAddress SQUdpAddress;
	typedef RakNet::FileListTransferCBInterface SQFileTransCallback;
	typedef std::shared_ptr<RakNet::Packet> SQSTUdpPacketPtr;

	struct SafeUdpConfig
	{
		uint32 m_port;				// 端口,如果是服务端，则是绑定的端口,
									// 如果是客户端，则是远程端口,
									// 如果是服务端模式，并且端口为0,则会随机产生可用的端口。

		string m_remote_ip;			// 远端IP
		bool m_is_server;			// 是否是服务端模式
		string m_password;			// 连接密码
		int32 m_max_connections;	// 最大支持的同时连接数
		bool  m_need_trans_file;	// 是否需要传输文件
		bool  m_has_progress;		// 传输文件的时候，是否需要进度通知

		SafeUdpConfig()
		{
			m_port = 0;
			m_remote_ip = "";
			m_password = "";
			m_is_server = false;
			m_max_connections = 0;
			m_need_trans_file = false;
			m_has_progress = false;
		}

		void copy(const SafeUdpConfig& right)
		{
			m_port = right.m_port;
			m_remote_ip = right.m_remote_ip;
			m_is_server = right.m_is_server;
			m_password = right.m_password;
			m_max_connections = right.m_max_connections;
			m_need_trans_file = right.m_need_trans_file;
			m_has_progress = right.m_has_progress;
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// 文件传输辅助类定义, SQUdpTransFileHelper
	class SQUdpTransFileHelper
	{
	public:
		SQUdpTransFileHelper();
		virtual ~SQUdpTransFileHelper();

		void add_trans_data(uint8_t *data, uint32_t len, const char *uniqueId);
		void clear();
		RakNet::FileList* operator &();

	private:
		RakNet::FileList m_file_list;
	};

	//////////////////////////////////////////////////////////////////////////
	// UDP通讯类定义
	typedef function<void (RakNet::Packet*, proto_msg_ptr)> udp_callback_func_t;
	typedef function<bool(uint32_t, proto_msg_ptr)> udpPreCallbackFunc_t;
	class SQSafetUDP : public SQFileTransCallback
	{
		enum {fs_default_time_out = 10000, fs_max_port_num = 65534};
		enum {udp_client_retry_count = 1};
		enum {
			SST_STATUS_READY = 0,
			SST_STATUS_STARTING,
			SST_STATUS_RUNNING,
			SST_STATUS_STOPPED,
		};

	public:
		// 客户端模式下状态定义
		enum {
			CST_STATUS_READY = 0,
			CST_STATUS_CONNECTED,
			CST_STATUS_DISCONNECTED,
			CST_STATUS_CLOSED,
		};

	public:
		SQSafetUDP();
		virtual ~SQSafetUDP();

	public:
		// 启动 
		bool start(const SafeUdpConfig& config);
		
		// 停止
		void stop();
		
		// 获取绑定的端口
		uint32 get_bind_port();		

		// 获取当前负载
		uint32 get_load_count();	

		// 注册消息
		bool register_message(uint32_t message_id, 
			const udp_callback_func_t& callback, 
			google::protobuf::Message* factory = nullptr);

		// 注册预处理消息
		void register_precallback(udpPreCallbackFunc_t callback);

		// 发送数据
		bool send_packet(const RakNet::AddressOrGUID systemIdentifier, 
			bool encrypt,
			uint32_t message_id,
			google::protobuf::Message *msg);

		// 发送文件或者大数据
		void trans_data(SQUdpTransFileHelper& fileHelper, 
			SQUdpAddress systemAddress);

		// 添加文件传输回调
		void add_file_callback(SQFileTransCallback *callback);

		void ping(RakNet::SystemAddress& target);

		int32 client_connection_status();

	public:
		// 回调函数
		virtual bool OnFile(OnFileStruct *onFileStruct) override;
		virtual void OnFileProgress(FileProgressStruct *fps) override;
		virtual bool OnDownloadComplete(DownloadCompleteStruct *dcs) override;

	protected:
		virtual void on_close(RakNet::Packet *p) = 0;
		virtual void on_connectd(RakNet::Packet *p) = 0;

	protected:
		void run_proc();
		bool listen_rand_port();
		bool connect();

		const string& get_public_key();
		string decrypt_packet(uint64_t ulpkey, uint8* data, uint32 size);
		void on_receive_packet(RakNet::Packet *packet, uint8 msg_id);
		void add_user_packet(RakNet::Packet* packet, uint32 message_id, proto_msg_ptr Message);
		void detect_packet_proc(const std::tuple<SQSTUdpPacketPtr, uint32_t, proto_msg_ptr>& task);

	private:
		RakNet::RakPeerInterface *m_rak_interface;
		RakNet::SystemAddress m_remote_rak;

		atomic_bool   m_started;
		bool   m_stop_sign;
		uint32 m_port;
		bool   m_conn_lost;
		int32  m_status;

		volatile uint32 m_load_count;

		SafeUdpConfig m_config;
		unordered_map<uint32_t, pair<udp_callback_func_t, google::protobuf::Message*> > m_message_map;

		snqu::TaskThread<std::tuple<SQSTUdpPacketPtr, uint32_t, proto_msg_ptr> > m_task_thread;
		std::thread m_run_thread;

		RakNet::FileListTransfer *m_fileTransfer;
		RakNet::IncrementalReadInterface m_incrementalReadInterface;
		
		SQFileTransCallback *m_callback;
		udpPreCallbackFunc_t m_preTranslateFunc;
		std::recursive_mutex m_lock;

		int32 m_client_status;
		std::recursive_mutex m_msgmap_lock;
	};
}}


#endif //SAFETUDP_H