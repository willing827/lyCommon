#include <sqwin/win/sqpath.h>
#include <sqwin/sqwin.h>
#include <corecrt_io.h>
#include <codec/sqcodec.h>
#include <sqwin/win/sqwindows.h>
#include <sqstd/sqformat.h>
#include <winternl.h>
#include <shlobj.h>
#include <sqwin/win/sqtools.h>
#include <Sddl.h>
#include <direct.h>
#include <sqstd/sqstringhelper.h>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "user32.lib")


namespace snqu{ namespace path{

std::string get_special_folder(int clsidl)
{
    char			path[MAX_PATH] = { 0 };
    std::string		folder("");
    LPITEMIDLIST	pidl = NULL;		// object to get special folder path	 

    CoInitialize(NULL);
    if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, clsidl, &pidl)))
    {
        if (SHGetPathFromIDListA(pidl, path))
        {
            folder = path;
        }
    }
    if (pidl)	CoTaskMemFree(pidl);
    ::CoUninitialize();

    return folder;
}

bool initialize_commonplace_security(SECURITY_ATTRIBUTES* psa)
{
    const char* szSD = "D:"         // Discretionary ACL
        "(A;OICI;GRGWGX;;;AU)" // Allow read/write/execute to authenticated users
        "(A;OICI;GRGWGX;;;BU)" // Allow read/write/execute to BU users
        "(A;OICI;GA;;;SY)"	 // Allow read/write/execute to BU users
        "(A;OICI;GA;;;BA)";    // Allow full control to administrators

    psa->bInheritHandle = FALSE;
    psa->nLength = sizeof(*psa);

    return ConvertStringSecurityDescriptorToSecurityDescriptorA(szSD, SDDL_REVISION_1, &psa->lpSecurityDescriptor, NULL) == TRUE;
}

HMODULE GetSelfModuleHandle()
{
    MEMORY_BASIC_INFORMATION mbi;
    return ((::VirtualQuery(GetSelfModuleHandle, &mbi, sizeof(mbi)) != 0)
        ? (HMODULE)mbi.AllocationBase : NULL);
}

std::string get_module_filename()
{
    char filename[MAX_PATH] = { 0 };

    auto hModule = GetSelfModuleHandle();

    DWORD length = ::GetModuleFileNameA(hModule, filename, sizeof(filename));
    if (length > 0 && length < sizeof(filename))
    {
        return absolute_path(filename);
    }
    else
    {
        return "";
    }
}

std::string get_parent_path(const std::string& full_path)
{
    std::string path = full_path;
    size_t pos = path.rfind("\\");
    if (pos != std::string::npos)
    {
        return path.substr(0, pos);
    }
    else
    {
        size_t	pos = path.rfind('/');
        if (pos != std::string::npos)
            return path.substr(0, pos);
    }
    return std::string();
}

void remove_last_path(std::string& path)
{
    size_t	pos = path.rfind('\\');
    if (pos != std::string::npos)
    {
        path.erase(pos);
    }
    else
    {
        size_t	pos = path.rfind('/');
        if (pos != std::string::npos)
            path.erase(pos);
    }
}

std::string get_module_path()
{
    return get_parent_path(get_module_filename());
}

std::string get_current_directory()
{
    char path[MAX_PATH] = { 0 };
    ::GetCurrentDirectoryA(sizeof(path), path);
    return path;
}

// 相对路径转换绝对路径
std::string absolute_path(const std::string& source)
{
    std::string destpath("");
    if (source.empty())
        return destpath;

    destpath.resize(source.size(), '\0');
    PathCombineA(const_cast<char *>(destpath.data()), source.c_str(), NULL);
    destpath.resize(strlen(destpath.c_str()));
    return destpath;
}

std::string temp_dir()
{
    char path[MAX_PATH] = { 0 };
    std::string retpath("");
    GetTempPathA(sizeof(path), path);
    retpath = path;
    return retpath;
}

std::string windows(void)
{
    return get_special_folder(CSIDL_WINDOWS);
}
std::string system(void)
{
    return get_special_folder(CSIDL_SYSTEM);
}

