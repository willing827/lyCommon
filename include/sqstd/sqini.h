
#ifndef __INIEX_H
#define __INIEX_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <functional>
#include <sqstd/sqinc.h>

using namespace std;
namespace snqu {
class SQIniConf
{
	struct STRU_RECORD
	{
		string	mstrComments;
		char	mcCommented;
		string	mstrSection;
		string	mstrKey;
		string	mstrValue;
	};

	enum ENUM_COMMENTCHAR
	{
		Pound = '#',
		SemiColon = ';'
	};

public:
	SQIniConf();
	virtual ~SQIniConf();

private:

	struct STRU_RECORDSECTIONKEYIS : std::unary_function<STRU_RECORD, bool>
	{
		std::string mstrSection;
		std::string mstrKey;

		STRU_RECORDSECTIONKEYIS(const std::string& section, const std::string& key): mstrSection(section),mstrKey(key){}

		bool operator()( const STRU_RECORD& rec ) const
		{
			return ((rec.mstrSection == mstrSection)&&(rec.mstrKey == mstrKey));
		}
	};

	vector<STRU_RECORD> moContent;
	vector<SQIniConf::STRU_RECORD> GetRecord(const string& astrKeyName, const string& astrSectionName);

public:
	void OpenInternal(istream &is);
	bool Open(string astrFilename, bool bEncrypt = FALSE);
	void Close();
	bool GetValue(const string& astrKey, const string& mstrSessionName, BYTE& abyValue, const BYTE abyDefaultValue = 0);
	bool GetValue(const string& astrKey, const string& mstrSessionName, WORD& awValue, const WORD awDefaultValue = 0);
	bool GetValue(const string& astrKey, const string& mstrSessionName, INT& aiValue, const INT aiDefaultValue = 0);
	bool GetValue(const string& astrKey, const string& mstrSessionName, snqu::uint32& aiValue, const INT aiDefaultValue = 0);
	bool GetValue(const string& astrKey, const string& mstrSessionName, LONG& alValue, const LONG alDefaultValue = 0);
	bool GetValue(const string& astrKey, const string& mstrSessionName, double& adbValue, const double adbDefaultValue = 0.0);
	bool GetValue(const string& astrKey, const string& mstrSessionName, string& astrValue, const string& astrDefaultValue = "");
};
}
#endif //__INIEX_H