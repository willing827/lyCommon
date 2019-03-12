#include <sqnet/safetudp.h>
#include <sqnet/sqnethelper2.h>
#include <sqnet/sqcrypthelper.h>
#include <sqnet/sqnet2.h>
#include <sqwin/win/sqtools.h>
#include "netlog.h"
#include <sqsafe/sqsafemodel.h>
#include <vmp/sqvmsdk.h>

using namespace snqu::net2;
namespace snqu { namespace net2{
	//////////////////////////////////////////////////////////////////////////
	// 文件传输辅助类实现
	SQUdpTransFileHelper::SQUdpTransFileHelper() {}
	SQUdpTransFileHelper::~SQUdpTransFileHelper() {}
	void SQUdpTransFileHelper::add_trans_data(uint8_t *data, uint32_t len, const char *uniqueId)
	{
		m_file_list.AddData((const char*)data, len, uniqueId);
	}

	void SQUdpTransFileHelper::clear()
	{
		m_file_list.Clear();
	}

	RakNet::FileList* SQUdpTransFileHelper::operator &()
	{
		return &m_file_list;
	}

	//////////////////////////////////////////////////////////////////////////
	// UDP通讯封装类
	SQSafetUDP::SQSafetUDP()
	{
		m_rak_interface = nullptr;
		m_started = false;
		m_conn_lost = false;
		m_stop_sign = false;
		m_port = 0;
		m_load_count = 0;
		m_callback = nullptr;
		m_preTranslateFunc = nullptr;
		m_fileTransfer = nullptr;
		m_status = SST_STATUS_READY;
		m_client_status = CST_STATUS_READY;
	}

	SQSafetUDP::~SQSafetUDP()
	{
	}

	bool SQSafetUDP::start(const SafeUdpConfig& config)
	{
		m_stop_sign = false;
		if (m_started)
		{
		    //SNLOG(kFatal, "SQSafetUDP模块重复初始化!\r\n");
			return true;
		}
		else
		{
			std::lock_guard<std::recursive_mutex> lock(m_lock);
			m_status = SST_STATUS_STARTING;
			m_config.copy(config);
			m_rak_interface = RakNet::RakPeerInterface::GetInstance();
			if (!m_rak_interface)
			{
			    //SNLOG(kFatal,"SQSafetUDP模块无法初始化网络支持");
				goto __done;
			}

			if (config.m_need_trans_file)
			{
				if (!m_fileTransfer)
				{
					m_fileTransfer = new RakNet::FileListTransfer();
					if (!m_fileTransfer)
					{
					    //SNLOG(kFatal, "SQSafetUDP模块m_fileTransfer初始化失败");
						goto __done;
					}
				}

				m_rak_interface->AttachPlugin(m_fileTransfer);
				m_fileTransfer->StartIncrementalReadThreads(2);

				if (config.m_has_progress)
					m_rak_interface->SetSplitMessageProgressInterval(9);
			}

			if (config.m_is_server)
			{
				m_rak_interface->SetIncomingPassword(config.m_password.c_str(), config.m_password.length());
				m_rak_interface->SetTimeoutTime(fs_default_time_out, RakNet::UNASSIGNED_SYSTEM_ADDRESS);

				if (!listen_rand_port())
				{
					goto __done;
				}

				m_rak_interface->SetMaximumIncomingConnections(config.m_max_connections);
			}
			else
			{
				// 客户端模式
				RakNet::SocketDescriptor description(0, 0);
				description.socketFamily = AF_INET;

				if (RakNet::RAKNET_STARTED != m_rak_interface->Startup(1, &description, 1))
				{
				    //SNLOG(kFatal, "启动UDP网络模块失败: %d", GetLastError());
					goto __done;
				}

				if (!connect())
				{
					// We're done with the network
					goto __done;
				}

				// 状态，已连接
				m_client_status = CST_STATUS_CONNECTED;
			}

			m_task_thread.begin(std::bind(&SQSafetUDP::detect_packet_proc, this, std::placeholders::_1));
			m_run_thread = std::thread(std::bind(&SQSafetUDP::run_proc, this));

			m_started = true;
			m_status = SST_STATUS_RUNNING;
		}

__done:
		if (!m_started)
		{
			m_status = SST_STATUS_READY;
			stop();
		}

		return m_started;
	}

