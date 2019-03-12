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
	// Parameter: cef嵌入的父窗口
	// Parameter: java脚本调用的处理函数
	// Parameter: is off-screen rendering
	// Parameter: cef嵌入的区域
	// Parameter: 默认打开的URL
	//************************************
	CefRefPtr<cef3::BrowserWindow> CreateBrowser(ClientWindowHandle parent_handle, 
						cef3::CefMSgCallBack javascript_callback_handler = nullptr,
						const std::string& url = "", const CefRect& rect = CefRect(), 
						bool with_osr = false);

	CefRefPtr<CefBrowser> GetBrowser(int browser_id);

	void LoadUrl(int browser_id, const CefString& url);
    // 发送一个请求，会带cookie和页面跳转
    void LoadRequest(int browser_id, const CefString& url);


	//************************************
	// Method:    ExecuteJavaScript
	// FullName:  cef3clt::ExecuteJavaScript
	// Parameter: cef窗口ID
	// Parameter: 要执行的java script脚本内容
	//************************************
	void ExecuteJavaScript(int browser_id, const CefString& js_data);

	bool IsEditing(int browser_id);

	/* 
		在CreateBrowser中注册的cef3::CefMSgCallBack会有如下的系统回调消息，其它为网页JS调用Js2Cpp_Event生成
		页面加载完成后的消息
		"OnLoadOver" param_list: 0->frame_id 1->httpcode 2->errorText 3->failedurl 4->browserid
		在WEB的窗口都销毁后再销毁窗口才是安全的，注意多个CEF对象的问题，离屏渲染才有此消息
		"OnDestroy"  param_list: NULL
	*/
#define CEF_ON_LOADOVER _T("OnLoadOver")
#define CEF_ON_DESTROY  _T("OnDestroy")

	/*
		离屏渲染下会发送的窗口消息

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

