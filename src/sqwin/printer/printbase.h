#pragma once
#include <string>
#include <vector>
#include <memory>


namespace snqu {

	struct printer_info_t
	{
		std::string printer_name_;
		std::string port_name_;
		std::string driver_name_;
		bool is_off_line_;
		printer_info_t()
			: is_off_line_(false)
		{}
	};

	struct job_unit_t
	{
		std::string font_name_;     // 字体名称
		int font_size_;             // 字体大小(10-30)
		int left_blank_;            // 左边距
		int section_blank_;         // 按段分隔
		int line_blank_;            // 行间距

		int align_;                 // 对齐方式: 0左对齐 1中间对齐 2右对齐
		std::vector<int> table_;    // table间隔 只有在is_table_为true时有效
		std::string print_data_;    // 打印的数据内容 \t表格分隔 \n换行

		job_unit_t()
			: font_size_(12)
			, left_blank_(0)
			, section_blank_(0)
			, align_(0)
			, line_blank_(0)
		{}
	};


	struct print_job_t
	{
		std::string printer_name_;
		std::string job_name_;
		std::vector<job_unit_t> job_list_;
	};

	typedef std::shared_ptr<print_job_t> print_job_ptr_t;

}