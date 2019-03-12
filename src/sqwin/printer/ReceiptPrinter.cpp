#include "ReceiptPrinter.h"
#include <sqwin/printer/PrintHelper.h>
#include <sqstd/sqstringhelper.h>


namespace snqu {

	struct ReceiptPrinter::impl
	{
		CDC *printer_dc_;
		int max_width_;

		impl()
			: max_width_(0)
			, printer_dc_(NULL)
		{
		}

		bool print_table(int x, long &y_off, std::vector<int> &tab, const std::string &text, int align)
		{
			CRect rectPrint;
			unsigned int row = tab.size();

			std::vector<std::string> line_data;
			str::split(text, "\t");
			std::string tmp_str;
			int max_h_move = 0;
			int x_move = 0;

			for (unsigned int r = 0; r < row; r++)
			{
				rectPrint.SetRect(x + x_move, y_off, tab[r], y_off + tab[r]);
				UINT text_algin = DT_WORDBREAK;
				if (0 != r)
					text_algin |= align;

				auto move = DrawTextA(printer_dc_->GetSafeHdc(), line_data[r].c_str(),
					line_data[r].length(), rectPrint, text_algin); // 打印到缓冲区

				if (0 == move)
					return false;

				x_move = tab[r]; // 右偏移
				if (max_h_move < move)
					max_h_move = move; // 纵向最大偏移
			}

			rectPrint.top += max_h_move;
			y_off = rectPrint.top;
			return true;
		}

		bool print_line(long &y_off)
		{
			COLORREF pen_col = RGB(0, 0, 0);
			CPen cpen(PS_DASH, 1, pen_col); // 打出来是虚线
			y_off += 10;
			printer_dc_->SelectObject(&cpen);
			CPoint pt(0, y_off);
			printer_dc_->MoveTo(pt);
			CPoint pt2(max_width_, y_off);
			y_off += 15;
			return TRUE == printer_dc_->LineTo(pt2);
		}

		bool print_text(int x, long &y_off, job_unit_t &job_unit, const std::string &text)
		{
			if (std::string::npos != text.find("--"))
			{// 打印横线
				return print_line(y_off);
			}

			if (std::string::npos == text.find('\t'))
			{// 打印一行
				CRect rectPrint(0, y_off, max_width_, y_off + max_width_);

				//X方向向右增加，Y方向向上增加
				printer_dc_->SetTextAlign(TA_TOP | TA_LEFT);  //设置对齐

				UINT text_align = DT_WORDBREAK | job_unit.align_;
				auto move = DrawTextA(printer_dc_->GetSafeHdc(), text.c_str(), text.length(), rectPrint, text_align);//打印到缓冲区

				if (0 == move)
					return false;

				rectPrint.top += move;
				y_off = rectPrint.top;
				return true;
			}

			// 打印表格
			print_table(job_unit.left_blank_, y_off, job_unit.table_, text, job_unit.align_);

			return true;
		}

		void calc_table_width(job_unit_t &job_unit, long &y_off)
		{
			if (job_unit.table_.size() > 0)
			{
				for (unsigned int row = 0; row < job_unit.table_.size(); row++)
				{// 按百分比分配宽度
					job_unit.table_[row] = job_unit.table_[row] * max_width_ / 100;
				}

				job_unit.table_.push_back(max_width_);  // 补充最大宽度
			}

			y_off += job_unit.section_blank_;       // 增加段间距
		}

	};

	ReceiptPrinter::ReceiptPrinter()
		: impl_(new impl)
	{

	}

	ReceiptPrinter::~ReceiptPrinter()
	{
		impl_.reset(nullptr);
	}

	bool ReceiptPrinter::before_print()
	{
		printer_dc_.StartPage();
		impl_->printer_dc_ = &printer_dc_;
		impl_->max_width_ = max_width_;
		return true;
	}

	bool ReceiptPrinter::after_print()
	{
		printer_dc_.EndPage();

		//结尾处留点空白，没找到好方法，暂时如此处理
		printer_dc_.StartPage();
		printer_dc_.EndPage();
		printer_dc_.StartPage();
		printer_dc_.EndPage();
		return true;
	}

	bool ReceiptPrinter::print_job_unit(job_unit_t &job_unit)
	{
		impl_->calc_table_width(job_unit, cur_print_pos_.y);

		int str_head(0), str_pos(0);

		//必须有一个换行符号
		while ((str_pos = job_unit.print_data_.find_first_of(L'\n', str_head)) != -1)
		{
			// 按行分解文字
			if (str_head != 0)
				cur_print_pos_.Offset(0, job_unit.line_blank_);  // 行间距

			if (print_str_.rfind(L'\r', str_pos) != -1) // 输入的字符串有回车符，替换掉  
				print_str_ = job_unit.print_data_.substr(str_head, str_pos - 1);
			else
				print_str_ = job_unit.print_data_.substr(str_head, str_pos - str_head);

			str_head = str_pos + 1;

			if (!print_str_.empty() && !impl_->print_text(0, cur_print_pos_.y, job_unit, print_str_))
			{
				return false;
			}
		}

		cur_print_pos_.Offset(0, 5); // 最后一行稍微移动点

		return true;
	}
}

