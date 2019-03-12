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
		std::string font_name_;     // ��������
		int font_size_;             // �����С(10-30)
		int left_blank_;            // ��߾�
		int section_blank_;         // ���ηָ�
		int line_blank_;            // �м��

		int align_;                 // ���뷽ʽ: 0����� 1�м���� 2�Ҷ���
		std::vector<int> table_;    // table��� ֻ����is_table_Ϊtrueʱ��Ч
		std::string print_data_;    // ��ӡ���������� \t���ָ� \n����

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