std::string app_data_local(void)
{
    return get_special_folder(CSIDL_LOCAL_APPDATA);
}
std::string program_files(void)
{
    return get_special_folder(CSIDL_PROGRAM_FILES);
}

std::string desktop(bool current_user)
{
    return get_special_folder(current_user ? CSIDL_DESKTOP : CSIDL_COMMON_DESKTOPDIRECTORY);
}
std::string start_menu(bool current_user)
{
    return get_special_folder(current_user ? CSIDL_STARTMENU : CSIDL_COMMON_STARTMENU);
}
std::string start_menu_program(bool current_user)
{
    return get_special_folder(current_user ? CSIDL_PROGRAMS : CSIDL_COMMON_PROGRAMS);
}
std::string start_menu_startup(bool current_user)
{
    return get_special_folder(current_user ? CSIDL_STARTUP : CSIDL_COMMON_STARTUP);
}
std::string start_menu_recently(void)
{
    return get_special_folder(CSIDL_RECENT);
}
std::string application_data(bool current_user)
{
    return get_special_folder(current_user ? CSIDL_APPDATA : CSIDL_COMMON_APPDATA);
}
std::string my_documents(bool current_user)
{
    return get_special_folder(current_user ? CSIDL_MYDOCUMENTS : CSIDL_COMMON_DOCUMENTS);
}

bool exist_folder(const std::string& destdir, bool create)
{
    int ftyp = _access(destdir.c_str(), 0);

    if (0 == ftyp)
    {
        return true;   // this is a directory!  
    }

    if (create)
    {
        return ERROR_SUCCESS == create_folder(destdir.c_str());
    }

    return false;    // this is not a directory!  
}

bool foreach_path(const std::string& cur_path, std::function<bool(const FileFDInfo&)> do_func, 
                   bool is_subdir, const std::string& extern_filter)
{
    std::string find_path;
    if (*cur_path.rbegin() == '\\' || *cur_path.rbegin() == '/')
        find_path = fmt::Format("{0}*", cur_path.c_str());
    else
        find_path = fmt::Format("{0}\\*", cur_path.c_str());

    if (!is_subdir && !extern_filter.empty())
    {
        find_path.append(extern_filter);
    }

    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;
    BOOL bContinue = TRUE;

    hFind = FindFirstFile(find_path.c_str(), &FindFileData);

    if (hFind == INVALID_HANDLE_VALUE)
        return false;

    bool ret = false;
    while (bContinue)
    {
        if (_stricmp(FindFileData.cFileName, "..") && _stricmp(FindFileData.cFileName, "."))
        {
            FileFDInfo file_fd_info;
            file_fd_info.file_path = cur_path;
            file_fd_info.file_name = FindFileData.cFileName;
            file_fd_info.is_dir = FILE_ATTRIBUTE_DIRECTORY == FindFileData.dwFileAttributes;
            SYSTEMTIME fSysTime;
            FileTimeToSystemTime(&FindFileData.ftCreationTime, &fSysTime);
            file_fd_info.create_time = os::SysTmToCTime(fSysTime);
            FileTimeToSystemTime(&FindFileData.ftLastWriteTime, &fSysTime);
            file_fd_info.last_edit_time = os::SysTmToCTime(fSysTime);
            file_fd_info.file_size = 0;
            if (FindFileData.nFileSizeHigh > 0)
                file_fd_info.file_size += (FindFileData.nFileSizeHigh * MAXDWORD + 1);
            file_fd_info.file_size += FindFileData.nFileSizeLow;

            // 当要遍历子目录时就不能在find的时候过滤,所以匹配扩展名
            bool is_cb = true;
            if (is_subdir && !extern_filter.empty())
                is_cb = get_externtion(file_fd_info.file_name) == extern_filter;

            if (is_cb && !do_func(file_fd_info))
            {// 用户打断返回成功
                bContinue = FALSE;
                break;
            }

            if (file_fd_info.is_dir && is_subdir)
            {
                std::string sub_dir = fmt::Format("{0}\\{1}\\", get_parent_path(find_path), FindFileData.cFileName);
                ret = foreach_path(sub_dir, do_func, is_subdir, extern_filter);
                if (!ret)
                    break;
            }
        }
        bContinue = FindNextFile(hFind, &FindFileData);
    }

    if (hFind != INVALID_HANDLE_VALUE)
    {
        FindClose(hFind);
        hFind = NULL;
    }

    if (bContinue == FALSE) ret = true;
    return ret;
}

