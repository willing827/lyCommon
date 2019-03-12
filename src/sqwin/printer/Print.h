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
        CFont  user_font_;  // �û�����
        CSize ch_size_;     // �����С
        CFont* def_font_;   // Ĭ������
        int max_width_;     // ҳ�������
        int max_height_;    // ҳ�����߶�
        CPoint cur_print_pos_;  // ��ǰ��ӡ�����ڵ�λ��

        void init_wh();     // ��ʼ��ҳ��Ŀ��
    };

}