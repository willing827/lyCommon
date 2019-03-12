#pragma once
#include "CompressionFormat.h"
#include <vector>
namespace SevenZip
{
    class ProgressCallback
    {
    public:

        virtual bool OnFileCount(unsigned int /*fileCount*/){ return false; };

        virtual bool OnFileItems(std::vector<std::wstring>&/*itemNames*/, std::vector<unsigned __int64>&/*itemSizes*/){ return true; };
        /*
        ��ʼ: ѹ��/��ѹ
        totalSize : ��ѹ���ļ��ܴ�С  /   ѹ������С? or ����ѹ�ļ��ܴ�С?
        */
        virtual void OnStart(const std::wstring& /*filePath*/, unsigned __int64 /*totalSize*/) {}

        /*
        ��ֵ���� : ѹ��/��ѹ
        inSize   : �Ѵ����ļ��ܴ�С  / �Ѵ���ѹ������С
        outSize  : ����ѹ������С    / ��ѹ�����ļ��ܴ�С
        */
        virtual void OnRadio(unsigned __int64  /*inSize*/, unsigned __int64 /*outSize*/) {}

        /*
        �ļ����� : ѹ��/��ѹ
        filePath       : �ļ�
        bytesCompleted : �Ѵ����С
        */
        virtual void OnProgress(const std::wstring& /*filePath*/, unsigned __int64 /*bytesCompleted*/) {}
        
        /*
        ѹ��/��ѹ���
        ArchiveFilePath : ѹ����·��
        */
        virtual void OnEnd(const std::wstring& /*ArchiveFilePath*/) {}


        /*
        ��ѹ�ļ���ʼ
        destFolder : Ŀ���ļ���
        ItemPath   : ���·��
        return     : �Ƿ������ѹ
        */
        virtual bool OnFileBegin(const std::wstring& /*destFolder*/, std::wstring&  /*ItemPath*/) { return true; };

        /*
        ��ѹ�ļ�����
        filePath       : �ļ�·��
        bytesCompleted : �ļ��ߴ�
        return         : �Ƿ������ѹ
        */
        virtual bool OnFileDone(const std::wstring& /*filePath*/, unsigned __int64 /*bytesCompleted*/) { return true; }
        
        /*
        �ع��ļ�����
        filePath       : �ļ�·��
        return         : �Ƿ�����ع�
        */
        virtual void OnRollBack(const std::wstring& /*filePath*/){}

    };
}
