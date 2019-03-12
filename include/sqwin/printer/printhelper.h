#pragma once
#include <set>
#include <memory>
#include "printbase.h"

namespace snqu{

class PrintHelper
{
public:
    PrintHelper();
    ~PrintHelper();

public:

    /*  业务：取得打印机列表
        参数： 是否刷新[is_refresh]
        返回值：[printer[]] */
    int GetPrinterList(std::vector<printer_info_t>&, bool is_refresh=false);

    /*  业务：打印小票 
        参数： 打印机名、打任务名、打印参数若干[]*/
	int PrintReceipt(const print_job_ptr_t &);

    /*  业务：打印标签 
        参数： 打印机名、打任务名、打印参数若干[]*/
	int PrintLabel(const print_job_ptr_t &);

    /*  业务：打印报表 仅供参考
    参数： 打印机名、打任务名、打印参数若干[]*/
	int PrintGrid(const print_job_ptr_t &);

private:
    struct impl;
    std::unique_ptr<impl> impl_;

    PrintHelper(const PrintHelper&);  
    const PrintHelper& operator=(const PrintHelper&); 
};

}

