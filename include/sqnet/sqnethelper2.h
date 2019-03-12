#ifndef SQNETHELPER2_H
#define SQNETHELPER2_H

#include <sqstd/sqinc.h>
#include <HPSocket/HPSocket.h>

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

struct SQHeaderEx
{
    uint32_t signature;   // 消息包开头标志
    uint16_t message_id;  // 消息ID
    uint16_t length;      // 包长度
    uint16_t pb_length;   // pb内容长度
    uint64_t private_key; // 加密私钥
    uint64_t site_id;     // 网吧编号
    uint16_t main_ver;    // 主版本
    uint16_t sub_ver;     // 次版本
    uint32_t req_from;    // 发起方ID
    uint32_t req_dest;    // 目标ID
    char body[0];         // 数据部分
    // uint32_t checksum; // 数据签名
};
#pragma warning(pop)

#pragma pack(pop)

struct info_msg
{
	CONNID connID;
	LPCTSTR evt;
	int contentLength;
	LPCTSTR content;

	static info_msg* Construct(CONNID dwConnID, LPCTSTR lpszEvent, int iContentLength, LPCTSTR lpszContent);
	static void Destruct(info_msg* pMsg);

private:
	info_msg(CONNID dwConnID, LPCTSTR lpszEvent, int iContentLength, LPCTSTR lpszContent);
	~info_msg();
};

struct TPkgInfo
{
	bool is_header;
	int length;
	SQHeader* _header;

	TPkgInfo(bool header = true, int len = sizeof(SQHeader)) : 
		is_header(header), length(len) 
	{ 
		_header = new SQHeader();  
		::memset(_header, 0, sizeof(SQHeader));
	}

	void Reset() 
	{
		is_header = true;
		length = sizeof(SQHeader);

		if(nullptr == _header) {
			_header = new SQHeader();
		}

		::memset(_header, 0, sizeof(SQHeader)); 
	}

	~TPkgInfo() 
	{ 
		SAFE_DELETE(_header); 
	}
};


#endif //SQNETHELPER2_H