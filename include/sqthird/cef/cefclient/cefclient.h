#pragma once
#include "browser/main_context_impl.h"
#include <tchar.h>

namespace cef3clt {

	scoped_ptr<cef3::MainContextImpl>& GetContex();
	bool Init(bool has_log = false);
	void Uninit();

	//************************************
	// Method:    CreateBrowser
	// FullName:  cef3clt::CreateBrowser
	// Returns:   CefRefPtr<cef3::BrowserWindow>
	// Parameter: cefǶ��ĸ�����
	// Parameter: java�ű����õĴ�����
	// Parameter: is off-screen rendering
	// Parameter: cefǶ�������
	// Parameter: Ĭ�ϴ򿪵�URL
	//************************************
	CefRefPtr<cef3::BrowserWindow> CreateBrowser(ClientWindowHandle parent_handle, 
						cef3::CefMSgCallBack javascript_callback_handler = nullptr,
						const std::string& url = "", const CefRect& rect = CefRect(), 
						bool with_osr = false);

	//�첽����
	void CloseBrowser(int browser_id);

	CefRefPtr<CefBrowser> GetBrowser(int browser_id);// ��������

	void LoadUrl(int browser_id, const CefString& url);
    // ����һ�����󣬻��cookie��ҳ����ת
	bool LoadRequest(int browser_id, const CefString& url, const CefString& frame_name = "");
	// ����һ���հ�ҳ�� ���㴰�ڴ�������
	void LoadBlank(int browser_id);

	//************************************
	// Method:    ExecuteJavaScript
	// FullName:  cef3clt::ExecuteJavaScript
	// Parameter: cef����ID
	// Parameter: Ҫִ�е�java script�ű�����
	//************************************
	void ExecuteJavaScript(int browser_id, const CefString& js_data);

	bool IsEditing(int browser_id);

	/* 
		��CreateBrowser��ע���cef3::CefMSgCallBack�������µ�ϵͳ�ص���Ϣ������Ϊ��ҳJS����Js2Cpp_Event����
		ҳ�������ɺ����Ϣ
		"OnLoadOver" param_list: 0->frame_id 1->httpcode 2->errorText 3->failedurl 4->browserid
		��WEB�Ĵ��ڶ����ٺ������ٴ��ڲ��ǰ�ȫ�ģ�ע����CEF��������⣬������Ⱦ���д���Ϣ
		"OnDestroy"  param_list: NULL
		ҳ���title�����仯ʱ��������Ϣ
		"OnSetTitle" param_list: 0->browserid 1->title
		ҳ��Ҫpop windowʱ��������Ϣ
		"OnPopup" param_list: 0->frame_id 1->browserid 2->target_url���´��ڵ�ַ��
		ҳ�������favicon�ͻᴥ������Ϣ
		"OnSetFavicon" param_list: 0->browserid 1->png_width 2->png_height  3->png_binary_data CefRefPtr<CefBinaryValue>
	*/
#define CEF_ON_LOADOVER _T("OnLoadOver")
#define CEF_ON_DESTROY  _T("OnDestroy")
#define CEF_ON_SETTITLE _T("OnSetTitle")
#define CEF_ON_POPUP	_T("OnPopup")
#define CEF_ON_FAVICON	_T("OnSetFavicon")

	/*
		������Ⱦ�»ᷢ�͵Ĵ�����Ϣ

	*/
	enum CefWMMsg
	{
		CEF_WM_BEGAIN,
		CEF_WM_LBUTTONDOWN = WM_USER + 1,
		CEF_WM_LBUTTONUP   = WM_USER + 2,
		CEF_WM_MOUSEMOVE   = WM_USER + 3,
		CEF_WM_END,
	};
}

