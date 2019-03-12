#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <set>

namespace snqu{ namespace net{

struct NetCardInfoT
{
	std::string m_card_name;
	std::string m_mac;
	std::string m_addr;
	std::string m_mask;
	std::string m_gate_way_addr;
	uint32_t	m_bandwidth = 0;
	uint32_t	m_traffics = 0;
};
typedef std::vector<NetCardInfoT> NetCardInfos;

// ��������
std::set<std::string> GetHostByName(const std::string&);

/***********��ȡ����������Ϣ***********
    param1: �ų���������
************************************/
NetCardInfos& GetNetCardInfo(bool no_wifi = true);

/***********����·�ɻ�ȡ���ص�IP��Ϣ***********/
std::string GetRouterIP();

/***********ͨ���Է�IP��ȡ�Է�MAC��Ϣ***********/
std::string GetMacByIp(const char* ip);
inline std::string ToMacStr(const void* mac)
{
    unsigned char * MacAddr = (unsigned char*)mac;
    char Mac[20] = { 0 };
    sprintf_s(Mac, "%02x-%02x-%02x-%02x-%02x-%02x",
        MacAddr[0], MacAddr[1], MacAddr[2], MacAddr[3], MacAddr[4], MacAddr[5]);
    return Mac;
}

bool IsValidIP(const char *ip);

inline bool IsValidIP(const std::string& ipaddress)
{
	return IsValidIP(ipaddress.c_str());
}

}}