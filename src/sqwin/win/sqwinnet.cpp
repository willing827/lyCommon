#include <sqwin/win/sqwinnet.h>
#include <sqwin/sqwin.h>
#include <iphlpapi.h>
#include <regex>
#include <WS2tcpip.h>

#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib,"Ws2_32.lib")

namespace snqu{ namespace net{

class WinNetHelper
{
public:
    static WinNetHelper& Instance()
    {
        static WinNetHelper global;
        return global;
    }

    NetCardInfos& GetNetCardInfo(bool no_wifi) 
    {
        m_netcard_info.clear();
        PIP_ADAPTER_INFO pAdapterInfo;
        PIP_ADAPTER_INFO pAdapter = NULL;
        DWORD dwRetVal = 0;
        ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
        pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
        if (pAdapterInfo == NULL)
            return m_netcard_info;

        ZeroMemory(pAdapterInfo, ulOutBufLen);

        if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
        {
            free(pAdapterInfo);
            pAdapterInfo = (IP_ADAPTER_INFO *)malloc(ulOutBufLen);
            if (pAdapterInfo == NULL)
                return m_netcard_info;
        }

        if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == NO_ERROR)
        {
            for (PIP_ADAPTER_INFO pAdapter = pAdapterInfo; pAdapter != NULL; pAdapter = pAdapter->Next)
            {
				//71为：无线网卡
				if (pAdapter->Type == IF_TYPE_IEEE80211 && no_wifi)
					continue;

                // 确保是以太网
                if (pAdapter->Type != MIB_IF_TYPE_ETHERNET || pAdapter->Type == IF_TYPE_IEEE80211)
                    continue;

                if (!_strcmpi(pAdapter->IpAddressList.IpAddress.String, "0.0.0.0") ||
                    !_strcmpi(pAdapter->IpAddressList.IpMask.String, "0.0.0.0"))
                {
                    pAdapter->Type = 0xffff;
                    continue;
                }

                // 确保MAC地址的长度为 00-00-00-00-00-00
                if (pAdapter->AddressLength != 6)
                    continue;
                NetCardInfoT card_info;
                char acMAC[32];
                sprintf_s(acMAC, 32, "%02X-%02X-%02X-%02X-%02X-%02X",
                    int(pAdapter->Address[0]),
                    int(pAdapter->Address[1]),
                    int(pAdapter->Address[2]),
                    int(pAdapter->Address[3]),
                    int(pAdapter->Address[4]),
                    int(pAdapter->Address[5]));

                card_info.m_mac = acMAC;
                card_info.m_card_name = pAdapter->Description;
                card_info.m_addr = pAdapter->IpAddressList.IpAddress.String;
                card_info.m_mask = pAdapter->IpAddressList.IpMask.String;
                card_info.m_gate_way_addr = pAdapter->GatewayList.IpAddress.String;
                m_netcard_info.push_back(card_info);
            }
        }

        free(pAdapterInfo);
        return m_netcard_info;
    }


protected:
    WinNetHelper()  {};
    ~WinNetHelper() {};
    NetCardInfos m_netcard_info;
};

NetCardInfos& GetNetCardInfo(bool no_wifi)
{
    return WinNetHelper::Instance().GetNetCardInfo(no_wifi);
}

std::string GetRouterIP()
{
	std::string router_ip;
	PMIB_IPFORWARDTABLE pIpRouteTable = NULL;
	DWORD dwActualSize = 0;
	if (GetIpForwardTable(pIpRouteTable, &dwActualSize, TRUE) == ERROR_INSUFFICIENT_BUFFER)
	{
		pIpRouteTable = (PMIB_IPFORWARDTABLE)GlobalAlloc(GPTR, dwActualSize);
		if (GetIpForwardTable(pIpRouteTable, &dwActualSize, TRUE) != NO_ERROR)
		{
			GlobalFree(pIpRouteTable);
			return router_ip;
		}
	}

	if (pIpRouteTable != NULL)
	{
		struct in_addr inadGateway;
		if (pIpRouteTable->dwNumEntries > 0)
		{
			inadGateway.s_addr = pIpRouteTable->table[0].dwForwardNextHop;
			char szGatewayIp[16] = { 0 };
			inet_ntop(AF_INET, &inadGateway, szGatewayIp, 16);
			router_ip = szGatewayIp;
		}
		GlobalFree(pIpRouteTable);
	}
	return router_ip;
}

std::string GetMacByIp(const char * ip)
{
	if (ip) {
		IPAddr SrcIp = 0;
		ULONG mac_addr[2];
		ULONG PhysAddrLen = 6;
        struct in_addr s_ad; // IPv4地址结构体
        s_ad.s_addr = inet_addr(ip);
		SendARP(s_ad.s_addr, SrcIp, &mac_addr, &PhysAddrLen);
		return ToMacStr(mac_addr);
	}

	return "";
}

std::set<std::string> GetHostByName(const std::string& web_str)
{
    struct addrinfo hints;
    struct addrinfo *res, *cur;
    int ret;
    struct sockaddr_in *addr;
    char ipbuf[16];
    std::set<std::string> re_ips;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; /* Allow IPv4 */
    hints.ai_flags = AI_PASSIVE; /* For wildcard IP address */
    hints.ai_protocol = 0; /* Any protocol */
    hints.ai_socktype = SOCK_STREAM;

    ret = ::getaddrinfo(web_str.c_str(), NULL, &hints, &res);

    if (ret != 0) return re_ips;


    ;
    for (cur = res; cur != NULL; cur = cur->ai_next) {
        addr = (struct sockaddr_in *)cur->ai_addr;
        re_ips.insert(inet_ntop(AF_INET, &addr->sin_addr, ipbuf, 16));
    }
    freeaddrinfo(res);

    return re_ips;
}

bool IsValidIP(const char *ip)
{
	std::string re = "^(1\\d{2}|2[0-4]\\d|25[0-5]|[1-9]\\d|[1-9])\\."
		"(1\\d{2}|2[0-4]\\d|25[0-5]|[1-9]\\d|\\d)\\."
		"(1\\d{2}|2[0-4]\\d|25[0-5]|[1-9]\\d|\\d)\\."
		"(1\\d{2}|2[0-4]\\d|25[0-5]|[1-9]\\d|\\d)$";

	const std::regex rule(re);
	//三位数的都可以，没有设置1-255 的条件
	bool valid = std::regex_match(ip, rule);
	return valid;
}

}}