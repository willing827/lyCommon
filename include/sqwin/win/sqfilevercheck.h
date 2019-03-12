/*---------------------------------------------------------------------------*/
/*  sqfilevercheck.h                                                         */
/*                                                                           */
/*  History                                                                  */
/*      05/26/2017  create                                                   */
/*                                                                           */
/*  Author                                                                   */
/*       feng hao                                                            */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*---------------------------------------------------------------------------*/

#ifndef FILE_VERCHECK_H_
#define FILE_VERCHECK_H_

#include <string>
/*
    // ��������ն�����
    typedef enum tagRptTerminalType {
        RPT_Fee_Server = 0,	 // �Ʒѷ����
        RPT_Goods_Server,	 // ��Ʒ�����
        RPT_App_Server,	     // Ӧ�÷����
        RPT_Update_Server,	 // ���·����
        RPT_Fee_Cashier,	 // �Ʒ�������
        RPT_Goods_Cashier,	 // ��Ʒ������
        RPT_Fee_Client,	     // �Ʒѿͻ���
        RPT_Goods_Client,	 // ��Ʒ�ͻ���
        RPT_Fee_Server_GUI,	 // ����˽������
        RPT_Rpt_Server,	     // ���󱨸����
        // ...
        RPT_Type_Max,
    } RptTerminalType;
*/

namespace snqu{ namespace vercheck{

    enum VersionCheckRule
    {
        Check_Logic_Equal           = 0,
        Check_Logic_BeyondEqual,
        Check_Logic_BelowEqual,
        Check_Logic_Beyond,
        Check_Logic_Below,
        Check_Logic_Unequal         = 5
    };

    enum VersionCheckType
    {
        Check_File_Version,
        Check_File_MD5,
        Check_File_Signature,
    };

    struct CheckResult
    {
        int check_rst;              // 0��ʾͨ�� -1���ʧ�� ���� ������
        int deal_logic;             // �����߼�  ���˿��ܲ�һ�� ����Ŀɰ�Ͷ�Ų����Լ���� 0 ��ֹ���� 1 ��ʾ 2 ���� 3 ����
        std::string tip_msg;        // ��ʾ��Ϣ UTF8
        std::string err_file_path;  // ������ļ�·��

        CheckResult()
        {
            check_rst = 0;
            deal_logic = 0;
        }
    };

    /*
        @brief ͬ���ӿڣ����ȫ�ֲ������ļ��汾��md5������
        @param termial_typeΪ��ǰģ������ͣ����tagRptTerminalTypeע��
        @param param Ͷ�ŵļ�����
        @return CheckResult
    */
    CheckResult VerCheck(int termial_type, const std::string& check_param);
}
}

#endif// FILE_VERCHECK_H_