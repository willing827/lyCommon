#include "GridPrinter.h"
#include <WinSpool.h>
#include <afxwin.h>
#include <sqwin/printer/PrintHelper.h>
#include <sqstd/sqstringhelper.h>

namespace snqu{

struct GridPrinter::impl
{
    CDC *printer_dc_;
    int max_width_; // 页面宽度
    bool is_tb_start_;

    impl()
        : max_width_(0)
        , printer_dc_(NULL)
        , is_tb_start_(true)
    {
    }

    bool print_table_text(CPoint &point, std::vector<int> &tab, const std::string &line_data)
    {
        int max_h_move = 0;
        int move_x = 0;
        CRect rectPrint;
        auto str_list = str::split(line_data, "\t");

        if (line_data.size() != tab.size())
            return false;

        for (unsigned int r = 0; r < tab.size(); r++)
        {
            rectPrint.SetRect(point.x + move_x + 20, point.y, tab[r], point.y+tab[r]);
            printer_dc_->DPtoLP(&rectPrint);
            
            UINT text_algin = DT_WORDBREAK;
            text_algin |= DT_CENTER;

            auto move = printer_dc_->DrawText(str_list[r].c_str(), 
                str_list[r].length(), rectPrint, text_algin); // 打印到缓冲区

            if (0 == move)
                return false;

            move_x = tab[r] - point.x; // 减个边距
            if (move < max_h_move)
                max_h_move = move; // 纵向最大偏移
        }
        rectPrint.OffsetRect(0, max_h_move);
        printer_dc_->LPtoDP(&rectPrint);
        point.y = rectPrint.top;
        return true;
    }

    void print_line(CPoint &start_point, CPoint &end_point)
    {
        printer_dc_->DPtoLP(&start_point);
        printer_dc_->DPtoLP(&end_point);
        printer_dc_->MoveTo(start_point);
        printer_dc_->LineTo(end_point);
        printer_dc_->LPtoDP(&start_point);
        printer_dc_->LPtoDP(&end_point);
    }

    bool print_table_grid(CPoint start_point, CPoint end_point, std::vector<int> &tab)
    {
        if (is_tb_start_)
        {// 画表格最上面的线
            CPoint fst_end;
            fst_end.SetPoint(tab[tab.size()-1], start_point.y);
            print_line(start_point, fst_end);
            is_tb_start_ = false;
        }

        int bot_x = end_point.x;
        // 画间隔的线
        for (unsigned int row = 0; row < tab.size(); row++)
        {
            print_line(start_point, end_point);
            start_point.SetPoint(tab[row], start_point.y);
            end_point.SetPoint(tab[row], end_point.y);
        }
        // 最右边底线
        print_line(start_point, end_point);
        // 画文字下面的线
        start_point.SetPoint(bot_x, end_point.y);
        print_line(start_point, end_point);

        return true;
    }

    bool print_table_line(CPoint &point, std::vector<int> &tab, const std::string &line_data)
    {
        unsigned int row = tab.size();

        CPoint point_start = point;
        point.Offset(0, 20); // 内容和表格间距

        if(!print_table_text(point, tab, line_data))
        {
            return false;
        }

        point.Offset(0, 20); // 内容和表格间距
        if(!print_table_grid(point_start, point, tab))
        {
            return false;
        }

        return true;
    }

    bool print_text(CPoint &ponit, job_unit_t &job_unit, const std::string &text)
    {
        CRect rectPrint(0, ponit.y, max_width_, ponit.y + max_width_); 
        printer_dc_->DPtoLP(&rectPrint);
        //X方向向右增加，Y方向向上增加
        printer_dc_->SetTextAlign(TA_TOP|TA_LEFT);  //设置对齐

        UINT text_algin = DT_WORDBREAK|job_unit.align_;
        auto move = DrawTextA(printer_dc_->GetSafeHdc(), text.c_str(), text.length(), rectPrint, text_algin);//打印到缓冲区

        if (0 == move)
            return false;

        rectPrint.top += move;
        printer_dc_->LPtoDP(&rectPrint);
        ponit.y = rectPrint.top;
        return true;
    }

