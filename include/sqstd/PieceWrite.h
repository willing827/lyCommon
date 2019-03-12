#pragma once
#include <string>
#include <vector>
#include <atomic>
#include <memory>


struct PieceWrite
{
public:
    PieceWrite();
    ~PieceWrite();

    /*
        pieces应该是线程数量，太小的文件不要用此函数，直接写就是 暂时只支持小于2G的文件
        返回0表示成功
    */
    int BuildFile(const std::string& path, uint64_t file_size, size_t pieces);

    std::pair<uint64_t/*start_pos*/, uint64_t/*end_pos*/> GetPieceRange(size_t piece_idx);

    /*
        此处应保证，同时对同一块只有一个线程在操作
        返回0表示成功
    */
    int Write(size_t pieces_id, const char* data, size_t len);

    /*
        清理一个块重新下载
    */
    int ClearPiece(size_t pieces_id);

    /*
        所有线程停止后再调用此函数
        返回0表示成功
    */
    int Close();

private:
    struct impl;
    std::unique_ptr<impl> m_impl;
};
