#include <sqwin/sqwin.h>
#include <zlib/unzip.h>
#include <zlib/zip.h>

#include <sqstd/sqsysfun.h>
#include <sqwin/win/sqwindows.h>
#include <sqstd/sqzip.h>
#include <sqstd/sqfilerw.h>
#include <assert.h>
#include <stdlib.h>
#include <sqwin/win/sqpath.h>


namespace snqu
{

	/**********************************************************
	*@brief : 解压zip文件
	*@unzipPath : 解压到的目录位置。默认为当前运行程序目录
	*@zipFileName : zip文件的路径（包含zip文件名称）
	*@return : 解压成功返回true，反之返回false
	***********************************************************/
	bool Zip::unzip(const char* zipFileName, const char* unzipPath)
	{
		if(nullptr == zipFileName) {
			assert(false && "好牛逼的使用");
			return false;
		}

		unz_global_info gi;
		unzFile hfile = unzOpen(zipFileName);
		if(unzGetGlobalInfo(hfile, &gi)) {
			unzClose(hfile);
			return false;
		}

		std::string curDir;
		if(nullptr == unzipPath) {
			std::string curDir = path::get_module_path();
			curDir = snqu::path::absolute_path(curDir);
		} else {
			curDir = unzipPath;
		}

		int result = 0;
		char filename[512] = {0};
		char ext[512] = {0};
		char com[2048] = {0};
		unz_file_info fileInfo;

		for(uLong i = 0; i < gi.number_entry; ++i) {

			memset(filename, 0, sizeof(filename));
			memset(ext, 0, sizeof(ext));
			memset(com, 0, sizeof(com));
			memset(&fileInfo, 0, sizeof(unz_file_info));

			if(unzGetCurrentFileInfo(hfile, &fileInfo, filename, 
				sizeof(filename), ext, sizeof(ext), 
				com, sizeof(com)) != UNZ_OK) {
					unzClose(hfile);
					return false;
			}

			if(!(fileInfo.external_fa & FILE_ATTRIBUTE_DIRECTORY)) {
				if(0 != unzOpenCurrentFile(hfile)) {
					unzClose(hfile);
					return false;
				}

				auto fileSize = sizeof(char) * fileInfo.uncompressed_size;
				auto fileData = (char*)malloc(fileSize);
				auto size = unzReadCurrentFile(hfile, fileData, fileSize);
				auto filePath = (curDir + "\\" + filename);
				bool isOK = snqu::save_file(filePath, 
					std::string(fileData, fileSize));
				free(fileData);
				fileData = nullptr;

				unzCloseCurrentFile(hfile);
				if(!isOK || (UNZ_OK != unzGoToNextFile(hfile) &&
					UNZ_END_OF_LIST_OF_FILE != unzGoToNextFile(hfile))) {
					unzClose(hfile);
					return false;
				}

			} else {
				snqu::path::create_folder((curDir + "\\" + filename).c_str());
				if(UNZ_OK != unzGoToNextFile(hfile) &&
					UNZ_END_OF_LIST_OF_FILE != unzGoToNextFile(hfile)) {
					unzClose(hfile);
					return false;
				}
			}
		}

		if(UNZ_OK != unzClose(hfile)) {
			return false;
		}

		return true;
	}

