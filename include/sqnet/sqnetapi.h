#pragma once
#include <stdint.h>
#include <sqwin/sqwin.h>
#include <sqstd/sqstringhelper.h>

namespace snqu { namespace net{


    inline uint64_t hl64ton(uint64_t  host)
    {
        uint64_t   ret = 0;
        uint32_t   high, low;
        low = host & 0xFFFFFFFF;
        high = (host >> 32) & 0xFFFFFFFF;
        low = htonl(low);
        high = htonl(high);
        ret = low;
        ret <<= 32;
        ret |= high;
        return   ret;
    }

    inline uint64_t ntohl64(uint64_t  host)
    {
        uint64_t   ret = 0;
        uint32_t   high, low;
        low = host & 0xFFFFFFFF;
        high = (host >> 32) & 0xFFFFFFFF;
        low = ntohl(low);
        high = ntohl(high);
        ret = low;
        ret <<= 32;
        ret |= high;
        return   ret;
    }

    inline std::string BinToHex(const std::string &strBin, bool bIsUpper = false)
    {
        std::string strHex;
        strHex.resize(strBin.size() * 2);
        for (size_t i = 0; i < strBin.size(); i++)
        {
            uint8_t cTemp = strBin[i];
            for (size_t j = 0; j < 2; j++)
            {
                uint8_t cCur = (cTemp & 0x0f);
                if (cCur < 10)
                {
                    cCur += '0';
                }
                else
                {
                    cCur += ((bIsUpper ? 'A' : 'a') - 10);
                }
                strHex[2 * i + 1 - j] = cCur;
                cTemp >>= 4;
            }
        }

        return strHex;
    }

    inline std::string HexToBin(const std::string &strHex)
    {
        if (strHex.size() % 2 != 0)
        {
            return "";
        }

        std::string strBin;
        strBin.resize(strHex.size() / 2);
        for (size_t i = 0; i < strBin.size(); i++)
        {
            uint8_t cTemp = 0;
            for (size_t j = 0; j < 2; j++)
            {
                char cCur = strHex[2 * i + j];
                if (cCur >= '0' && cCur <= '9')
                {
                    cTemp = (cTemp << 4) + (cCur - '0');
                }
                else if (cCur >= 'a' && cCur <= 'f')
                {
                    cTemp = (cTemp << 4) + (cCur - 'a' + 10);
                }
                else if (cCur >= 'A' && cCur <= 'F')
                {
                    cTemp = (cTemp << 4) + (cCur - 'A' + 10);
                }
                else
                {
                    return "";
                }
            }
            strBin[i] = cTemp;
        }

        return strBin;
    }

    inline std::pair<std::string, unsigned short> GetIpPort(const std::string& input)
    {
        std::pair<std::string, unsigned short> ret;
        auto params = str::split(input, ":");

        if (params.size() < 2)
            return ret;

        auto tmp = snqu::str::ToUShort(params[1]);

        if (0 == tmp)
            return ret;

        ret.first = params[0];
        ret.second = tmp;

        return ret;
    }
}
}