    void calc_table_width(job_unit_t &job_unit, CPoint &cur_pos)
    {
        if (job_unit.table_.size() > 0)
        {
            for (unsigned int row = 0; row < job_unit.table_.size(); row++)
            {// 按百分比分配宽度
                job_unit.table_[row] = job_unit.table_[row] * (max_width_ - job_unit.left_blank_*2)/100 + job_unit.left_blank_;
            }

            job_unit.table_.push_back(max_width_-job_unit.left_blank_); // 补充最大宽度
        }// 普通表格打印左右都留空白才好看

        cur_pos.Offset(job_unit.left_blank_, job_unit.section_blank_);  // 增加页边距
    }

};

GridPrinter::GridPrinter()
    : impl_(new impl)
{

}

GridPrinter::~GridPrinter()
{

}

bool GridPrinter::before_print()
{
    if (max_width_ < 1200)
    {// 打印表格纸不能太小
        return false;
    }
    printer_dc_.StartPage();
    impl_->printer_dc_ = &printer_dc_;
    impl_->max_width_ = max_width_;

    COLORREF pen_col = RGB(0,0,0);
    CPen cpen(PS_SOLID, 15, pen_col);
    printer_dc_.SelectObject(&cpen);
    return true;
}

bool GridPrinter::after_print()
{
    printer_dc_.EndPage();
    return true;
}

bool GridPrinter::print_job_unit(job_unit_t &job_unit)
{
    impl_->calc_table_width(job_unit, cur_print_pos_);

    impl_->is_tb_start_ = true;

    for (int i = 0; i < 100; i++)
    {
        impl_->print_table_line(cur_print_pos_, job_unit.table_, job_unit.print_data_);

        if ((max_height_ - job_unit.section_blank_ - cur_print_pos_.y) < ch_size_.cy * 2)
        {
            printer_dc_.EndPage();
            cur_print_pos_.y = job_unit.section_blank_;
            impl_->is_tb_start_ = true;
            printer_dc_.StartPage();
        }
    }

    return true;
}

 LPDEVMODE GetLandscapeDevMode(HWND hWnd, char *pDevice)
   {

   HANDLE      hPrinter;
   LPDEVMODE   pDevMode;
   DWORD       dwNeeded, dwRet;

   /* Start by opening the printer */ 
   if (!OpenPrinter(pDevice, &hPrinter, NULL))
       return NULL;

   /*
    * Step 1:
    * Allocate a buffer of the correct size.
    */ 
   dwNeeded = DocumentProperties(hWnd,
       hPrinter,       /* Handle to our printer. */ 
       pDevice,        /* Name of the printer. */ 
       NULL,           /* Asking for size, so */ 
       NULL,           /* these are not used. */ 
       0);             /* Zero returns buffer size. */ 
   pDevMode = (LPDEVMODE)malloc(dwNeeded);

   /*
    * Step 2:
    * Get the default DevMode for the printer and
    * modify it for your needs.
    */ 
   dwRet = DocumentProperties(hWnd,
       hPrinter,
       pDevice,
       pDevMode,       /* The address of the buffer to fill. */ 
       NULL,           /* Not using the input buffer. */ 
       DM_OUT_BUFFER); /* Have the output buffer filled. */ 
   if (dwRet != IDOK)
   {
       /* If failure, cleanup and return failure. */ 
       free(pDevMode);
       ClosePrinter(hPrinter);
       return NULL;
   }

   /*
        * Make changes to the DevMode which are supported.
    */ 
   if (pDevMode->dmFields & DM_ORIENTATION)
   {
       /* If the printer supports paper orientation, set it.*/ 
       pDevMode->dmOrientation = DMORIENT_LANDSCAPE;
   }

   if (pDevMode->dmFields & DM_DUPLEX)
    {
       /* If it supports duplex printing, use it. */ 
       pDevMode->dmDuplex = DMDUP_HORIZONTAL;
   }

   /*
    * Step 3:
    * Merge the new settings with the old.
    * This gives the driver an opportunity to update any private
    * portions of the DevMode structure.
    */ 
    dwRet = DocumentProperties(hWnd,
       hPrinter,
       pDevice,
       pDevMode,       /* Reuse our buffer for output. */ 
       pDevMode,       /* Pass the driver our changes. */ 
       DM_IN_BUFFER |  /* Commands to Merge our changes and */ 
       DM_OUT_BUFFER); /* write the result. */ 

   /* Finished with the printer */ 
   ClosePrinter(hPrinter);

   if (dwRet != IDOK)
   {
       /* If failure, cleanup and return failure. */ 
       free(pDevMode);
       return NULL;
   }

   /* Return the modified DevMode structure. */ 
   return pDevMode;
}

// 设置横向打印
bool GridPrinter::set_direct()
{
    BYTE *pdata = new BYTE[sizeof(DEVMODE)];
    PDEVMODE pDevMode = (PDEVMODE)pdata;

    long lRet = ::DocumentProperties(NULL, NULL, (LPSTR)"Send To OneNote 2013", 
        pDevMode, pDevMode, DM_OUT_BUFFER);
    pDevMode->dmPaperSize=DMPAPER_A4;
    pDevMode->dmOrientation=DMORIENT_LANDSCAPE; //横向打印
    bool ret = (TRUE == printer_dc_.ResetDC(pDevMode));
    delete [] pdata;
    return ret;
}


}

