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
        // 用API函数DeleteDC销毁一个打印机设备句柄 
        ::DeleteDC(printer_dc_.Detach());
    }
}

bool Printer::create(const printer_info_t &printer_info)
{
    char *szOutput(NULL);
    //定义一个设备环境句柄
    HDC hdc_print = ::CreateDCA(printer_info.driver_name_.c_str(), 
        printer_info.printer_name_.c_str(), szOutput, NULL); 

    if (0 == hdc_print)
        return false;

    //把打印设备环境附加到DC对象
    if(FALSE == printer_dc_.Attach(hdc_print))
        return false;

    //保存现在的字体
    def_font_ = printer_dc_.GetCurrentFont();

    //初始化打印参数
    printer_dc_.SetMapMode(MM_TEXT);       //设置当前影射模式为：单位1像素
    printer_dc_.SetWindowOrg(0, 0);        //设置原点
    init_wh();
    return true;
}

//为DC创建字体
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
        printer_dc_.SelectObject(def_font_);  //恢复原来的字体
    }
    // 字体修改后获取当前字体的大小
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
    cur_print_pos_.Offset(0, ch_size_.cy); // 打印一行的位移
    return ret;
}

bool Printer::after_print()
{
    //通知打印机驱动程序页结束  
    return printer_dc_.EndPage() >= 0;                 
}

void Printer::init_wh()
{
    max_width_ = printer_dc_.GetDeviceCaps(HORZRES);    //返回设备的.以像素为单位的物理显示宽度 
    max_height_ = printer_dc_.GetDeviceCaps(VERTRES);   //返回设备的.以像素为单位的物理显示高度 
}

bool Printer::start_print(print_job_ptr_t print_job)
{
    print_job_ = print_job;

    //定义一个打印作业
    DOCINFO docInfo;
    memset(&docInfo,0,sizeof(DOCINFO));
    docInfo.cbSize = sizeof(DOCINFO);
    docInfo.lpszDocName = print_job->job_name_.c_str();
    docInfo.lpszOutput = NULL ;

    //通知打印机驱动程序执行一新的打印任务
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

    //通知打印机驱动程序打印完毕 
    printer_dc_.EndDoc();                  

    return true;
}

}