#ifndef _SQ_ZIP_H_
#define _SQ_ZIP_H_

#ifdef _DEBUG
#pragma comment(lib, "zlibd.lib")
#elif ZLIB_STATIC
//mt版本编译会报错，把SAFESEH关闭: 工程属性 -> Linker -> Advanced -> Image Has Safe Exception Handlers 改为 NO
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
	*@brief : 解压zip文件
	*@unzipPath : 解压到的目录位置。默认为当前运行程序目录
	*@zipFileName : zip文件的路径（包含zip文件名称）
	*@return : 解压成功返回true，反之返回false
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