#include "Print.h"
#include <sqstd/sqstringhelper.h>

namespace snqu {

Printer::Printer()
    : def_font_(nullptr)
    , max_width_(0) , max_height_(0)
    , print_job_(nullptr)
{
    cur_print_pos_.SetPoint(0, 0);
}

Printer::~Printer()
{
    if (0 != printer_dc_.GetSafeHdc())
    {
        // ��API����DeleteDC����һ����ӡ���豸��� 
        ::DeleteDC(printer_dc_.Detach());
    }
}

bool Printer::create(const printer_info_t &printer_info)
{
    char *szOutput(NULL);
    //����һ���豸�������
    HDC hdc_print = ::CreateDCA(printer_info.driver_name_.c_str(), 
        printer_info.printer_name_.c_str(), szOutput, NULL); 

    if (0 == hdc_print)
        return false;

    //�Ѵ�ӡ�豸�������ӵ�DC����
    if(FALSE == printer_dc_.Attach(hdc_print))
        return false;

    //�������ڵ�����
    def_font_ = printer_dc_.GetCurrentFont();

    //��ʼ����ӡ����
    printer_dc_.SetMapMode(MM_TEXT);       //���õ�ǰӰ��ģʽΪ����λ1����
    printer_dc_.SetWindowOrg(0, 0);        //����ԭ��
    init_wh();
    return true;
}

//ΪDC��������
bool Printer::set_font(const job_unit_t &job_unit)
{
    user_font_.DeleteObject();    
    if (!job_unit.font_name_.empty())
    {
        VERIFY(user_font_.CreatePointFont(job_unit.font_size_*10, 
            job_unit.font_name_.c_str(), &printer_dc_));
        auto bef_font = printer_dc_.SelectObject(&user_font_);
        if (nullptr == bef_font)
            def_font_ = bef_font;
    }
    else
    {
        printer_dc_.SelectObject(def_font_);  //�ָ�ԭ��������
    }
    // �����޸ĺ��ȡ��ǰ����Ĵ�С
    ch_size_ = printer_dc_.GetTextExtent("00", 2);

    return true;
}

bool Printer::before_print()
{
    return 0 != printer_dc_.StartPage();
}

bool Printer::do_print()
{
    if (nullptr == print_job_)
        return false;

    for (UINT idx = 0; idx < print_job_->job_list_.size(); idx++)
    {
        if(!set_font(print_job_->job_list_[idx]))
        {
            return false;
        }

        if(!print_job_unit(print_job_->job_list_[idx]))
        {
            return false;
        }
    }

    return true;
}

bool Printer::print_job_unit(job_unit_t &job_unit)
{
    bool ret = (TRUE == printer_dc_.TextOut(0, cur_print_pos_.y, job_unit.print_data_.c_str()));
    cur_print_pos_.Offset(0, ch_size_.cy); // ��ӡһ�е�λ��
    return ret;
}

bool Printer::after_print()
{
    //֪ͨ��ӡ����������ҳ����  
    return printer_dc_.EndPage() >= 0;                 
}

void Printer::init_wh()
{
    max_width_ = printer_dc_.GetDeviceCaps(HORZRES);    //�����豸��.������Ϊ��λ��������ʾ��� 
    max_height_ = printer_dc_.GetDeviceCaps(VERTRES);   //�����豸��.������Ϊ��λ��������ʾ�߶� 
}

bool Printer::start_print(print_job_ptr_t print_job)
{
    print_job_ = print_job;

    //����һ����ӡ��ҵ
    DOCINFO docInfo;
    memset(&docInfo,0,sizeof(DOCINFO));
    docInfo.cbSize = sizeof(DOCINFO);
    docInfo.lpszDocName = print_job->job_name_.c_str();
    docInfo.lpszOutput = NULL ;

    //֪ͨ��ӡ����������ִ��һ�µĴ�ӡ����
    if(printer_dc_.StartDoc(&docInfo) < 0)      
    {
        return false;
    }

    if (!before_print())
    {
        return false;
    }

    if (!do_print())
    {
        return false;
    }

    if (!after_print())
    {
        return false;
    }

    //֪ͨ��ӡ�����������ӡ��� 
    printer_dc_.EndDoc();                  

    return true;
}

}