#include <sqwin/printer/PrintHelper.h>
#include <string>
#include <algorithm>
#include <map>
#include "ReceiptPrinter.h"
#include "GridPrinter.h"
#include "LabelPrinter.h"
#include <WinSpool.h>


namespace snqu{

struct PrintHelper::impl
{
    std::vector<std::string> virtual_filter_;
    std::map<std::string, printer_info_t> printer_list_;

    impl()
    {
        //Physical printers has portname like LPT1,COM1,USB001
        virtual_filter_.push_back("LPT");
        virtual_filter_.push_back("COM");
        virtual_filter_.push_back("USB");
    }

    // 通过商品来判断是否为虚拟打印机
    bool is_virtual_printer(char *PortName)
    {
        auto f_iter = virtual_filter_.begin();
        for (; f_iter != virtual_filter_.end(); f_iter++)
        {
            _strupr_s(PortName, strlen(PortName)+1);
            if(strstr(PortName, (*f_iter).c_str()))
            {
                return false;
            }
        }
        
        return true;
    }

    bool read_printer_data_impl(DWORD printer_type)
    {
        DWORD Level = 2;        //information level   
        LPPRINTER_INFO_2A pPrinterInfo = NULL; //printer information buffer 
        DWORD cbBuf(0);         //size of printer information buffer 
        DWORD pcReturned;       //number of printers enumerated  
        char Name[500] = {0};   //name of printer object   
        DWORD pcbNeeded(0);     //bytes   received   or   required

        memset(Name, 0, sizeof(char)* 500)   ; 
        ::EnumPrintersA(printer_type, Name, Level, NULL, 0, &pcbNeeded, &pcReturned); 
        cbBuf = pcbNeeded + 4;
        pPrinterInfo = (LPPRINTER_INFO_2A)::LocalAlloc(LPTR, cbBuf);

        if (!pPrinterInfo) 
        {
            return false;
        }

        if (!::EnumPrintersA(printer_type, Name, Level, (LPBYTE)pPrinterInfo, cbBuf, &cbBuf, &pcReturned)) 
        {
            ::LocalFree(pPrinterInfo);
            return false;
        }

        printer_info_t tmp_printer;

        for (unsigned int idx = 0; idx < pcReturned; idx++)
        {
            if(is_virtual_printer((pPrinterInfo+idx)->pPortName))
                continue;

            tmp_printer.printer_name_ = (pPrinterInfo+idx)->pPrinterName;
            tmp_printer.port_name_ = (pPrinterInfo+idx)->pPortName;
            tmp_printer.driver_name_ = (pPrinterInfo+idx)->pDriverName;
//             if (PRINTER_ATTRIBUTE_WORK_OFFLINE & (pPrinterInfo+idx)->Attributes || 
//                 PRINTER_STATUS_OFFLINE & (pPrinterInfo+idx)->Attributes)
            {
                tmp_printer.is_off_line_ = false;
            }
            printer_list_.insert(std::make_pair(tmp_printer.printer_name_, tmp_printer));
        }

        ::LocalFree(pPrinterInfo);
        return true;
    }

    bool read_printer_data()
    {
        printer_list_.clear();
        //printer object types: remote printers 
        DWORD Flags = PRINTER_ENUM_FAVORITE | PRINTER_ENUM_REMOTE ;   
        if(!read_printer_data_impl(Flags))
        { "no remote printers"; }

        //printer object types: local printers 
        Flags =  PRINTER_ENUM_FAVORITE | PRINTER_ENUM_LOCAL ; 
        if(!read_printer_data_impl(Flags)) return false;
        return true;
    }

    int check_printer(print_job_ptr_t print_job)
    {
        auto printer = printer_list_.find(print_job->printer_name_);

        if (printer == printer_list_.end())
        {
            //ERR_SYS_NO_PRINTER
            return -1;
        }

        if (printer->second.is_off_line_)
        {
            read_printer_data();
            printer = printer_list_.find(print_job->printer_name_);
            if (printer->second.is_off_line_)
                //ERR_SYS_PRINTER_OFFLINE
                return -1;
        }

        return 0;
    }

    // print receipt by pos printer
    int print_receipt(print_job_ptr_t print_job)    
    {
        int chk_ret = check_printer(print_job);
        if (0 != chk_ret)
        {
            return chk_ret;
        }
        auto printer = printer_list_.find(print_job->printer_name_);
        ReceiptPrinter myprint;
        if(!myprint.create(printer->second))
        {
            return -1;
        }

        return myprint.start_print(print_job) ? 0 : -1;
    }

    // print label by labeller
    int print_label(print_job_ptr_t print_job)    
    {
        int chk_ret = check_printer(print_job);

        if (0 != chk_ret)
        {
            return chk_ret;
        }

        auto printer = printer_list_.find(print_job->printer_name_);

        LabelPrinter myprint;
        if(!myprint.create(printer->second))
        {
            return -1;
        }

        return myprint.start_print(print_job) ? 0 : -1;
    }

    // print report by A4 printer
    bool print_report(print_job_ptr_t print_job)    
    {
        printer_info_t tmp;
        tmp.port_name_ = "nul:";
        tmp.printer_name_ = "Send To OneNote 2013";
        tmp.driver_name_ = "Send To Microsoft OneNote 15 Driver";
        GridPrinter myprint;
        myprint.create(tmp);
        //myprint.set_direct();
        print_job->job_name_ = "打印输入测试";
        print_job->printer_name_ = "Send To OneNote 2013";
        job_unit_t test_ut;
        test_ut.table_.push_back(20);
        test_ut.table_.push_back(40);
        test_ut.table_.push_back(60);
        test_ut.table_.push_back(80);
        test_ut.font_size_ = 16;
        test_ut.left_blank_ = 200;
        test_ut.section_blank_ = 200;
        print_job->job_list_.push_back(test_ut);
        myprint.start_print(print_job);
        return false;
    }
};

PrintHelper::PrintHelper()
    :impl_(new impl)
{
}

PrintHelper::~PrintHelper()
{
    impl_.reset(nullptr);
}


int PrintHelper::GetPrinterList(std::vector<printer_info_t>& ret_data, bool is_refresh)
{
    if (is_refresh)
    {
        if (!impl_->read_printer_data())
            return -1;
    }

    if (impl_->printer_list_.size() > 0)
    {
        std::for_each(impl_->printer_list_.begin(), impl_->printer_list_.end(), [&ret_data](const std::pair<std::string, printer_info_t> &data)
        {
            ret_data.push_back(data.second);
        });
    }
    return 0;
}

int PrintHelper::PrintReceipt(const print_job_ptr_t &job_ptr)
{
    auto ret_ct = impl_->print_receipt(job_ptr);
    return ret_ct;
}

int PrintHelper::PrintLabel(const print_job_ptr_t & job_ptr)
{
    auto ret_ct = impl_->print_label(job_ptr);
    return ret_ct;
}

int PrintHelper::PrintGrid(const print_job_ptr_t & job_ptr)
{
    auto ret_ct = impl_->print_report(job_ptr);
    return ret_ct;
}

}

