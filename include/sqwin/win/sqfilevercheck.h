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
    // 报告服务终端类型
    typedef enum tagRptTerminalType {
        RPT_Fee_Server = 0,	 // 计费服务端
        RPT_Goods_Server,	 // 商品服务端
        RPT_App_Server,	     // 应用服务端
        RPT_Update_Server,	 // 更新服务端
        RPT_Fee_Cashier,	 // 计费收银端
        RPT_Goods_Cashier,	 // 商品收银端
        RPT_Fee_Client,	     // 计费客户端
        RPT_Goods_Client,	 // 商品客户端
        RPT_Fee_Server_GUI,	 // 服务端界面程序
        RPT_Rpt_Server,	     // 错误报告服务
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
        int check_rst;              // 0表示通过 -1检测失败 其它 错误码
        int deal_logic;             // 处理逻辑  各端可能不一样 具体的可按投放参数自己设计 0 禁止启动 1 提示 2 锁屏 3 重启
        std::string tip_msg;        // 提示信息 UTF8
        std::string err_file_path;  // 报错的文件路径

        CheckResult()
        {
            check_rst = 0;
            deal_logic = 0;
        }
    };

    /*
        @brief 同步接口，检测全局参数对文件版本及md5的限制
        @param termial_type为当前模块的类型，详见tagRptTerminalType注释
        @param param 投放的检测参数
        @return CheckResult
    */
    CheckResult VerCheck(int termial_type, const std::string& check_param);
}
}

#endif// FILE_VERCHECK_H_