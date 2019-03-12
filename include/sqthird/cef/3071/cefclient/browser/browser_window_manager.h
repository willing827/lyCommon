// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFCLIENT_BROWSER_ROOT_WINDOW_MANAGER_H_
#define CEF_TESTS_CEFCLIENT_BROWSER_ROOT_WINDOW_MANAGER_H_
#pragma once

#include <set>

#include "include/base/cef_scoped_ptr.h"
#include "include/cef_command_line.h"
#include "cefclient/browser/browser_window.h"

namespace cef3 {

// Used to create/manage RootWindow instances. The methods of this class can be
// called from any browser process thread unless otherwise indicated.
class BrowserManager : public BrowserWindow::Delegate {
 public:
  // If |terminate_when_all_windows_closed| is true quit the main message loop
  // after all windows have closed.
  explicit BrowserManager(bool terminate_when_all_windows_closed);

  // Create a new top-level native window that loads |url|.
  // This method can be called from anywhere to create a new browser window.
  CefRefPtr<BrowserWindow> CreateBrowser(ClientWindowHandle parent_handle, bool with_osr,
									  const CefRect& bounds, const std::string& url);

  // Returns the RootWindow associated with the specified browser ID. Must be
  // called on the main thread.
  CefRefPtr<CefBrowser> GetBrowser(int browser_id);

  // Close all existing windows. If |force| is true onunload handlers will not
  // be executed.
  void CloseAllBrowsers(bool force);

  bool IsDestroyed() { return browser_windows_.empty(); }

  std::set<int> LeftBrowsers();

  bool IsEditing(int browser_id);

  CefRefPtr<CefRequestContext> GetRequestContext(BrowserWindow* browser_window);

 private:
  // Allow deletion via scoped_ptr only.
  friend struct base::DefaultDeleter<BrowserManager>;

  ~BrowserManager();

  void OnBrowserWindowCreated(CefRefPtr<BrowserWindow> root_window);
  CefRefPtr<BrowserWindow> GetBrowserWindow(int browser_id);

  // BrowserWindow::Delegate methods.
  void OnBrowserWindowDestroyed(int browser_id) OVERRIDE;
  void OnSetLoadingState(bool isLoading, bool canGoBack, bool canGoForward) OVERRIDE;
  void OnSetDraggableRegions(const std::vector<CefDraggableRegion>& regions) OVERRIDE;

  const bool terminate_when_all_windows_closed_;
  bool request_context_per_browser_;
  bool request_context_shared_cache_;

  // Existing root windows. Only accessed on the main thread.
  typedef std::map<int, CefRefPtr<BrowserWindow>> BrowserWindowMap;
  BrowserWindowMap browser_windows_;

  CefRefPtr<CefRequestContext> shared_request_context_;

  DISALLOW_COPY_AND_ASSIGN(BrowserManager);
};

}  // namespace client

#endif  // CEF_TESTS_CEFCLIENT_BROWSER_ROOT_WINDOW_MANAGER_H_
