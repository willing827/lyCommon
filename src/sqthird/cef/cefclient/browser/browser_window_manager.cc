// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cefclient/browser/browser_window_manager.h"

#include <sstream>

#include "include/base/cef_bind.h"
#include "include/base/cef_logging.h"
#include "include/wrapper/cef_helpers.h"
#include "cefclient/browser/main_context.h"
#include "../common/client_switches.h"
#include "cefclient/browser/main_message_loop.h"
#include "../browser/browser_window_std_win.h"
#include "../browser/browser_window_osr_win.h"

namespace cef3 {

namespace {

class ClientRequestContextHandler : public CefRequestContextHandler {
 public:
  ClientRequestContextHandler() {}

  bool OnBeforePluginLoad(const CefString& mime_type,
                          const CefString& plugin_url,
                          bool is_main_frame,
                          const CefString& top_origin_url,
                          CefRefPtr<CefWebPluginInfo> plugin_info,
                          PluginPolicy* plugin_policy) OVERRIDE {
    // Always allow the PDF plugin to load.
    if (*plugin_policy != PLUGIN_POLICY_ALLOW && mime_type == "application/pdf") {
      *plugin_policy = PLUGIN_POLICY_ALLOW;
      return true;
    }

    return false;
  }

 private:
  IMPLEMENT_REFCOUNTING(ClientRequestContextHandler);
};

}  // namespace

BrowserManager::BrowserManager(bool terminate_when_all_windows_closed)
    : terminate_when_all_windows_closed_(terminate_when_all_windows_closed) {
  CefRefPtr<CefCommandLine> command_line =
      CefCommandLine::GetGlobalCommandLine();
  DCHECK(command_line.get());
  request_context_per_browser_ = command_line->HasSwitch(switches::kRequestContextPerBrowser);
  request_context_shared_cache_ = command_line->HasSwitch(switches::kRequestContextSharedCache);
}

BrowserManager::~BrowserManager() {
  // All root windows should already have been destroyed.
  DCHECK(browser_windows_.empty());
}

CefRefPtr<BrowserWindow> BrowserManager::CreateBrowser(ClientWindowHandle parent_handle, bool with_osr,
											        const CefRect& bounds, const std::string& url)
{
	CefBrowserSettings settings = {};
	MainContext::Get()->PopulateBrowserSettings(&settings);
	scoped_refptr<BrowserWindow> browser_window;
	if (with_osr) {
		OsrRenderer::Settings osrsettings = {};
		MainContext::Get()->PopulateOsrSettings(&osrsettings);
		browser_window = (new BrowserWindowOsrWin(this, url, osrsettings));
	}
	else {
		browser_window = (new BrowserWindowStdWin(this, url));
	}

	browser_window->CreateBrowser(parent_handle, bounds, settings, NULL);

	// Store a reference to the window on the main thread.
	OnBrowserWindowCreated(browser_window);

	return browser_window;
}

CefRefPtr<CefBrowser> BrowserManager::GetBrowser(int browser_id) {
  REQUIRE_MAIN_THREAD();

  auto it = browser_windows_.find(browser_id);
  if (it != browser_windows_.end())
  {
	  CefRefPtr<CefBrowser> browser = it->second->GetBrowser();
	  if (browser.get() && browser->GetIdentifier() == browser_id)
		  return browser;
  }

  return NULL;
}

void BrowserManager::CloseBrowser(int browser_id) {
	REQUIRE_MAIN_THREAD();

	auto it = browser_windows_.find(browser_id);
	if (it != browser_windows_.end())
	{
		CefRefPtr<CefBrowser> browser = it->second->GetBrowser();
		if (browser.get() && browser->GetIdentifier() == browser_id)
		{
			
			it->second->Close();
		}
	}
}

bool BrowserManager::IsEditing(int browser_id) {
	REQUIRE_MAIN_THREAD();

	auto it = browser_windows_.find(browser_id);
	if (it != browser_windows_.end())
	{
		return it->second->IsEditing();
	}

	return false;
}

CefRefPtr<BrowserWindow> BrowserManager::GetBrowserWindow(int browser_id)
{
	REQUIRE_MAIN_THREAD();

	auto it = browser_windows_.find(browser_id);
	if (it != browser_windows_.end())
		return it->second;
	return NULL;
}

void BrowserManager::CloseAllBrowsers(bool force) {
  if (!CURRENTLY_ON_MAIN_THREAD()) {
    // Execute this method on the main thread.
    MAIN_POST_CLOSURE(base::Bind(&BrowserManager::CloseAllBrowsers, base::Unretained(this), force));
    return;
  }

  if (browser_windows_.empty())
    return;

  for (auto& item : browser_windows_)
  {
	  auto browser_window = item.second;
	  if (browser_window && !browser_window->IsClosing()) {
		  browser_window->Close();
	  }
  }
}

std::set<int> BrowserManager::LeftBrowsers()
{
    std::set<int> ret_val;

    for (auto& item : browser_windows_)
    {
        ret_val.insert(item.first);
    }

    return ret_val;
}

void BrowserManager::OnBrowserWindowCreated(CefRefPtr<BrowserWindow> browser_window)
{
  if (!CURRENTLY_ON_MAIN_THREAD()) {
    // Execute this method on the main thread.
    MAIN_POST_CLOSURE(base::Bind(&BrowserManager::OnBrowserWindowCreated, base::Unretained(this), browser_window));
    return;
  }

  if (browser_window->GetBrowser())
	browser_windows_.insert(std::make_pair(browser_window->GetBrowser()->GetIdentifier(), browser_window));
}

CefRefPtr<CefRequestContext> BrowserManager::GetRequestContext(BrowserWindow* browser_window) {
  REQUIRE_MAIN_THREAD();

  if (request_context_per_browser_) {
    // Create a new request context for each browser.
    CefRequestContextSettings settings;

    CefRefPtr<CefCommandLine> command_line = CefCommandLine::GetGlobalCommandLine();
    if (command_line->HasSwitch(switches::kCachePath)) {
      if (request_context_shared_cache_) {
        // Give each browser the same cache path. The resulting context objects
        // will share the same storage internally.
        CefString(&settings.cache_path) = command_line->GetSwitchValue(switches::kCachePath);
      } else {
        // Give each browser a unique cache path. This will create completely
        // isolated context objects.
        std::stringstream ss;
        ss << command_line->GetSwitchValue(switches::kCachePath).ToString() << time(NULL);
        CefString(&settings.cache_path) = ss.str();
      }
    }

    return CefRequestContext::CreateContext(settings, new ClientRequestContextHandler);
  }

  // All browsers will share the global request context.
  if (!shared_request_context_.get()) {
    shared_request_context_ = CefRequestContext::CreateContext(CefRequestContext::GetGlobalContext(), new ClientRequestContextHandler);
  }

  return shared_request_context_;
}

void BrowserManager::OnBrowserWindowDestroyed(int browser_id)
{
	REQUIRE_MAIN_THREAD();

	auto it = browser_windows_.find(browser_id);
	DCHECK(it != browser_windows_.end());
	if (it != browser_windows_.end())
	{
		browser_windows_.erase(it);
	}

	if (terminate_when_all_windows_closed_ && browser_windows_.empty()) {
		// Nofify Quit when all windows have closed.
		MainMessageLoop::Get()->Quit();
	}
}

void BrowserManager::OnSetLoadingState(bool isLoading, bool canGoBack, bool canGoForward)
{

}

void BrowserManager::OnSetDraggableRegions(const std::vector<CefDraggableRegion>& regions)
{

}

}  // namespace client