	bool SQSafetUDP::connect()
	{
		bool ok = false;
		for (int i = 0; i < udp_client_retry_count; i++)
		{
			m_remote_rak = ConnectBlocking(m_rak_interface, 
							m_config.m_remote_ip.c_str(), m_config.m_port, m_config.m_password.c_str());
			if (m_remote_rak == RakNet::UNASSIGNED_SYSTEM_ADDRESS)
			{
			    //SNLOG(kFatal, "连接UDPServer %s:%d失败.\r\n", m_config.m_remote_ip.c_str(), m_config.m_port);
				continue;
			}

			ok = true;
			m_conn_lost = false;
			break;
		}

		if (!ok)
		{
		    //SNLOG(kFatal, "连接UDP服务%s:%d失败，错误代码:%d\r\n", m_config.m_remote_ip.c_str(), m_config.m_port, GetLastError());
		}
		else
		{
			if (!m_config.m_is_server && m_config.m_need_trans_file)
			{
				m_fileTransfer->SetupReceive(this, false, m_remote_rak);
			}

		    //SNLOG(kTrace, "连接UDP服务%s:%d成功.\r\n", m_config.m_remote_ip.c_str(), m_config.m_port);
		}

		return ok;
	}

	uint32 SQSafetUDP::get_bind_port()
	{
		return m_port;
	}

	uint32 SQSafetUDP::get_load_count()
	{
		return m_load_count;
	}

	void SQSafetUDP::ping(RakNet::SystemAddress& target)
	{
		if (m_rak_interface != nullptr && !m_stop_sign)
		{
			if (m_config.m_is_server)
				m_rak_interface->Ping(target);
			else
				m_rak_interface->Ping(m_remote_rak);
		}
	}

	void SQSafetUDP::run_proc()
	{
		RakNet::Packet *packet = nullptr;
		uint8 msg_id = 0;

		while (!m_stop_sign)
		{
			// This sleep keeps RakNet responsive
			RakSleep(1);
			
			if (!m_rak_interface)
				break;

			for (packet = m_rak_interface->Receive(); 
				packet; 
				m_rak_interface->DeallocatePacket(packet), packet = m_rak_interface->Receive())
			{
				msg_id = GetPacketIdentifier(packet);
				if (msg_id >= ID_USER_PACKET_ENUM)
				{
					on_receive_packet(packet, msg_id);
				}
				else
				{
					// Check if this is a network message packet
					switch (msg_id)
					{
					case ID_DISCONNECTION_NOTIFICATION:
					    //SNLOG(kDebug, "ID_DISCONNECTION_NOTIFICATION from %s\n", packet->systemAddress.ToString(true));
						on_close(packet);
						break;

					case ID_NEW_INCOMING_CONNECTION:
						InterlockedIncrement(&m_load_count);
						on_connectd(packet);
						break;
						
					case ID_CONNECTION_LOST:
						// Couldn't deliver a reliable packet - i.e. the other system was abnormally
						// terminated
					    //SNLOG(kDebug, "ID_CONNECTION_LOST from %s\n", packet->systemAddress.ToString(true));
						InterlockedDecrement(&m_load_count);
						
						// 连接断开了
						m_client_status = CST_STATUS_DISCONNECTED;
						on_close(packet);
						break;

					default:
						break;
					}
				}

				if (m_stop_sign)
					break;
			}
		}
	}

	void SQSafetUDP::on_receive_packet(RakNet::Packet *packet, uint8 msg_id)
	{
		_VMProtectBegin(__FUNCTION__);
		bool parseOk = false;
		uint32_t message_id = 0;
		uint32_t pb_size = 0;
		uint32_t options = 0;
		MiniUdpPacket *Mini_Packet = (MiniUdpPacket *)packet->data;
		SQHeader* pHeader = (SQHeader*)&Mini_Packet->sqpacket;
		int32 required = 0;

		if (SIGNATURE != pHeader->signature)
			return;

		// 数据负载大小，包括checksum
		required = pHeader->length - sizeof(MiniUdpPacket);// - sizeof(uint32_t)/*checksum*/;
		message_id = pHeader->message_id;
		pb_size = pHeader->pb_length;
		options = pHeader->option;

		BYTE *body = (BYTE *)pHeader->body;
		uint16_t checksum = *reinterpret_cast<const uint16_t*>(body + required - sizeof(uint32_t));

		auto it = m_message_map.find(message_id);
		if (it != m_message_map.end())
		{
			std::lock_guard<std::recursive_mutex> lock(m_msgmap_lock);
			auto message = it->second.second ? it->second.second->New() : nullptr;
			proto_msg_ptr msg(message, [](google::protobuf::Message *p){ delete p;});

			if (PACKET_HAS_ENCRYPT_BIT(options))
			{
				string plaint_body = 
				decrypt_packet(pHeader->private_key, body, required - sizeof(uint32_t));
				parseOk = msg->ParseFromArray(plaint_body.c_str(), pb_size);
			}
			else
				parseOk = msg->ParseFromArray(body, required - sizeof(uint32_t));

			if (parseOk)
			{
				/*
				if (m_preTranslateFunc != nullptr)
					m_preTranslateFunc(message_id, msg);

				if (it->second.first != nullptr)
					it->second.first(packet, msg);
				//*/
				add_user_packet(packet, message_id, msg);
			}
			else
			{
			   //SNLOG(kFatal, "解析协议包失败,msg_id: %d\r\n", message_id);
			}
		}
		else
		{
		   //SNLOG(kFatal, "没有注册的消息:%d\r\n", message_id);
		}

		_VMProtectEnd();
	}

