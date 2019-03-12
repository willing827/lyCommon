#include <sqwin/win/sqpeinfo.h>

namespace snqu{

ULONG RvaToOffset(IMAGE_NT_HEADERS * pNtHeader, ULONG Rva);

PEInfoPtr GetPEInfo(const std::string& file_name)
{
	//DOS头
	PIMAGE_DOS_HEADER pImageDosHeader;
	//NT头(包括PE标识+Image_File_Header+OptionHeader)
	PIMAGE_NT_HEADERS pImageNtHeaders;

	HANDLE hFile = NULL;
	HANDLE hMapObject = NULL;
	//DOS头
	PUCHAR uFileMap = NULL;
	int ret = 0;
	PEInfoPtr ret_ptr = nullptr;

	do 
	{
		hFile = CreateFileA(file_name.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, 0);
		if (hFile == NULL)
		{
			ret = GetLastError();
			break;
		}

		hMapObject = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
		if (hMapObject == NULL)
		{
			ret = GetLastError();
			break;
		}

		//PE基址
		uFileMap = (PUCHAR)MapViewOfFile(hMapObject, FILE_MAP_READ, 0, 0, 0);
		if (uFileMap == NULL)
		{
			ret = GetLastError();
			break;
		}

		pImageDosHeader = (PIMAGE_DOS_HEADER)uFileMap;
		if (pImageDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		{
			ret = GetLastError();
			break;
		}

		//定位到NT PE头
		pImageNtHeaders = (PIMAGE_NT_HEADERS)((PUCHAR)uFileMap + pImageDosHeader->e_lfanew);
		ret_ptr = std::make_shared<IMAGE_NT_HEADERS>();
		*ret_ptr = *pImageNtHeaders;

		//导入表的相对虚拟地址(RVA)
		ULONG rva_ofimporttable = pImageNtHeaders->OptionalHeader.DataDirectory[1].VirtualAddress;
		//根据相对虚拟(rva)地址计算偏移地址(offset)
		ULONG offset_importtable = RvaToOffset(pImageNtHeaders, rva_ofimporttable);
		if (!offset_importtable)
		{
			ret = GetLastError();
			break;
		}

		break;

		//取得导入表的地址
		IMAGE_IMPORT_DESCRIPTOR *pImportTable = (IMAGE_IMPORT_DESCRIPTOR *)((char*)uFileMap + offset_importtable);

		IMAGE_IMPORT_DESCRIPTOR null_iid;
		IMAGE_THUNK_DATA null_thunk;
		memset(&null_iid, 0, sizeof(null_iid));
		memset(&null_thunk, 0, sizeof(null_thunk));

		//每个元素代表了一个引入的DLL。
		for (int i = 0; memcmp(pImportTable + i, &null_iid, sizeof(null_iid)) != 0; i++)
		{
			char *dllName = (char*)(uFileMap + RvaToOffset(pImageNtHeaders, pImportTable[i].Name));
			//拿到了DLL的名字
			//printf("模块[%d]: %s\n", i, (char*)dllName);
			PIMAGE_THUNK_DATA32 pThunk = (PIMAGE_THUNK_DATA32)(uFileMap + RvaToOffset(pImageNtHeaders, pImportTable[i].FirstThunk));

			while (pThunk->u1.Ordinal != NULL)
			{
				PIMAGE_IMPORT_BY_NAME pname = (PIMAGE_IMPORT_BY_NAME)(uFileMap + RvaToOffset(pImageNtHeaders, pThunk->u1.AddressOfData));
				//printf("函数编号: %d 名称: %s\n", pname->Hint, pname->Name);
				pThunk++;
			}
		}

	} while (0);

	if (uFileMap) UnmapViewOfFile(uFileMap);
	if (hMapObject) CloseHandle(hMapObject);
	if (hFile) CloseHandle(hFile);

	return ret_ptr;
}


//计算Offset
ULONG RvaToOffset(IMAGE_NT_HEADERS * pNtHeader, ULONG Rva)
{
	//PE节
	IMAGE_SECTION_HEADER *p_section_header;
	ULONG sNum, i;
	//取得节表项数目
	sNum = pNtHeader->FileHeader.NumberOfSections;
	//取得第一个节表项
	p_section_header = (IMAGE_SECTION_HEADER *)
		((BYTE *)pNtHeader + sizeof(IMAGE_NT_HEADERS));
	for (i = 0; i < sNum; i++)
	{
		if ((p_section_header->VirtualAddress <= Rva) && Rva < (p_section_header->VirtualAddress + p_section_header->SizeOfRawData))
		{
			return Rva - p_section_header->VirtualAddress + p_section_header->PointerToRawData;
		}
		p_section_header++;
	}
	return 0;
}

bool Isx64PE(const std::string& file)
{
	auto peinfo = GetPEInfo(file);

	if (!peinfo) return false;

	bool is_x64_pe = false;
	if (IMAGE_NT_OPTIONAL_HDR32_MAGIC == peinfo->OptionalHeader.Magic)
	{//32位程序
		is_x64_pe = false;
	}
	else if (IMAGE_NT_OPTIONAL_HDR64_MAGIC == peinfo->OptionalHeader.Magic)
	{//64位程序
		is_x64_pe = true;
	}

	return is_x64_pe;
}

}