#include <sqwin/sqwin.h>
#include <sqstd/PieceWrite.h>
#include <sqwin/win/sqpath.h>


struct PieData
{
    HANDLE m_fp          = INVALID_HANDLE_VALUE;
    uint64_t m_start     = 0;
    uint64_t m_end       = 0;
    size_t piece_id      = 0; //分片编号
    uint64_t m_write_num = 0; //已经写入的数量
};

struct PieceWrite::impl
{
    std::vector<PieData> m_data;
    uint64_t m_file_size = 0;
    size_t m_pieces = 0;
    std::atomic_int m_inuse;

    impl() {}
    ~impl() {}

    int create_size_file(const char *filename, uint64_t file_size)
    {
        HANDLE file_handle;
        int err = -1;
//         HMODULE module;
//         HANDLE token = 0;
//         typedef int (WINAPI *sfvd)(HANDLE hFile, LONGLONG ValidDataLength);
//         sfvd SetFileValidData;
//         TOKEN_PRIVILEGES tp = { 0 };
        do 
        {

//             module = GetModuleHandleA("kernel32.dll");
//             if (module == NULL) {
//                 fprintf(stderr, "get module[kernel32.dll] error[%d]\n", GetLastError());
//                 break;
//             }
// 
//             SetFileValidData = (sfvd)GetProcAddress(module, ("SetFileValidData"));
//             if (!SetFileValidData) {
//                 fprintf(stderr, "GetProcAddress error[%d]\n", GetLastError());
//                 break;
//             }
// 
//             if (!OpenProcessToken(INVALID_HANDLE_VALUE, TOKEN_ALL_ACCESS, &token)) {
//                 fprintf(stderr, "OpenProcessToken error[%d]\n", GetLastError());
//                 break;
//             }
// 
//             if (!LookupPrivilegeValueA(0, "SeManageVolumePrivilege", &tp.Privileges->Luid)) {
//                 fprintf(stderr, "LookupPrivilegeValue error[%d]\n", GetLastError());
//                 break;
//             }
// 
//             // 提升权限
//             tp.PrivilegeCount = 1;
//             tp.Privileges->Attributes = SE_PRIVILEGE_ENABLED;
//             if (!AdjustTokenPrivileges(token, 0, &tp, sizeof(tp), 0, 0)) {
//                 fprintf(stderr, "AdjustTokenPrivileges error[%d]\n", GetLastError());
//                 break;
//             }

            file_handle = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (file_handle == INVALID_HANDLE_VALUE) {
                fprintf(stderr, "CreateFile error[%d]\n", GetLastError());
                break;
            }

            long size_lo = (long)file_size;
            long size_hi = (long)(file_size >> 32);

            if (SetFilePointer(file_handle, size_lo, &size_hi, FILE_BEGIN) == HFILE_ERROR) {
                fprintf(stderr, "SetFilePointer error[%d]\n", GetLastError());
                break;
            }

            if (SetEndOfFile(file_handle) == 0) {
                fprintf(stderr, "SetEndOfFile error[%d]\n", GetLastError());
                break;
            }

//             //不对磁盘进行清0操作
//             if (!SetFileValidData(file_handle, file_size)) {
//                 fprintf(stderr, "SetFileValidData error[%d]\n", GetLastError());
//                 break;
//             }
//             if (token) CloseHandle(token);

            if (file_handle) CloseHandle(file_handle);

            
            err = 0;

        } while (0);
       
        return err;
    }

};


PieceWrite::PieceWrite()
    : m_impl(new impl)
{

}

PieceWrite::~PieceWrite()
{
    m_impl.reset();
}

