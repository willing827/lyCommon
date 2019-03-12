#ifndef _SQ_ZIP_H_
#define _SQ_ZIP_H_

#ifdef _DEBUG
#pragma comment(lib, "zlibd.lib")
#elif ZLIB_STATIC
//mt�汾����ᱨ����SAFESEH�ر�: �������� -> Linker -> Advanced -> Image Has Safe Exception Handlers ��Ϊ NO
#pragma comment(lib, "zlib_mt.lib")
#else
#define ZLIB_DLL
#pragma comment(lib, "zlib.lib")
#endif

namespace snqu
{

class Zip
{
public:
	/**********************************************************
	*@brief : ��ѹzip�ļ�
	*@unzipPath : ��ѹ����Ŀ¼λ�á�Ĭ��Ϊ��ǰ���г���Ŀ¼
	*@zipFileName : zip�ļ���·��������zip�ļ����ƣ�
	*@return : ��ѹ�ɹ�����true����֮����false
	***********************************************************/
    static bool unzip(const char* zipFileName, const char* unzipPath = nullptr);

	typedef std::function<void(const std::string& filepath, 
		const std::string& filedata, void* user_data)> DownLoadCB;
    static bool unzip_for_each(const char* zipFileName, DownLoadCB callback, void *user_data);

    static bool zip(const char* zipFileName, const char* zipPath);

    static std::string zip_data(const char* data_ptr, const size_t data_len);

    static std::string unzip_data(const char* data_ptr, const size_t data_len);
};

}


#endif //_SQ_ZIP_H_