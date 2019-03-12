#include <sqstd/sqstringhelper.h>
#include <sqwin/win/sqtools.h>
#include <sqstd/sqformat.h>
#include <sstream>
#include <iostream>
#include <locale>


namespace snqu{ namespace str{

std::vector<std::string> split(const std::string& s, const std::string& delim) 
{
    std::vector<std::string> ret;

    if (s.empty())
        return ret;

    size_t last = 0;  
    size_t index=s.find_first_of(delim,last);  
    while (index!=std::string::npos)  
    {  
        ret.push_back(s.substr(last,index-last));  
        last=index+1;  
        index=s.find_first_of(delim,last);  
    }  
    if (index-last>0)  
    {  
        ret.push_back(s.substr(last,index-last));  
    }  

    return std::move(ret);
}

std::set<std::string> split_to_set(const std::string& s, const std::string& delim)
{
    std::set<std::string> ret;

    if (s.empty())
        return ret;

    size_t last = 0;  
    size_t index=s.find_first_of(delim,last);  
    while (index!=std::string::npos)  
    {  
        ret.insert(s.substr(last,index-last));  
        last=index+1;  
        index=s.find_first_of(delim,last);  
    }  
    if (index-last>0)  
    {  
        ret.insert(s.substr(last,index-last));  
    }  

    return std::move(ret);
}

std::string rally(const std::vector<std::string>& s, const std::string& delim)
{
    std::string ret;

    for (auto& item : s)
    {
        ret.append(item).append(delim);
    }

    if (ret.length() > delim.length())
        ret.resize(ret.length() - delim.length());
    return std::move(ret);
}

std::string rally(const std::set<std::string>& s, const std::string& delim)
{
    std::string ret;

    for (auto& item : s)
    {
        ret.append(item).append(delim);
    }

    if (ret.length() > delim.length())
        ret.resize(ret.length() - delim.length());
    return std::move(ret);
}

int IsTextUTF8(char* str,ULONGLONG length)
{
	int i;
	DWORD nBytes=0;//UFT8����1-6���ֽڱ���,ASCII��һ���ֽ�
	UCHAR chr;
	BOOL bAllAscii=TRUE; //���ȫ������ASCII, ˵������UTF-8

	for(i=0; i < length; i++)
	{
		chr= *(str+i);
			if( (chr&0x80) != 0 ) // �ж��Ƿ�ASCII����,�������,˵���п�����UTF-8,ASCII��7λ����,����һ���ֽڴ�,���λ���Ϊ0,o0xxxxxxx
				bAllAscii= FALSE;
		if(nBytes==0) //�������ASCII��,Ӧ���Ƕ��ֽڷ�,�����ֽ���
		{
			if(chr>=0x80)
			{
				if(chr>=0xFC&&chr<=0xFD)
					nBytes=6;
				else if(chr>=0xF8)
					nBytes=5;
				else if(chr>=0xF0)
					nBytes=4;
				else if(chr>=0xE0)
					nBytes=3;
				else if(chr>=0xC0)
					nBytes=2;
				else
				{
					return FALSE;
				}
				nBytes--;
			}
		}
		else //���ֽڷ��ķ����ֽ�,ӦΪ 10xxxxxx
		{
			if( (chr&0xC0) != 0x80 )
			{
				return FALSE;
			}
			nBytes--;
		}
	}
	if( nBytes > 0 ) //Υ������
	{
		return FALSE;
	}
	if( bAllAscii ) //���ȫ������ASCII, ˵������UTF-8
	{
		return FALSE;
	}

	return TRUE;
} 

std::string cent_to_string(int value)
{
    int mod = abs(value % 100);
    int integ = abs(value / 100);

    if (0 == value || 0 == mod)
        return std::to_string(int(value/100));

	if (value > 0)
    {
        if (mod < 10)
            return fmt::Format("{0}.0{1}", integ, mod);
        else
        {
            if ((mod % 10) == 0)
                mod = mod / 10;
            return fmt::Format("{0}.{1}", integ, mod);
        }
    }
	else if (value < 0)
    {
        if ((mod) < 10)
            return fmt::Format("-{0}.0{1}", integ, mod);
        else
        {
            if ((mod % 10) == 0)
                mod = mod / 10;
            return fmt::Format("-{0}.{1}", abs(integ), abs(mod));
        }
    }

    return "0";
}

/* ����2λ��С��ת�� */
std::string to_string(double value, int decplaces)
{
    std::ostringstream out;
    out.precision(18);                   //����Ĭ�Ͼ���
    out << value;
    std::string str= out.str();          //������ȡ���ַ���
    size_t n = str.find('.');
    if ((n!=std::string::npos)           //��С������
        && (str.size()> n+decplaces))    //�������ٻ���decplacesλ��
    {
        str[n+decplaces]='\0';           //���ǵ�һ���������
    }
    str.swap(std::string(str.c_str()));  //ɾ��nul֮��Ķ����ַ�
    return str;
}

std::string to_string(time_t utc_time, std::string format)
{
    char str_time[100];  
    struct tm *local_time = NULL;  
    local_time = localtime(&utc_time);  
    strftime(str_time, sizeof(str_time), format.c_str(), local_time);  
    return str_time;
}

bool is_id(const std::string& param)
{
    int i = 0;
    std::locale loc("");
    for (auto& item : param)
    {
        if(item < '0' || item > '9')
            return false;
        i++;
        if(i > 15)
            break;
    }
    return true;
}

std::pair<bool, std::string> get_birthday_from_id(const std::string& paper_id)
{
    std::pair<bool, std::string> ret;

    auto format = [](std::string& birthday){
        birthday.insert(4, "-");
        birthday.insert(7, "-");
    };
    if (paper_id.length() == 15)
    {//7-12λ����������,����670401����1967��4��1��
        //15λ��Ϊ������ŮΪ˫��
        ret.second = paper_id.substr(6, 6);
        auto temp_char = str::get_right(paper_id, 2)[0];
        auto is_male = atoi(&temp_char);
        ret.first = (is_male % 2) != 0;
        ret.second.insert(0, "19");
        format(ret.second);
    }
    else if (paper_id.length() == 18)
    {//7-14λ���������գ�����19670401����1967��4��1��
        //17λ�������ڶ�λ����Ϊ������ŮΪ˫��
        ret.second = paper_id.substr(6, 8);
        auto temp_char = str::get_right(paper_id, 2)[0];
        auto is_male = atoi(&temp_char);
        ret.first = (is_male % 2) != 0;
        format(ret.second);
    }

    return ret;
}

bool check_birthday(const string& birth, time_t ntime)
{
    auto now_tm = str::to_string(ntime,"%m-%d");
    if (birth.length() < 10) return false;
    auto tmp_birth = birth.substr(5, 5);
    if (tmp_birth != now_tm) return false;
    return true;
}

char* split_strings(char *lpszSrc, char *lpszBTag, char *lpszETag, char *lpszValue, int nValueSize)
{
	char *lPtr = NULL;
	char *lPtr1 = NULL;
	char *lpszTag = NULL;
	int nSize = 0;

	if (!lpszBTag && lpszETag != NULL)
	{
		if ((lPtr = strstr(lpszSrc, lpszETag)) != NULL)
		{
			nSize = min((int)(lPtr - lpszSrc), (int)nValueSize);
			strncpy(lpszValue, lpszSrc, nSize);
		}
	}
	else if (lpszBTag != NULL && !lpszETag)
	{
		if ((lPtr = strstr(lpszSrc, lpszBTag)) != NULL)
		{
			lPtr += strlen(lpszBTag);
			nSize = min((int)strlen(lPtr), (int)nValueSize);
			strncpy(lpszValue, lPtr, nSize);
		}
	}
	else if (lpszBTag != NULL && lpszETag != NULL)
	{
		if ((lPtr = strstr(lpszSrc, lpszBTag)) != NULL)
		{
			lPtr += strlen(lpszBTag);
			if ((lPtr1 = strstr(lPtr, lpszETag)) != NULL)
			{
				nSize = min((int)(lPtr1 - lPtr), (int)nValueSize);
				strncpy(lpszValue, lPtr, nSize);
			}
		}
	}
	else if (!lpszBTag && !lpszETag)
	{
		nSize = min((int)(strlen(lpszSrc)), (int)nValueSize);
		strncpy(lpszValue, lpszSrc, nSize);
		lPtr = lpszSrc + strlen(lpszSrc);
	}

	return lPtr;
}


void split_strings2list(const std::string& src, const char* tag, std::vector<int>& alist)
{
	int32 len = 0;
	char temp_str[64] = {0};
	char *ptr = (char *)src.c_str();
	while (*ptr != '\0')
	{
		string keyword = "";
		zero_memory(temp_str, sizeof(temp_str));
		if (strstr(ptr, tag) != NULL)
		{
			ptr = split_strings(ptr, NULL, (char *)tag, temp_str, sizeof(temp_str));
			ptr++;
		}
		else
		{
			ptr = split_strings(ptr, NULL, NULL, temp_str, sizeof(temp_str));
		}

		alist.push_back(atoi(temp_str));
	}
}

void split_strings2list(const std::string& src, const char* tag, std::vector<std::string>& alist)
{
	int32 len = 0;
	char temp_str[256] = {0};
	char *ptr = (char *)src.c_str();
	while (*ptr != '\0')
	{
		string keyword = "";
		zero_memory(temp_str, sizeof(temp_str));
		if (strstr(ptr, tag) != NULL)
		{
			ptr = split_strings(ptr, NULL, (char *)tag, temp_str, sizeof(temp_str));
			ptr += strlen(tag);
		}
		else
		{
			ptr = split_strings(ptr, NULL, NULL, temp_str, sizeof(temp_str));
		}

		alist.push_back(temp_str);
	}
}

std::string get_right(const std::string& in_data, size_t len)
{
    if (in_data.size() > len)
        return in_data.substr(in_data.size() - len, len);
    return in_data;
}

bool VersionIsBelow(std::vector<int>& ver1, std::vector<int>& ver2)
{
	if (ver1.size() <= 0 || ver2.size() <= 0)
		return true;

	if (ver1.size() != ver2.size()) 
		return false;

	// 9.2.3.1883
	// 9.2.0.10386
	for (int i = 0; i < ver1.size(); i++)
	{
		if (ver1[i] > ver2[i])
			return false;		
		else if (ver2[i] == ver1[i])
			continue;
		else
			return true;
	}

    return false;
}

int version_compare(const std::string& ver1, const std::string& ver2)
{
    if (nequal(ver1, ver2))
        return 0;

    vector<int> lst_ver1, lst_ver2;
    split_strings2list(ver1, ".", lst_ver1);
    split_strings2list(ver2, ".", lst_ver2);

    if ((lst_ver1.size() != lst_ver2.size()) || lst_ver2.size() != 4)
    {
        if (ver1.empty())
            return -1;
        else if (ver2.empty())
            return 1;
        else
            return  ver1 > ver2 ? 1 : -1; 
    }

    if (VersionIsBelow(lst_ver1, lst_ver2))
        return -1;

    return 1;
}

}}