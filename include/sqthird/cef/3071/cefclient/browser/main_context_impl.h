// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFCLIENT_BROWSER_MAIN_CONTEXT_IMPL_H_
#define CEF_TESTS_CEFCLIENT_BROWSER_MAIN_CONTEXT_IMPL_H_
#pragma once

#include "include/base/cef_scoped_ptr.h"
#include "include/base/cef_thread_checker.h"
#include "include/cef_app.h"
#include "include/cef_command_line.h"
#include "cefclient/browser/main_context.h"
#include "cefclient/browser/browser_window_manager.h"
#include <thread>

namespace cef3 {

// Used to store global context in the browser process.
class MainContextImpl : public MainContext {
 public:
  MainContextImpl(bool terminate_when_all_windows_closed = true);

  // MainContext members.
  std::string GetConsoleLogPath() OVERRIDE;
  std::string GetDownloadPath(const std::string& file_name) OVERRIDE;
  std::string GetAppWorkingDirectory() OVERRIDE;
  std::string GetMainURL() OVERRIDE;
  cef_color_t GetBackgroundColor() OVERRIDE;
  bool UseViews() OVERRIDE;
  bool UseWindowlessRendering() OVERRIDE;
  void PopulateSettings(CefSettings* settings) OVERRIDE;
  void PopulateBrowserSettings(CefBrowserSettings* settings) OVERRIDE;
  void PopulateOsrSettings(OsrRenderer::Settings* settings) OVERRIDE;
  BrowserManager* GetBrowserWindowManager() OVERRIDE;

  // Initialize CEF and associated main context state. This method must be
  // called on the same thread that created this object.
  bool Initialize(bool is_dev);

  // …Ë÷√—’…´
  void SetBackgroundColor(cef_color_t color);

  // Shut down CEF and associated context state. This method must be called on
  // the same thread that created this object.
  void Shutdown();

  void run();

  void quit();

  bool is_dev();

 private:
  // Allow deletion via scoped_ptr only.
  friend struct base::DefaultDeleter<MainContextImpl>;

  ~MainContextImpl();

  // Returns true if the context is in a valid state (initialized and not yet
  // shut down).
  bool InValidState() const { return initialized_ && !shutdown_; }

  CefRefPtr<CefCommandLine> command_line_;
  const bool terminate_when_all_windows_closed_;

  // Track context state. Accessing these variables from multiple threads is
  // safe because only a single thread will exist at the time that they're set
  // (during context initialization and shutdown).
  bool initialized_;
  bool shutdown_;

  std::string main_url_;
  cef_color_t background_color_;
  cef_color_t browser_background_color_;
  bool use_windowless_rendering_;
  bool use_views_;
  bool is_dev_;
  CefRefPtr<CefApp> cef_app_;

  scoped_ptr<BrowserManager> browser_window_manager_;

  // Used to verify that methods are called on the correct thread.
  base::ThreadChecker thread_checker_;

  scoped_ptr<MainMessageLoop> messsage_loop_;

  int process_type_;

  DISALLOW_COPY_AND_ASSIGN(MainContextImpl);
};

}  // namespace client

#endif  // CEF_TESTS_CEFCLIENT_BROWSER_MAIN_CONTEXT_IMPL_H_
