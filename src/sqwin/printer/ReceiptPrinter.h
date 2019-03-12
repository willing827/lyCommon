#pragma once
#include "Print.h"

namespace snqu{

class ReceiptPrinter : public Printer
{
public:
    ReceiptPrinter();
    ~ReceiptPrinter();

private:
    struct impl;
    std::unique_ptr<impl> impl_;

    virtual bool before_print();
    virtual bool after_print();
    virtual bool print_job_unit(job_unit_t &job_unit);

    std::string print_str_; //Ҫ��ӡ���ַ�����ע���г��ȵ����ƣ� 
};


}