// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cefclient/browser/main_context_impl.h"

#include "include/cef_parser.h"
#include "include/cef_web_plugin.h"
#include "../common/client_switches.h"
#include "../common/client_app.h"
#include "../browser/client_app_browser.h"
#include "../renderer/client_app_renderer.h"
#include "../common/client_app_other.h"
#include "../browser/main_message_loop_multithreaded_win.h"
#include "../browser/main_message_loop_external_pump.h"

namespace cef3 {

namespace {

// The default URL to load in a browser window.
const char kDefaultUrl[] = "http://www.baidu.com";

// Returns the ARGB value for |color|.
cef_color_t ParseColor(const std::string& color) {
  std::string colorToLower;
  colorToLower.resize(color.size());
  std::transform(color.begin(), color.end(), colorToLower.begin(), ::tolower);

  if (colorToLower == "black")
    return CefColorSetARGB(255, 0, 0, 0);
  else if (colorToLower == "blue")
    return CefColorSetARGB(255, 0, 0, 255);
  else if (colorToLower == "green")
    return CefColorSetARGB(255, 0, 255, 0);
  else if (colorToLower == "red")
    return CefColorSetARGB(255, 255, 0, 0);
  else if (colorToLower == "white")
    return CefColorSetARGB(255, 255, 255, 255);

  // Use the default color.
  return 0;
}

}  // namespace

MainContextImpl::MainContextImpl(bool terminate_when_all_windows_closed)
    : terminate_when_all_windows_closed_(terminate_when_all_windows_closed),
      initialized_(false),
      shutdown_(false),
      background_color_(0),
      browser_background_color_(0),
      use_views_(false),
	  cef_app_(nullptr),
	  process_type_(0), is_dev_(false),
	 command_line_(nullptr)
{
  // Enable High-DPI support on Windows 7 or newer.
  CefEnableHighDPISupport();
  // Parse command-line arguments.
  command_line_ = CefCommandLine::CreateCommandLine();
  if (!command_line_)
	  return;
   
  command_line_->InitFromString(::GetCommandLineW());

  DCHECK(command_line_.get());

  // Create a ClientApp of the correct type.
#ifdef WEB_RENDER
	  cef_app_ = new cef3::ClientAppRenderer();
#else
  process_type_ = ClientApp::GetProcessType(command_line_);
  if (process_type_ == cef3::ClientApp::BrowserProcess)
	  cef_app_ = new cef3::ClientAppBrowser();
  else if (process_type_ == cef3::ClientApp::RendererProcess)
	  cef_app_ = new cef3::ClientAppRenderer();
  else if (process_type_ == cef3::ClientApp::OtherProcess)
	  cef_app_ = new cef3::ClientAppOther();
#endif


  // Set the main URL.
  if (command_line_->HasSwitch(switches::kUrl))
    main_url_ = command_line_->GetSwitchValue(switches::kUrl);
  if (main_url_.empty())
    main_url_ = kDefaultUrl;

  // Whether windowless (off-screen) rendering will be used.
  use_windowless_rendering_ =
      command_line_->HasSwitch(switches::kOffScreenRenderingEnabled);

  // Whether transparent painting is used with windowless rendering.
  const bool use_transparent_painting =
      use_windowless_rendering_ &&
      command_line_->HasSwitch(switches::kTransparentPaintingEnabled);

  // Whether the Views framework will be used.
  use_views_ = command_line_->HasSwitch(switches::kUseViews);

  if (use_windowless_rendering_ && use_views_) {
    LOG(ERROR)
        << "Windowless rendering is not supported by the Views framework.";
    use_views_ = false;
  }

  if (command_line_->HasSwitch(switches::kBackgroundColor)) {
    // Parse the background color value.
    background_color_ = ParseColor(command_line_->GetSwitchValue(switches::kBackgroundColor));
  }

  if (background_color_ == 0 && !use_views_) {
    // Set an explicit background color.
    background_color_ = ParseColor("white");
  }

  // |browser_background_color_| should remain 0 to enable transparent painting.
  if (!use_transparent_painting) {
    browser_background_color_ = background_color_;
  }

//   const std::string& cdm_path = command_line_->GetSwitchValue(switches::kWidevineCdmPath);
//   if (!cdm_path.empty()) {
//     // Register the Widevine CDM at the specified path. See comments in
//     // cef_web_plugin.h for details. It's safe to call this method before
//     // CefInitialize(), and calling it before CefInitialize() is required on
//     // Linux.
//     CefRegisterWidevineCdm(cdm_path, NULL);
//   }
}

MainContextImpl::~MainContextImpl() {
  // The context must either not have been initialized, or it must have also
  // been shut down.
  DCHECK(!initialized_ || shutdown_);
}

std::string MainContextImpl::GetConsoleLogPath() {
  return GetAppWorkingDirectory() + "\\logs\\cef_console.log";
}

std::string MainContextImpl::GetMainURL() {
  return main_url_;
}

void MainContextImpl::SetBackgroundColor(cef_color_t color) {
    background_color_ = color;
}

cef_color_t MainContextImpl::GetBackgroundColor() {
  return background_color_;
}

bool MainContextImpl::UseViews() {
  return use_views_;
}

bool MainContextImpl::UseWindowlessRendering() {
  return use_windowless_rendering_;
}

void MainContextImpl::PopulateSettings(CefSettings* settings) 
{
  settings->multi_threaded_message_loop = command_line_->HasSwitch(switches::kMultiThreadedMessageLoop);
  
  if (!settings->multi_threaded_message_loop) {
    settings->external_message_pump =
        command_line_->HasSwitch(switches::kExternalMessagePump);
  }

  CefString(&settings->cache_path) = command_line_->GetSwitchValue(switches::kCachePath);

  if (use_windowless_rendering_)
	  settings->windowless_rendering_enabled = true;

  if (browser_background_color_ != 0)
    settings->background_color = browser_background_color_;
}

void MainContextImpl::PopulateBrowserSettings(CefBrowserSettings* settings) {
  if (command_line_->HasSwitch(switches::kOffScreenFrameRate)) {
    settings->windowless_frame_rate =
        atoi(command_line_->GetSwitchValue(switches::kOffScreenFrameRate)
                 .ToString()
                 .c_str());
  }

  if (browser_background_color_ != 0)
    settings->background_color = browser_background_color_;
}

void MainContextImpl::PopulateOsrSettings(OsrRenderer::Settings* settings) {
  settings->show_update_rect =
      command_line_->HasSwitch(switches::kShowUpdateRect);

  if (browser_background_color_ != 0)
    settings->background_color = browser_background_color_;
}

BrowserManager* MainContextImpl::GetBrowserWindowManager() {
  DCHECK(InValidState());
  return browser_window_manager_.get();
}

bool MainContextImpl::is_dev()
{
    return is_dev_;
}

bool MainContextImpl::Initialize(bool is_dev) 
{
  DCHECK(thread_checker_.CalledOnValidThread());
  DCHECK(!initialized_);
  DCHECK(!shutdown_);
  if (!command_line_)
	  return false;

  is_dev_ = is_dev;

  CefMainArgs main_args;
  void* sandbox_info = NULL;

  // Execute the secondary process, if any.
  int exit_code = CefExecuteProcess(main_args, cef_app_, sandbox_info);
  if (exit_code >= 0)
	  return exit_code;

  CefSettings settings;
  // Populate the settings based on command line arguments.
  PopulateSettings(&settings);
  CefString(&settings.log_file) = ("logs\\cef3client.log");
  if (is_dev_)
      settings.log_severity = LOGSEVERITY_VERBOSE;
  else
      settings.log_severity = LOGSEVERITY_DISABLE;
  settings.no_sandbox = true;
  settings.persist_session_cookies = TRUE;
  settings.remote_debugging_port = 5050;
  settings.uncaught_exception_stack_size = 100;
  settings.windowless_rendering_enabled = true;
  CefString(&settings.locale).FromASCII("zh-CN");
  CefString(&settings.accept_language_list).FromASCII("zh-CN");

  const char* path = "Caches\\";
  //store cache on hdd
  CefString(&settings.cache_path).FromASCII(path);

  if (settings.multi_threaded_message_loop)
	  messsage_loop_.reset(new MainMessageLoopMultithreadedWin);
  else if (settings.external_message_pump)
	  messsage_loop_ = MainMessageLoopExternalPump::Create();
  else
	  messsage_loop_.reset(new MainMessageLoopStd);

  if (!CefInitialize(main_args, settings, cef_app_, sandbox_info))
    return false;

  browser_window_manager_.reset(new BrowserManager(terminate_when_all_windows_closed_));
  initialized_ = true;

  return true;
}

void MainContextImpl::run()
{
	messsage_loop_->Run();
}

void MainContextImpl::quit()
{
	messsage_loop_->Quit();
}

void MainContextImpl::Shutdown() {
  DCHECK(thread_checker_.CalledOnValidThread());
  DCHECK(initialized_);
  DCHECK(!shutdown_);
  //messsage_loop_->Quit();
  // wait all window closed
  browser_window_manager_.reset();
  CefShutdown();
  shutdown_ = true;
  messsage_loop_.reset();
}

}  // namespace client
