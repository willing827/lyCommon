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

    /*  ҵ��ȡ�ô�ӡ���б�
        ������ �Ƿ�ˢ��[is_refresh]
        ����ֵ��[printer[]] */
    int GetPrinterList(std::vector<printer_info_t>&, bool is_refresh=false);

    /*  ҵ�񣺴�ӡСƱ 
        ������ ��ӡ������������������ӡ��������[]*/
	int PrintReceipt(const print_job_ptr_t &);

    /*  ҵ�񣺴�ӡ��ǩ 
        ������ ��ӡ������������������ӡ��������[]*/
	int PrintLabel(const print_job_ptr_t &);

    /*  ҵ�񣺴�ӡ���� �����ο�
    ������ ��ӡ������������������ӡ��������[]*/
	int PrintGrid(const print_job_ptr_t &);

private:
    struct impl;
    std::unique_ptr<impl> impl_;

    PrintHelper(const PrintHelper&);  
    const PrintHelper& operator=(const PrintHelper&); 
};

}