	bool Zip::unzip_for_each(const char* zipFileName, DownLoadCB callback, void *user_data)
	{
#define SQZIP_COMMENT_SIZE 2048
		if (nullptr == zipFileName) {
			assert(false && "unzip_for_each exception!");
			return false;
		}

		unz_global_info gi;
		unzFile hfile = unzOpen(zipFileName);
		if(unzGetGlobalInfo(hfile, &gi)) {
			unzClose(hfile);
			return false;
		}

		string curDir = snqu::path::get_module_path();
		curDir = snqu::path::absolute_path(curDir);
		
		int result = 0;
		char filename[512] = {0};
		char ext[512] = {0};
		char *com = nullptr;
		unz_file_info fileInfo;

		com = new char[SQZIP_COMMENT_SIZE];
		if (com) {
			for(uLong i = 0; i < gi.number_entry; ++i) {

				memset(filename, 0, sizeof(filename));
				memset(ext, 0, sizeof(ext));
				memset(com, 0, SQZIP_COMMENT_SIZE);
				memset(&fileInfo, 0, sizeof(unz_file_info));

				if (unzGetCurrentFileInfo(hfile, &fileInfo, filename, sizeof(filename), ext, sizeof(ext), com, SQZIP_COMMENT_SIZE) != UNZ_OK) {
					unzClose(hfile);
					return false;
				}

				if (!(fileInfo.external_fa & FILE_ATTRIBUTE_DIRECTORY)) {
					if (UNZ_OK != unzOpenCurrentFile(hfile)) {
						unzClose(hfile);
						return false;
					}

					auto fileSize = sizeof(char) * fileInfo.uncompressed_size;
					string fileData(fileSize, '\0');
					auto size = unzReadCurrentFile(hfile, const_cast<char*>(fileData.c_str()), fileSize);
					auto filePath = (curDir + "\\" + filename);
					
					if (callback != nullptr) {
						callback(filePath, fileData, user_data);
					}

					unzCloseCurrentFile(hfile);
					if((UNZ_OK != unzGoToNextFile(hfile) && UNZ_END_OF_LIST_OF_FILE != unzGoToNextFile(hfile))) {
						unzClose(hfile);
						return false;
					}

				} else {
					snqu::path::create_folder((curDir + "\\" + filename).c_str());
					if(UNZ_OK != unzGoToNextFile(hfile) && UNZ_END_OF_LIST_OF_FILE != unzGoToNextFile(hfile)) {
						unzClose(hfile);
						return false;
					}
				}
			}

			SAFE_DELETE_ARRAY(com);
		}
		
		if(UNZ_OK != unzClose(hfile)) {
			return false;
		}

		return true;
	}

    bool Zip::zip(const char* zipFileName, const char* zipPath)
    {
        FILE * fp_in = NULL;
        int len = 0;
        char buf[16384];
        auto err_n = fopen_s(&fp_in, zipFileName,"rb");
        if( NULL == fp_in || err_n != 0)
        {
            return false;
        }
        /////////////////////////////////////////////
        int status = APPEND_STATUS_CREATE;
        if (FileRW::Exist(zipPath)) 
            status = APPEND_STATUS_ADDINZIP;
        zipFile out = zipOpen(zipPath, status);

        if(out == NULL)
        {
            return false;
        }

        zip_fileinfo zi;
        unsigned long crcFile=0;
        zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
            zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
        zi.dosDate = 0;
        zi.internal_fa = 0;
        zi.external_fa = 0;

        std::string save_file_name = zipFileName;
        auto r_pos_1 = save_file_name.rfind("\\");
        auto r_pos_2 = save_file_name.rfind("/");
        if (r_pos_1 != std::string::npos || r_pos_2 != std::string::npos)
        {
            if (r_pos_1 == std::string::npos) r_pos_1 = 0;
            if (r_pos_2 == std::string::npos) r_pos_2 = 0;
            save_file_name = save_file_name.substr(max(r_pos_1, r_pos_2) + 1, save_file_name.length()-1);
        }

        if (ZIP_OK != zipOpenNewFileInZip(out, save_file_name.c_str(), &zi,
            NULL,0,NULL,0,NULL, Z_DEFLATED,Z_DEFLATED))
        {
            zipCloseFileInZip(out);
            fclose(fp_in);
            return false;
        }

        bool ret = false;
        for(;;)
        {
            len = fread(buf,1,sizeof(buf),fp_in);

            if(ferror(fp_in))
            {
                break;
            }

            if(len == 0) 
            {
                ret = true;
                break;
            }

            if(ZIP_OK != zipWriteInFileInZip(out, buf, len))
            {
                break;
            }
        }

        zipCloseFileInZip(out);
        zipClose(out, NULL);
        fclose(fp_in);
        return ret;
    }

