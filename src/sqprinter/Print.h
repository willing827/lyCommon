#pragma once
#include <sqwin/printer/printbase.h>
#include <afxwin.h>


namespace snqu{

    class Printer
    {
    public:
        Printer();
        ~Printer();

        bool create(const printer_info_t &printer_info);
        bool start_print(print_job_ptr_t print_job);
    private:
        Printer(const Printer&);  
        const Printer& operator=(const Printer&); 

        bool do_print();
        bool set_font(const job_unit_t &job_unit);
        virtual bool before_print();
        virtual bool after_print();
        virtual bool print_job_unit(job_unit_t &job_unit);

    protected:
        CDC printer_dc_;  
        print_job_ptr_t print_job_;
        CFont  user_font_;  // 用户字体
        CSize ch_size_;     // 字体大小
        CFont* def_font_;   // 默认字体
        int max_width_;     // 页面最大宽度
        int max_height_;    // 页面最大高度
        CPoint cur_print_pos_;  // 当前打印点所在的位置

        void init_wh();     // 初始化页面的宽高
    };

}