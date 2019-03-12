// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cefclient/browser/browser_window.h"

#include "include/base/cef_bind.h"
#include "cefclient/browser/main_message_loop.h"
#include "cefclient/cefclient.h"

namespace cef3 {

BrowserWindow::BrowserWindow(Delegate* delegate)
    : delegate_(delegate), is_closing_(false) {
	cef_js_msg_cb_ = nullptr;
  DCHECK(delegate_);
}

void BrowserWindow::SetDeviceScaleFactor(float device_scale_factor) {}

float BrowserWindow::GetDeviceScaleFactor() const {
  return 1.0f;
}

CefRefPtr<CefBrowser> BrowserWindow::GetBrowser() const {
  REQUIRE_MAIN_THREAD();
  return browser_;
}

void BrowserWindow::Close()
{
	REQUIRE_MAIN_THREAD();
	cef_js_msg_cb_ = nullptr;
	browser_->StopLoad();
	browser_->GetHost()->CloseBrowser(true);
	return ;
}

bool BrowserWindow::IsClosing() const {
  REQUIRE_MAIN_THREAD();
  return is_closing_;
}

void BrowserWindow::OnBrowserCreated(CefRefPtr<CefBrowser> browser) {
  REQUIRE_MAIN_THREAD();
  DCHECK(!browser_);
  browser_ = browser;
}

void BrowserWindow::OnBrowserClosing(CefRefPtr<CefBrowser> browser) {
  REQUIRE_MAIN_THREAD();
  DCHECK_EQ(browser->GetIdentifier(), browser_->GetIdentifier());
  is_closing_ = true;
}

void BrowserWindow::OnBrowserClosed(CefRefPtr<CefBrowser> browser) {
  REQUIRE_MAIN_THREAD();
  int browser_id = 0;
  if (browser_.get()) {
	browser_id = browser_->GetIdentifier();
    DCHECK_EQ(browser->GetIdentifier(), browser_->GetIdentifier());
    browser_ = NULL;
  }

  if (cef_js_msg_cb_ && is_osr_)
  {
	  CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(CEF_ON_DESTROY);
	  DisPatchProcessMsg(msg);
  }

  client_handler_->DetachDelegate();
  client_handler_ = NULL;

  // |this| may be deleted.
  delegate_->OnBrowserWindowDestroyed(browser_id);
}

void BrowserWindow::OnSetLoadingState(bool isLoading,
                                      bool canGoBack,
                                      bool canGoForward) {
  REQUIRE_MAIN_THREAD();
  delegate_->OnSetLoadingState(isLoading, canGoBack, canGoForward);
}

void BrowserWindow::OnSetDraggableRegions(
    const std::vector<CefDraggableRegion>& regions) {
  REQUIRE_MAIN_THREAD();
  delegate_->OnSetDraggableRegions(regions);
}

void BrowserWindow::DisPatchProcessMsg(CefRefPtr<CefProcessMessage> message)
{
    if (!CURRENTLY_ON_MAIN_THREAD()) {
        // Execute this method on the main thread.
        MAIN_POST_CLOSURE(base::Bind(&BrowserWindow::DisPatchProcessMsg, this, message));
    }
    else
    {
		if (browser_)
			cef_js_msg_cb_(message, browser_->GetIdentifier());
    }
}

bool BrowserWindow::OnProcessMessageReceived(CefRefPtr<CefProcessMessage> message) {
    REQUIRE_MAIN_THREAD();
    
    if (cef_js_msg_cb_ && browser_.get())
    {
        DisPatchProcessMsg(message);
        return true;
    }
	return false;
}

void BrowserWindow::OnLoadOver(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
	int httpCode,
	const CefString& errorText,
	const CefString& failedUrl)
{
	if (cef_js_msg_cb_ && browser_.get())
	{
		CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(CEF_ON_LOADOVER);
		// Retrieve the argument list object.
		CefRefPtr<CefListValue> args = msg->GetArgumentList();

		// Populate the argument values.
		args->SetSize(5);
		args->SetString(0, std::to_string(frame->GetIdentifier()));
		args->SetString(1, std::to_string(httpCode));
		args->SetString(2, errorText);
		args->SetString(3, failedUrl);
		args->SetString(4, std::to_string(browser->GetIdentifier()));
        DisPatchProcessMsg(msg);
	}
    else
    {
        LOG(ERROR) << "cef_js_msg_cb_ isnull:" << (cef_js_msg_cb_ == nullptr) << ". browser_ is null:" << browser_.get();
    }
}
void BrowserWindow::OnSetTitle(const std::string& title) 
{
	if (cef_js_msg_cb_ && browser_.get())
	{
		CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(CEF_ON_SETTITLE);
		// Retrieve the argument list object.
		CefRefPtr<CefListValue> args = msg->GetArgumentList();

		// Populate the argument values.
		args->SetSize(2);
		args->SetString(0, std::to_string(browser_->GetIdentifier()));
		args->SetString(1, title);
		DisPatchProcessMsg(msg);
	}
	else
	{
		LOG(ERROR) << "cef_js_msg_cb_ isnull:" << (cef_js_msg_cb_ == nullptr) << ". browser_ is null:" << browser_.get();
	}
}

void BrowserWindow::OnPopup(CefRefPtr<CefBrowser> browser,
							CefRefPtr<CefFrame> frame, 
							const std::string& target_url)
{
	if (cef_js_msg_cb_ && browser_.get())
	{
		CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(CEF_ON_POPUP);
		// Retrieve the argument list object.
		CefRefPtr<CefListValue> args = msg->GetArgumentList();

		// Populate the argument values.
		args->SetSize(3);
		args->SetString(0, std::to_string(frame->GetIdentifier()));
		args->SetString(1, std::to_string(browser->GetIdentifier()));
		args->SetString(2, target_url);
		DisPatchProcessMsg(msg);
	}
	else
	{
		LOG(ERROR) << "cef_js_msg_cb_ isnull:" << (cef_js_msg_cb_ == nullptr) << ". browser_ is null:" << browser_.get();
	}
}

void BrowserWindow::OnSetFavicon(CefRefPtr<CefImage> image)
{
	int w, h;
	auto img_buff = image->GetAsPNG(1.0, true, w, h);

	if (cef_js_msg_cb_ && browser_.get() && img_buff)
	{
		CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(CEF_ON_FAVICON);
		// Retrieve the argument list object.
		CefRefPtr<CefListValue> args = msg->GetArgumentList();

		// Populate the argument values.
		args->SetSize(4);
		args->SetString(0, std::to_string(browser_->GetIdentifier()));
		args->SetString(1, std::to_string(w));
		args->SetString(2, std::to_string(h));
		args->SetBinary(3, img_buff);
		DisPatchProcessMsg(msg);
	}
	else
	{
		LOG(ERROR) << "cef_js_msg_cb_ isnull:" << (cef_js_msg_cb_ == nullptr) << ". browser_ is null:" << browser_.get();
	}
}

}  // namespace client