std::string strip_path(const std::string& src)
{
    char szPath[MAX_PATH] = { 0 };
    strcpy_s(szPath, MAX_PATH, src.c_str());
    ::PathStripPathA(szPath);
    return szPath;
}

std::string get_externtion(const std::string& src)
{
    return ::PathFindExtension(src.c_str());
}


bool dospath_to_ntpath(const std::string& DosPath, std::string& NtPath)
{
    typedef BOOLEAN(__stdcall *fnRtlDosPathNameToNtPathName_U)(PCWSTR DosFileName, PUNICODE_STRING NtFileName, PWSTR *FilePart, PVOID Reserved);
    static fnRtlDosPathNameToNtPathName_U RtlDosPathNameToNtPathName = 
        (fnRtlDosPathNameToNtPathName_U)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlDosPathNameToNtPathName_U");
    //定义变量
    bool ret = false;

    //参数效验
    if (RtlDosPathNameToNtPathName == NULL) return false;

    std::wstring temp_dos = codec::S2W(DosPath);
    _UNICODE_STRING ret_data;
    if (RtlDosPathNameToNtPathName(temp_dos.c_str(), &ret_data, NULL, NULL))
    {
        ret = true;
    }
    NtPath = codec::W2S(ret_data.Buffer);
    return ret;
}

char* normalize_path_for_fileoperation(const char* path_file)	// call delete[] to free return buffer
{
    // multiple backblash will lead SHFileOperationW to failed with code 0xb7
    std::string	file(path_file);
    char			*buf = NULL;
    size_t			pos = file.find("\\\\", 1);	// fine from 1 to avoid net-neighbour path

    while (pos != std::string::npos)
    {
        file.erase(pos, 1);
        pos = file.find("\\\\", 1);
    }
    buf = new char[file.length() + 4];
    strcpy_s(buf, file.length() + 2, file.c_str());
    buf[file.length()] = buf[file.length() + 1] = 0;

    return buf;
}

int rename_file(const char* from, const char* to)
{
    SHFILEOPSTRUCT	fo = { 0 };
    int				ret = 0;
    char			*buf = normalize_path_for_fileoperation(from), *to_buf = normalize_path_for_fileoperation(to);

    fo.fFlags = FOF_NO_UI;
    fo.wFunc = FO_MOVE;
    fo.pFrom = buf;
    fo.pTo = to_buf;
    ret = SHFileOperationA(&fo);
    if (ret == ERROR_NOT_SAME_DEVICE)
    {
        fo.wFunc = FO_COPY;
        ret = SHFileOperationA(&fo);
        if (ret == ERROR_SUCCESS)
        {
            ret = delete_file(from);
        }
    }
    delete[] buf;
    delete[] to_buf;

    return ret;
}

int move_file(const char* from, const char* to)
{
    SHFILEOPSTRUCT	fo = { 0 };
    int				ret = 0;
    char			*buf = normalize_path_for_fileoperation(from), *to_buf = normalize_path_for_fileoperation(to);

    fo.fFlags = FOF_NO_UI;
    fo.wFunc = FO_MOVE;
    fo.pFrom = buf;
    fo.pTo = to_buf;
    ret = SHFileOperationA(&fo);
    delete[] buf;
    delete[] to_buf;

    return ret;
}

