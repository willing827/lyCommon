/*---------------------------------------------------------------------------*/
/*  sqjsonconf.cpp                                                           */
/*                                                                           */
/*  History                                                                  */
/*      05/30/2015                                                           */
/*                                                                           */
/*  Author                                                                   */
/*      GUO LEI																 */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*---------------------------------------------------------------------------*/
#include <sqstd/sqtypes.h>
#include <sqstd/sqjsonconf.h>
#include <sqwin/win/sqtools.h>
#include <sqstd/sqfilerw.h>
#include <sqstd/sqinc.h>
#include <vmp/sqvmsdk.h>
#include <sqsafe/sqsafemodel.h>
#include <codec/sqcodec.h>

namespace snqu {

using namespace rapidjson;
SQJsonConf::SQJsonConf()
{
	m_fileName = "";
	m_encrypt = false;
}

SQJsonConf::~SQJsonConf()
{
    m_root.Clear();
}

bool SQJsonConf::open(string fileName, bool encrypt, bool is_utf8)
{
    _VMProtectBegin(__FUNCTION__);
    bool rel = false;
    BYTE byKey[] = {
        0x12, 0x3f, 0x14, 0x15, 0xec, 0x2e, 0x3c, 0x3d,
        0x2e, 0xc3, 0x04, 0x3f, 0xcc, 0x90, 0x86, 0xe3
    };

    if (fileName.empty())
    {
        return rel;
    }

    m_fileName = fileName;
    m_encrypt = encrypt;
    std::string key((char*)byKey, 16);

    std::string file_data;
    if (!FileRW::Read(fileName.c_str(), file_data))
    {
        return rel;
    }

    if (m_encrypt)
    {
        SNQUSimpleCryptData(byKey, sizeof(byKey), (LPBYTE)file_data.c_str(), file_data.length());
    }
	
    if (!m_root.Parse(file_data.c_str()))
    {
        return rel;
    }

    return true;
    _VMProtectEnd();
}

bool SQJsonConf::save()
{
    _VMProtectBegin(__FUNCTION__);
    int jason_size = 0;
    bool rel = true;
    BYTE byKey[] = {
        0x12, 0x3f, 0x14, 0x15, 0xec, 0x2e, 0x3c, 0x3d,
        0x2e, 0xc3, 0x04, 0x3f, 0xcc, 0x90, 0x86, 0xe3
    };
    std::string file_data = m_root.DebugDump();
    if (m_encrypt)
    {
        SNQUSimpleCryptData(byKey, sizeof(byKey), (LPBYTE)file_data.c_str(), file_data.length());
    }

    if (!FileRW::OverWrite(m_fileName, file_data.c_str(), file_data.length()))
    {
        rel = false;
    }

    _VMProtectEnd();
    return rel;
}

bool SQJsonConf::save_utf8()
{
	_VMProtectBegin(__FUNCTION__);
	int jason_size = 0;
	bool rel = true;
	BYTE byKey[] = {
		0x12, 0x3f, 0x14, 0x15, 0xec, 0x2e, 0x3c, 0x3d,
		0x2e, 0xc3, 0x04, 0x3f, 0xcc, 0x90, 0x86, 0xe3
	};
	std::string file_data = codec::A2U(m_root.DebugDump());
	if (m_encrypt)
	{
		SNQUSimpleCryptData(byKey, sizeof(byKey), (LPBYTE)file_data.c_str(), file_data.length());
	}

	if (!FileRW::OverWrite(m_fileName, file_data.c_str(), file_data.length()))
	{
		rel = false;
	}

	_VMProtectEnd();
	return rel;
}

void SQJsonConf::get_value(string valueName, int& value, int default_val)
{
    auto item = m_root[valueName.c_str()];
    if (item.IsNull())
        value = default_val;
    else
        value = item.GetInt();
}
    

void SQJsonConf::get_value(string valueName, unsigned short& value, int default_val)
{
    auto item = m_root[valueName.c_str()];
    if (item.IsNull())
        value = default_val;
    else
        value = (unsigned short)item.GetInt();;
}

void SQJsonConf::get_value(string valueName, __int64& value, __int64 default_val)
{
    auto item = m_root[valueName.c_str()];
    if (item.IsNull())
        value = default_val;
    else
        value = item.GetInt64();;
}

void SQJsonConf::get_value(string valueName, string& value, std::string default_val)
{
    auto item = m_root[valueName.c_str()];
    if (item.IsNull())
        value = default_val;
    else
        value = item.GetString();;
}


void SQJsonConf::set_value(string valueName, int value)
{
    m_root[valueName.c_str()] = value;
}

void SQJsonConf::set_value(string valueName, __int64 value)
{
    m_root[valueName.c_str()] = value;
}

void SQJsonConf::set_value(string valueName, string value)
{
    m_root[valueName.c_str()] = value;
}

}