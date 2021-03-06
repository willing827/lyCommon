
// SnlogDecodeDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "SnlogDecode.h"
#include "SnlogDecodeDlg.h"
#include "afxdialogex.h"

#include <string>
#include "sqsafe/sqsafemodel.h"
#include "sqstd/sqsysfun.h"
#include "sqstd/sqconvert.h"
#include <iosfwd>
#include "Resource.h"
#include "codec/sqcodec.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSnlogDecodeDlg 对话框



CSnlogDecodeDlg::CSnlogDecodeDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SNLOGDECODE_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSnlogDecodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSnlogDecodeDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DROPFILES()
END_MESSAGE_MAP()


// CSnlogDecodeDlg 消息处理程序

BOOL CSnlogDecodeDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	const char av_key[] = { 'S', 'N', 'Q', 'U', '8', '8', '8', '8', '8', '8', '8', '8', '\0' };
	SetDlgItemTextA(AfxGetApp()->m_pMainWnd->m_hWnd, IDC_EDIT_KEY, av_key);

	//Decode("E:\\new_v6dj\\sqcommon\\V6Esport_180810_145118879.log");

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CSnlogDecodeDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CSnlogDecodeDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSnlogDecodeDlg::Decode(const char* filePath)
{
	std::string file(filePath);
	std::string txt = snqu::load_file(file);
	CString strKey;
	GetDlgItemText(IDC_EDIT_KEY, strKey);
	std::string key = snqu::codec::W2S(strKey.GetBuffer());

	auto func = [](const std::string& src_data, const std::string& pukey)
	{
		return safe::ISQSafeModel::std_algo()->rc4_algofx(src_data, pukey);
	};

	std::string decode_text = "";
	size_t read_index = 0;
	size_t txtSize = txt.size();
	while (read_index < txtSize) {
		std::string code_len = txt.substr(read_index, 4);
		int* pData = (int*)code_len.c_str();
		int data = *pData;
		int data2 = data + 9832;
		int data3 = ntohl(data2);
		if (data3 < (std::numeric_limits<unsigned short>::min)() ||
			data3 > (std::numeric_limits<unsigned short>::max)()) {
			//单次数据太长，可能数据错误，退出循环直接写文件
			break;
		}
		else {
			std::string code_txt = txt.substr(read_index + 4, data3);
			std::string decode = func(code_txt, key);
			decode_text.append(decode);
			read_index += 4 + data3;
		}
	}

	//
	size_t pos = file.find_last_of('.');
	if (pos != std::string::npos) {
		std::string name = file.substr(0, pos);
		name += "_decrypt.log";
		snqu::save_file(name, decode_text);
	}
}


void CSnlogDecodeDlg::OnDropFiles(HDROP hDropInfo)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	int DropCount = DragQueryFile(hDropInfo, -1, NULL, 0);//取得被拖动文件的数目
	if (DropCount != 1) {
		MessageBox(L"请不要一次拖拽多个文件!");
		return;
	}

	char wcStr[MAX_PATH];
	DragQueryFileA(hDropInfo, 0, wcStr, MAX_PATH);

	Decode(wcStr);

	DragFinish(hDropInfo);  //拖放结束后,释放内存


	CDialogEx::OnDropFiles(hDropInfo);
}
