#include <sqwin/sqwin.h>
#include <string>

namespace snqu{

typedef std::shared_ptr<IMAGE_NT_HEADERS> PEInfoPtr;
/*
	获取文件的PE信息
*/
PEInfoPtr GetPEInfo(const std::string& file_name);

/*
	取得文件导入表的信息
*/
//void GetImportTable();

//判断PE文件是否64位
bool Isx64PE(const std::string& file);

}