int PieceWrite::BuildFile(const std::string& path, uint64_t file_size, size_t pieces)
{
    if (!m_impl->m_data.empty())
        return -1;

    int err = 0;
    m_impl->m_file_size = file_size;
    m_impl->m_pieces = pieces;
    m_impl->m_inuse = 0;

    auto piece_size = m_impl->m_file_size / m_impl->m_pieces;

    //先创建目录
    auto up_path = snqu::path::get_parent_path(path);

    if (!snqu::path::exist_folder(up_path, true))
        return GetLastError();

    //先创建文件
    err = m_impl->create_size_file(path.c_str(), file_size);
    if (0 != err) return err;

    //对文件进行分片
    for (size_t i  = 0; i < pieces; i++)
    {
        PieData temp;
        temp.piece_id = i;
        temp.m_fp = ::CreateFileA(path.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
            FILE_FLAG_RANDOM_ACCESS, NULL);

        if (INVALID_HANDLE_VALUE == temp.m_fp)
        {
            err = GetLastError();
            break;
        }

        auto ret = GetFileSize(temp.m_fp, NULL);

        //计算每一个分片的长度，列如前200个字节就是0-199
        temp.m_start = i * piece_size;
        temp.m_end = (i + 1)  * piece_size - 1;
        if (i == pieces - 1)
            temp.m_end =  m_impl->m_file_size - 1;

        //指定文件开始写的位置
        long size_lo = (long)temp.m_start;
        long size_hi = (long)(temp.m_start >> 32);
        if (-1 == (::SetFilePointer(temp.m_fp, size_lo, &size_hi, FILE_BEGIN)))
        {
            err = GetLastError();
            break;
        }

        m_impl->m_data.emplace_back(temp);
    }
    
    if (err)
    {
        Close();
    }

    return err;
}

std::pair<uint64_t, uint64_t> PieceWrite::GetPieceRange(size_t piece_idx)
{
    std::pair<uint64_t, uint64_t> ret(0, 0);
    if (m_impl->m_data.size() <= piece_idx)
    {
        return ret;
    }

    ret.first = m_impl->m_data[piece_idx].m_start;
    ret.second = m_impl->m_data[piece_idx].m_end;
    return ret;
}

int PieceWrite::Write(size_t piece_id, const char* data, size_t len)
{
    int ret = -1;
    if (-1 == m_impl->m_inuse)
        return ret;

    m_impl->m_inuse++;
    do 
    {
        if (m_impl->m_data.size() <= piece_id)
        {
            break;
        }

        if (m_impl->m_data[piece_id].piece_id != piece_id)
        {
            ret = -2;
            break;
        }
            

        // 不能写入超过预分配的长度 
        auto piece_size = (m_impl->m_data[piece_id].m_end - m_impl->m_data[piece_id].m_start) + 1;
        if (m_impl->m_data[piece_id].m_write_num + len > piece_size)
        {
            ret = -4;
            break;
        }

        DWORD dwWrite = 0;
        if (!WriteFile(m_impl->m_data[piece_id].m_fp, data, len, &dwWrite, NULL) || dwWrite != len)
        {
            ret = GetLastError();
            break;
        }

        m_impl->m_data[piece_id].m_write_num += dwWrite;
    } while (0);

    
    m_impl->m_inuse--;
    return 0;
}

int PieceWrite::ClearPiece(size_t piece_id)
{
    if (-1 == m_impl->m_inuse)
        return -1;

    if (m_impl->m_data.size() <= piece_id)
    {
        return -2;
    }

    m_impl->m_data[piece_id].m_write_num = 0;

    long size_lo = (long)m_impl->m_data[piece_id].m_start;
    long size_hi = (long)(m_impl->m_data[piece_id].m_start >> 32);
    if (-1 == (::SetFilePointer(m_impl->m_data[piece_id].m_fp, size_lo, &size_hi, FILE_BEGIN)))
    {
        return GetLastError();
    }

    return 0;
}

int PieceWrite::Close()
{
    if (m_impl->m_inuse > 0)
    {
        return -1;
    }
    // 当计数为-1时不能再操作文件
    m_impl->m_inuse = -1;

    for (auto& item : m_impl->m_data)
    {
        ::FlushFileBuffers(item.m_fp);
        ::CloseHandle(item.m_fp);
        item.m_fp = INVALID_HANDLE_VALUE;
    }
    m_impl->m_data.clear();
    m_impl->m_inuse = 0;
    return 0;
}