int copy_file(const char* from, const char* to)
{
    SHFILEOPSTRUCT	fo = { 0 };
    int				ret = 0;
    char			*buf = normalize_path_for_fileoperation(from), *to_buf = normalize_path_for_fileoperation(to);

    fo.fFlags = FOF_NO_UI;
    fo.wFunc = FO_COPY;
    fo.pFrom = buf;
    fo.pTo = to_buf;
    ret = SHFileOperationA(&fo);
    delete[] buf;
    delete[] to_buf;

    return ret;
}
int delete_file(const char* file)
{
    SHFILEOPSTRUCT	fo = { 0 };
    int				len = strlen(file);
    char			*buf = normalize_path_for_fileoperation(file);

    fo.fFlags = FOF_NO_UI;
    fo.wFunc = FO_DELETE;
    fo.pFrom = buf;
    len = SHFileOperationA(&fo);
    delete[] buf;

    return len;
}
int create_folder(const char* path_folder, SECURITY_ATTRIBUTES* psa)
{
    SECURITY_ATTRIBUTES	sa = { 0 };
    char				*norm = normalize_path_for_fileoperation(path_folder);
    std::string		folder(norm);

    delete[] norm;
    if (psa == NULL)
    {
        psa = &sa;
        initialize_commonplace_security(psa);
    }
	//ERROR_ALREADY_EXISTS
    return SHCreateDirectoryExA(NULL, folder.c_str(), psa);
}

// 删除目录
bool delete_folder(const std::string& szdir)
{
    char filepath[512] = {0};
    foreach_path(szdir, [&filepath](const FileFDInfo& fd_info)->bool
    {
        sprintf_s(filepath, "%s\\%s", fd_info.file_path.c_str(), fd_info.file_name.c_str());
        if (fd_info.is_dir)
        {
            if (!delete_folder(filepath))
            {
                return false;
            }
        }
        else
        {//不是文件就是目录
            if (remove(filepath) != 0)
            {
                return false;
            }
        }
        return true;

    }, false);
    Sleep(100);
    int ret = _rmdir(szdir.c_str());
    if (ret != 0)
    {
        auto err = GetLastError();
    }
    return ret == 0;
}

bool read_dir(std::list<std::string>& ret_list, const std::string& path, bool is_dir)
{
    char filepath[512] = { 0 };
    bool ret = foreach_path(path, [&](const FileFDInfo& fd_info)->bool
    {
        sprintf_s(filepath, "%s\\%s", fd_info.file_path.c_str(), fd_info.file_name.c_str());
        if (fd_info.is_dir)
        {
            if (is_dir)
                ret_list.push_back(filepath);
        }
        else
        {
            if (!is_dir)
                ret_list.push_back(filepath);
        }
        return true;

    }, false);

    return ret;
}

std::string to_linux_path(const std::string& input)
{
	std::string ret = input;
	str::string_replace(ret, std::string("\\\\"), std::string("/"));
	str::string_replace(ret, std::string("\\"), std::string("/"));
	return ret;
}

std::string to_windows_path(const std::string& input)
{
	std::string ret = input;
	str::string_replace(ret, std::string("/"), std::string("\\"));
	return ret;
}

bool get_path_by_link(const std::string& link_name, std::string& ret_path, std::string* ret_args)
{
	bool ret = false;
	HRESULT   hres;
	IShellLink*   psl;
	char   szGotPath[MAX_PATH];
	char   szArguement[MAX_PATH];
	WIN32_FIND_DATA   wfd;
	std::wstring wslink = codec::S2W(link_name);
	CoInitialize(0);

	hres = CoCreateInstance(CLSID_ShellLink, NULL,
		CLSCTX_INPROC_SERVER, IID_IShellLinkA, (LPVOID*)&psl);
	if (SUCCEEDED(hres)) {
		IPersistFile*   ppf;
		hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);
		if (SUCCEEDED(hres)) 
		{
			hres = ppf->Load(wslink.c_str(), STGM_READ);
			if (SUCCEEDED(hres)) 
			{
				hres = psl->Resolve(0, 0);
				if (SUCCEEDED(hres)) 
				{
					hres = psl->GetPath(szGotPath, MAX_PATH, (WIN32_FIND_DATA*)&wfd, SLGP_RAWPATH);

					if (SUCCEEDED(hres))
					{
						ret_path = szGotPath;
						ret = true;
					}
					hres = psl->GetArguments(szArguement, MAX_PATH);
					if (SUCCEEDED(hres) && ret_args)
						*ret_args = szArguement;
				}
			}
			ppf->Release();
		}
		psl->Release();
	}
	CoUninitialize();
	return ret;
}

}}