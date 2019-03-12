/*---------------------------------------------------------------------------*/
/*  sqjsonconf.h                                                             */
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
#ifndef __JSONCONF_H__
#define __JSONCONF_H__

#include "sqtypes.h"
#include "sqinc.h"
#include "sqthird/rapidjson/sqjson.h"

namespace snqu {

class SQJsonConf 
{
public:
	SQJsonConf();
	virtual ~SQJsonConf();

protected:
    bool save();    //继承后，设置值之后再保存
    bool open(string fileName, bool encrypt = false, bool is_utf8 = false);
    void get_value(string valueName, string& value, std::string default_val = "");
	void get_value(string valueName, int& value, int default_val = 0);
    void get_value(string valueName, unsigned short& value, int default_val = 0);
    void get_value(string valueName, __int64& value, __int64 default_val = 0);

	void set_value(string valueName, string value);
	void set_value(string valueName, int value);
    void set_value(string valueName, __int64 value);
    //有复杂json格式需求的时候使用
    JsVal& get_root() { return m_root; };

	bool save_utf8();
private:
	bool m_encrypt;
	string  m_fileName;
	std::list<std::pair<string, int32>> m_key_list;
    JsVal m_root;
};
}
#endif //__JSONCONF_H__