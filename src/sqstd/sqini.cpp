#include <sqstd/sqini.h>
#include <sqstd/sqinc.h>
#include <sqwin/win/sqtools.h>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;
namespace snqu {
SQIniConf::SQIniConf(void)
{
}

SQIniConf::~SQIniConf(void)
{
	Close();
}

void Trim(std::string& str, const std::string & ChrsToTrim = " \t\n\r", int TrimDir = 0)
{
	size_t startIndex = str.find_first_not_of(ChrsToTrim);
	if (startIndex == std::string::npos){str.erase(); return;}
	if (TrimDir < 2) str = str.substr(startIndex, str.size()-startIndex);
	if (TrimDir!=1) str = str.substr(0, str.find_last_not_of(ChrsToTrim) + 1);
}

vector<SQIniConf::STRU_RECORD> SQIniConf::GetRecord(const string& astrKeyName, const string& astrSectionName)
{
	vector<STRU_RECORD> data;
	vector<STRU_RECORD>::iterator iter = std::find_if(moContent.begin(), moContent.end(), 
													  SQIniConf::STRU_RECORDSECTIONKEYIS(astrSectionName, astrKeyName));

	if (iter == moContent.end()) return data;
	data.push_back (*iter);

	return data;
}


void SQIniConf::OpenInternal(istream &is)
{
	string lstrLineData = "";
	string lstrCurrentSection = ""; // Section Name

	moContent.clear();
	string lstrComments = ""; // A string to store comments in

	bool lbIniIsEof = false;

	while(!lbIniIsEof)
	{
		lbIniIsEof = std::getline(is, lstrLineData).eof();

		Trim(lstrLineData);
		if(!lstrLineData.empty())
		{
			STRU_RECORD r;

			if((lstrLineData[0]=='#')||(lstrLineData[0]==';'))
			{
				if ((lstrLineData.find('[')==string::npos)&&
					(lstrLineData.find('=')==string::npos))
				{
					lstrComments += lstrLineData + '\n';
				} 
				else 
				{
					r.mcCommented = lstrLineData[0];
					lstrLineData.erase(lstrLineData.begin());
					Trim(lstrLineData);
				}
			} 
			else
			{
				r.mcCommented = ' ';
			}

			if(lstrLineData.find('[')!=string::npos)
			{		
				lstrLineData.erase(lstrLineData.begin());
				lstrLineData.erase(lstrLineData.find(']'));
				r.mstrComments = lstrComments;
				lstrComments = "";
				r.mstrSection = lstrLineData;
				r.mstrKey = "";
				r.mstrValue = "";
				lstrCurrentSection = lstrLineData;
			}

			if(lstrLineData.find('=')!=string::npos)
			{
				r.mstrComments = lstrComments;
				lstrComments = "";
				r.mstrSection = lstrCurrentSection;	
				r.mstrKey = lstrLineData.substr(0,lstrLineData.find('='));
				r.mstrValue = lstrLineData.substr(lstrLineData.find('=')+1);	
			}
			if(lstrComments == "")
				moContent.push_back(r);
		}
	}
}


bool SQIniConf::Open(string astrFilename, bool bEncrypt /*= FALSE*/)
{
	bool rel = false;
	if (astrFilename.empty())
	{
		return rel;
	}

	if (!bEncrypt)
	{
		ifstream inFile (astrFilename.c_str());
		if (!inFile.is_open()) 
			return false;

		OpenInternal(inFile);
		inFile.close();
		rel = true;
	}
	else
	{
		BYTE byKey[] = { 
		  0x12, 0x3f, 0x14, 0x15, 0xec, 0x2e, 0x3c, 0x3d, 
		  0x2e, 0xc3, 0x04, 0x3f, 0xcc, 0x90, 0x86, 0xe3
		};

		/*
		ifstream  efin(astrFilename.c_str(), ios::binary);
		efin.exceptions(ifstream::eofbit | ifstream::failbit | ifstream::badbit);
		
		istream::pos_type current_pos = efin.tellg();
		efin.seekg(0,ios_base::end);

		istream::pos_type file_size = efin.tellg();
		efin.seekg(current_pos);
		//*/
		__try_code_start;
		DWORD nSize = GetPathFileSize((LPSTR)astrFilename.c_str());
		if (nSize > 0)
		{
			uint8* data = new uint8[nSize + 4];
			if (data != NULL)
			{
				if (ReadPathFileBuffer((LPSTR)astrFilename.c_str(), data, nSize))
				{
					string strdata = (char *)data;
					stringstream strs(strdata);
					OpenInternal(strs);
					rel = true;
				}
			}

			SAFE_DELETE_ARRAY(data);
		}
		__try_code_end(;);
		//efin.close();
	}

	return rel;
}

void SQIniConf::Close()
{
	moContent.clear();
}

bool SQIniConf::GetValue(const string& astrKey, const string& mstrSessionName, string& astrValue, const string& astrDefaultValue)
{
	vector<STRU_RECORD> content = GetRecord(astrKey, mstrSessionName);

	if(!content.empty())
	{
		astrValue = content[0].mstrValue;
		return true;
	}

	astrValue = astrDefaultValue;
	return false;
}

bool SQIniConf::GetValue(const string& astrKey, const string& mstrSessionName, 
					   BYTE& abyValue, const BYTE abyDefaultValue  )
{
	string lstrValue;
	if(GetValue(astrKey, mstrSessionName, lstrValue))
	{
		abyValue = atoi(lstrValue.c_str());
		return true;
	}
	else
	{
		abyValue = abyDefaultValue;
		return false;
	}
}
bool SQIniConf::GetValue(const string& astrKey, const string& mstrSessionName, WORD& awValue, const WORD awDefaultValue )
{
	string lstrValue;
	if(GetValue(astrKey, mstrSessionName, lstrValue))
	{
		awValue = atoi(lstrValue.c_str());
		return true;
	}
	else
	{
		awValue = awDefaultValue;
		return false;
	}
}
bool SQIniConf::GetValue(const string& astrKey, const string& mstrSessionName, INT& aiValue, const INT aiDefaultValue)
{
	string lstrValue;
	if(GetValue(astrKey, mstrSessionName, lstrValue))
	{
		aiValue = atoi(lstrValue.c_str());
		return true;
	}
	else
	{
		aiValue = aiDefaultValue;
		return false;
	}
}
bool SQIniConf::GetValue(const string& astrKey, const string& mstrSessionName, snqu::uint32& aiValue, const INT aiDefaultValue )
{
	string lstrValue;
	if(GetValue(astrKey, mstrSessionName, lstrValue))
	{
		aiValue = (snqu::uint32)atoi(lstrValue.c_str());
		return true;
	}
	else
	{
		aiValue = aiDefaultValue;
		return false;
	}
}
bool SQIniConf::GetValue(const string& astrKey, const string& mstrSessionName, 
					  LONG& alValue, const LONG alDefaultValue)
{
	string lstrValue;
	if(GetValue(astrKey, mstrSessionName, lstrValue))
	{
		alValue = atol(lstrValue.c_str());
		return true;
	}
	else
	{
		alValue = alDefaultValue;
		return false;
	}
}
bool SQIniConf::GetValue(const string& astrKey, const string& mstrSessionName, 
					  double& adbValue, const double adbDefaultValue)
{
	string lstrValue;
	if(GetValue(astrKey, mstrSessionName, lstrValue))
	{
		adbValue = atof(lstrValue.c_str());
		return true;
	}
	else
	{
		adbValue = adbDefaultValue;
		return false;
	}
}}