	const string& SQSafetUDP::get_public_key()
	{
		_VMProtectBegin(__FUNCTION__);
		static std::string ret;
		if(!ret.empty()) 
		{
			return ret;
		}

		char buffer64[64] = "0";
		_i64toa_s(SIGNATURE, buffer64, 64, 10);

		ret = buffer64;
		_VMProtectEnd();
		return ret;
	}

	string SQSafetUDP::decrypt_packet(uint64_t ulpkey, uint8* data, uint32 size)
	{	
		string cipher((char *)data, size);
		string private_key = fmt::Format("{0}", ulpkey);
		return safe::ISQSafeModel::std_algo()->aes_decryptx(cipher, get_public_key(), private_key);
	}

	bool SQSafetUDP::send_packet(const RakNet::AddressOrGUID systemIdentifier, 
		bool encrypt,
		uint32_t message_id,
		google::protobuf::Message *msg)
	{
		_VMProtectBegin(__FUNCTION__);
		int len = 0;
		char *buf = nullptr;
		uint32_t encrypt_size = 0;
		string output;
		uint32_t input_size = 0;

		if (!m_rak_interface)
			return false;

		if (encrypt && msg != nullptr)
		{
			// 加密处理
			input_size = msg->ByteSize();
			input_size += AES_BLOCK_LEN;

			encrypt_size = input_size;
			len = sizeof(MiniUdpPacket) + encrypt_size + sizeof(uint32_t);
		}
		else
			len = msg ? sizeof(MiniUdpPacket) + msg->ByteSize() + sizeof(uint32_t) : 
			sizeof(MiniUdpPacket) + sizeof(uint32_t);

		buf = new char[len];
		auto mini_header = reinterpret_cast<MiniUdpPacket*>(buf);
		mini_header->packetIdentifier = ID_USER_PACKET_ENUM + 1;
		auto header = &mini_header->sqpacket;
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
				string encode_body = 
					safe::ISQSafeModel::std_algo()->aes_encryptx(sbody, 
					get_public_key(),
					pkey);
				header->private_key = _strtoui64(pkey.c_str(), nullptr, 10);
				memcpy(&header->body, encode_body.c_str(), encode_body.size());
			}
			else
				msg->SerializeToArray(&header->body, msg->GetCachedSize());

			header->pb_length = msg->GetCachedSize();
		}

		// TODO: calc checksum
		uint32_t check_sum = 
			SQEncryptHelper::checksum((char *)&header->body, len - sizeof(MiniUdpPacket) - sizeof(uint32_t));
		*reinterpret_cast<uint32_t*>(((char*)header) + header->length - sizeof(uint32_t) - 1) = check_sum;

		bool rel = false;
		int sendorder = -1;

		if (m_config.m_is_server)
			sendorder = m_rak_interface->Send((const char*)buf, len, MEDIUM_PRIORITY, RELIABLE_ORDERED, 0, systemIdentifier, false);
		else
			sendorder = m_rak_interface->Send((const char*)buf, len, IMMEDIATE_PRIORITY, RELIABLE_ORDERED, 0, m_remote_rak, 0);

		if (sendorder > 0)
			rel = true;

		SAFE_DELETE(buf);

