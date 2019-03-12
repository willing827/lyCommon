#include "cefclient/cefclient.h"

#include "include/base/cef_bind.h"
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/cef_parser.h"
#include "include/cef_ssl_status.h"
#include "include/cef_x509_certificate.h"
#include "include/wrapper/cef_closure_task.h"
#include "cefclient/browser/main_context.h"
#include "cefclient/browser/browser_window_manager.h"
#include "common/client_switches.h"
#include "cefclient/browser/main_message_loop.h"
#include "cefclient/browser/util_win.h"
#include "cefclient/browser/dialog_handler.h"
#include "browser/main_message_loop_std.h"
#include "cefclient/browser/util_win.h"

namespace cef3clt {

	scoped_ptr<cef3::MainContextImpl>& GetContex()
	{
		static scoped_ptr<cef3::MainContextImpl> g_cef_context;
		return g_cef_context;
	}

	bool Init(bool is_dev)
	{
		// Create the main context object.
		GetContex().reset(new cef3::MainContextImpl());

		// Initialize CEF.
		return GetContex()->Initialize(is_dev);
	}

	void Uninit()
	{
		// Shut down CEF.
		GetContex()->Shutdown();
		GetContex().reset();
	}

	CefRefPtr<cef3::BrowserWindow> CreateBrowser(ClientWindowHandle parent_handle, 
		cef3::CefMSgCallBack javascript_callback_handler,
		const std::string& url, const CefRect& rect, bool with_osr)
	{
		if (!GetContex().get()) return NULL;

		auto ret_ptr = GetContex()->GetBrowserWindowManager()->CreateBrowser(parent_handle, with_osr, rect, url);

		if (ret_ptr && javascript_callback_handler)
		{
			ret_ptr->RegisterMessageHandler(javascript_callback_handler, with_osr);
		}
		return ret_ptr;
	}

	void CloseBrowser(int browser_id)
	{
		if (!GetContex()) return;

		GetContex()->GetBrowserWindowManager()->CloseBrowser(browser_id);
	}

	CefRefPtr<CefBrowser> GetBrowser(int browser_id)
	{
		if (!GetContex()) return NULL;

		return GetContex()->GetBrowserWindowManager()->GetBrowser(browser_id);
	}

	void LoadUrl(int browser_id, const CefString& url)
	{
		auto browser_ptr = GetBrowser(browser_id);
		if (NULL == browser_ptr)
			return;

		browser_ptr->GetMainFrame()->LoadURL(url.c_str());
	}

	void LoadBlank(int browser_id)
	{
		auto browser_ptr = GetBrowser(browser_id);
		if (NULL == browser_ptr)
			return;

		std::stringstream ss;
		ss << "<html><head><title>Page Blank</title></head>"
			"<body bgcolor=\"white\">";
		ss << "</body></html>";
		browser_ptr->GetMainFrame()->LoadURL(cef3::GetDataURI(ss.str(), "text/html"));
	}

    bool LoadRequest(int browser_id, const CefString& url, const CefString& frame_name)
    {
        auto browser_ptr = GetBrowser(browser_id);
        if (NULL == browser_ptr)
            return false;

        auto req = CefRequest::Create();
        req->SetFirstPartyForCookies(url);
        req->SetFlags(UR_FLAG_ALLOW_STORED_CREDENTIALS);
        req->SetMethod("POST");
        req->SetURL(url.c_str());
		std::vector<CefString> test;
		browser_ptr->GetFrameNames(test);
		if (frame_name.empty())
			browser_ptr->GetMainFrame()->LoadRequest(req);
		else
		{
			auto frame = browser_ptr->GetFrame(frame_name);
			if (!frame)
				return false;
			frame->LoadRequest(req);
		}

		return true;
    }

	void ExecuteJavaScript(int browser_id, const CefString& js_data)
	{
		auto browser_ptr = GetBrowser(browser_id);
		if (NULL == browser_ptr)
			return;

		browser_ptr->GetMainFrame()->ExecuteJavaScript(js_data.c_str(), "", 0);
	}

	bool IsEditing(int browser_id)
	{
		return GetContex()->GetBrowserWindowManager()->IsEditing(browser_id);
	}
}