    std::string Zip::zip_data(const char* data_ptr, const size_t data_len)
    {
        std::string ret_data;
        ret_data.reserve(data_len);
        z_stream c_stream;
        if (!data_ptr || !data_len) {
            return "";
        }
        c_stream.zalloc = NULL;
        c_stream.zfree = NULL;
        c_stream.opaque = NULL;
        c_stream.next_out = NULL;
        if (deflateInit2(&c_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
            MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
            return "";
        }
        c_stream.next_in = (Bytef *)data_ptr;
        c_stream.avail_in = data_len;
        auto tmp_buff = new Bytef[data_len];
        memset(tmp_buff, 0, data_len);
        c_stream.next_out = tmp_buff;
        c_stream.avail_out = data_len;
        while (c_stream.avail_in != 0 && c_stream.total_out < c_stream.avail_out) {
            if (deflate(&c_stream, Z_NO_FLUSH) != Z_OK) {
                goto end;
            }
        }
        if (c_stream.avail_in != 0) {
            return "";
        }
        int ret = -1;
        for (;;) {
            ret = deflate(&c_stream, Z_FINISH);
            if (ret == Z_STREAM_END) {
                break;
            }
            else if (ret != Z_OK) {
                break;
            }
        }
    end:
        if (deflateEnd(&c_stream) == Z_OK) {
            ret_data.assign((char*)tmp_buff, c_stream.total_out);
        }

        if (NULL != tmp_buff)
        {
            delete[] tmp_buff;
            tmp_buff = NULL;
        }

        return ret_data;
    }

    std::string Zip::unzip_data(const char* data_ptr, const size_t data_len)
    {
        if (!data_ptr || !data_len) {
            return "";
        }
        std::string ret_data;
        ret_data.reserve(data_len * 3);
        int ret = -1;
        z_stream d_stream = { 0 }; /* decompression stream */
        static char dummy_head[2] = {
            0x8 + 0x7 * 0x10,
            (((0x8 + 0x7 * 0x10) * 0x100 + 30) / 31 * 31) & 0xFF,
        };
        d_stream.zalloc = NULL;
        d_stream.zfree = NULL;
        d_stream.opaque = NULL;
        d_stream.next_in = (Bytef *)data_ptr;
        d_stream.avail_in = data_len;
        auto tmp_buff = new Bytef[128];
        memset(tmp_buff, 0, 128);
        d_stream.next_out = tmp_buff;
        if (inflateInit2(&d_stream, MAX_WBITS + 16) != Z_OK) {
            return "";
        }
        while (d_stream.total_in < data_len) {
            d_stream.avail_out = 128; /* force small buffers */
            ret = inflate(&d_stream, Z_SYNC_FLUSH);
            switch (ret) {
            case Z_OK:
                ret_data.append((char*)tmp_buff, d_stream.total_out);
                memset(tmp_buff, 0, 128);
                d_stream.total_out = 0;
                d_stream.next_out = tmp_buff;
                continue;
            case Z_STREAM_END:
                ret_data.append((char*)tmp_buff, d_stream.total_out);
                d_stream.total_out = 0;
                goto end;
            case Z_DATA_ERROR:
                d_stream.next_in = (Bytef *)dummy_head;
                d_stream.avail_in = sizeof(dummy_head);
                if ((ret = inflate(&d_stream, Z_NO_FLUSH)) != Z_OK) {
                    goto end;
                }
                break;
            default:
                goto end;
            }
        }
    end:
        bool ret_b = true;
        if (inflateEnd(&d_stream) != Z_OK) {
            ret_data = "";
        }

        if (NULL != tmp_buff)
        {
            delete[] tmp_buff;
            tmp_buff = NULL;
        }
        return ret_data;
    }
}
