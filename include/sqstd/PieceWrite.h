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
        piecesӦ�����߳�������̫С���ļ���Ҫ�ô˺�����ֱ��д���� ��ʱֻ֧��С��2G���ļ�
        ����0��ʾ�ɹ�
    */
    int BuildFile(const std::string& path, uint64_t file_size, size_t pieces);

    std::pair<uint64_t/*start_pos*/, uint64_t/*end_pos*/> GetPieceRange(size_t piece_idx);

    /*
        �˴�Ӧ��֤��ͬʱ��ͬһ��ֻ��һ���߳��ڲ���
        ����0��ʾ�ɹ�
    */
    int Write(size_t pieces_id, const char* data, size_t len);

    /*
        ����һ������������
    */
    int ClearPiece(size_t pieces_id);

    /*
        �����߳�ֹͣ���ٵ��ô˺���
        ����0��ʾ�ɹ�
    */
    int Close();

private:
    struct impl;
    std::unique_ptr<impl> m_impl;
};
