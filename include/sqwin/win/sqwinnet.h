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

// 解析域名
std::set<std::string> GetHostByName(const std::string&);

/***********获取物理网卡信息***********
    param1: 排除无线网卡
************************************/
NetCardInfos& GetNetCardInfo(bool no_wifi = true);

/***********根据路由获取网关的IP信息***********/
std::string GetRouterIP();

/***********通过对方IP获取对方MAC信息***********/
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