		_VMProtectEnd();
		return rel;
	}

	bool SQSafetUDP::listen_rand_port()
	{
		if (nullptr == m_rak_interface) 
			return false;

		bool startOK = false;
		uint32 port = 6000;
		while (!m_stop_sign)
		{
			if (!m_config.m_port)
				port = snqu::SQRandIntervalNum(0, 6000, fs_max_port_num);
			else
				port = m_config.m_port;

			RakNet::SocketDescriptor descriptor;
			descriptor.port = port;
			descriptor.socketFamily = AF_INET; // Test out IPV4

			if (m_rak_interface->Startup(m_config.m_max_connections, &descriptor, 1) == 
				RakNet::RAKNET_STARTED)
			{
				startOK = true;
				break;
			}

			Sleep(1);
		}

		if (startOK)
		{
			m_port = port;
		    //SNLOG(kTrace, "启动文件服务成功，PORT: %d\r\n", port);
		}
		else
		{
		    //SNLOG(kFatal, "启动文件服务失败！\r\n");
		}

		return startOK;
	}

	void SQSafetUDP::stop()
	{
		std::lock_guard<std::recursive_mutex> lock(m_lock);
        if (!m_started) return;

		m_stop_sign = true;
		if (m_started)
		{
			m_task_thread.end();
			if (m_run_thread.native_handle() != nullptr && 
				m_run_thread.get_id() != this_thread::get_id() &&
				m_run_thread.joinable())
			{
				m_run_thread.join();
			}

			if (m_run_thread.joinable())
				m_run_thread.detach();
		}
		
		while (SST_STATUS_STARTING == m_status)
		{
			Sleep(5);
		}

		if (m_rak_interface != nullptr)
		{
			m_rak_interface->Shutdown(0);
			RakNet::RakPeerInterface::DestroyInstance(m_rak_interface);
			m_rak_interface = nullptr;
		}
		
		m_client_status = CST_STATUS_CLOSED;
		SAFE_DELETE(m_fileTransfer);
	    //SNLOG(kTrace, "SQSafetUDP停止.\r\n");
		m_callback = nullptr;
		m_started = false;
		m_load_count = 0;
	}

	bool SQSafetUDP::register_message(uint32_t message_id, 
		const udp_callback_func_t& callback, 
		google::protobuf::Message* factory /*= nullptr*/)
	{
		if (m_message_map.find(message_id) != m_message_map.end())
			return false;

		m_message_map[message_id] = make_pair(callback, factory);
		return true;
	}

	// 注册预处理消息
	void SQSafetUDP::register_precallback(udpPreCallbackFunc_t callback)
	{
		m_preTranslateFunc = callback;
	}

	void SQSafetUDP::add_user_packet(RakNet::Packet* packet, uint32 message_id, proto_msg_ptr Message)
	{
		//SNLOG(kTrace, "添加数据包:%d", message_id);
		auto packet_ptr = std::make_shared<RakNet::Packet>();
		packet_ptr->systemAddress = packet->systemAddress;
		packet_ptr->guid = packet->guid;
		m_task_thread.post_task(std::make_tuple(packet_ptr, message_id, Message));
	}

	void SQSafetUDP::detect_packet_proc(const std::tuple<SQSTUdpPacketPtr, uint32_t, proto_msg_ptr>& task)
	{
		std::lock_guard<std::recursive_mutex> lock(m_msgmap_lock);
		auto message_id = std::get<1>(task);
		auto it = m_message_map.find(message_id);
		if (it != m_message_map.end())
		{
			//SNLOG(kTrace, "开始处理数据包:%d, time: %d", message_id, time(nullptr));
			if (m_preTranslateFunc != nullptr)
				m_preTranslateFunc(message_id, std::get<2>(task));

			if (it->second.first != nullptr)
				it->second.first(std::get<0>(task).get(), std::get<2>(task));
			//SNLOG(kTrace, "结束处理数据包:%d, time: %d", message_id, time(nullptr));
		}
	}

	// 发送文件或者大数据
	void SQSafetUDP::trans_data(SQUdpTransFileHelper& fileHelper, SQUdpAddress systemAddress)
	{
		if (m_rak_interface != nullptr && m_fileTransfer != nullptr)
		{
			if (m_config.m_is_server)
				m_fileTransfer->Send(&fileHelper, m_rak_interface, systemAddress, 0, HIGH_PRIORITY, 0, &m_incrementalReadInterface);
		}
	}

	// 添加文件传输回调
	void SQSafetUDP::add_file_callback(SQFileTransCallback *callback)
	{
		m_callback = callback;
	}

	bool SQSafetUDP::OnFile(OnFileStruct *onFileStruct)
	{
		if (m_callback != nullptr)
			m_callback->OnFile(onFileStruct);
		return true;
	}

	void SQSafetUDP::OnFileProgress(FileProgressStruct *fps)
	{
		if (m_callback != nullptr)
			m_callback->OnFileProgress(fps);
	}

	bool SQSafetUDP::OnDownloadComplete(DownloadCompleteStruct *dcs)
	{
		if (m_callback != nullptr)
			m_callback->OnDownloadComplete(dcs);
		return true;
	}

	int32 SQSafetUDP::client_connection_status()
	{
		// 客户端模式有效
		return m_client_status;
	}
}}