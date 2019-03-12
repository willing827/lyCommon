// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFCLIENT_BROWSER_BROWSER_WINDOW_H_
#define CEF_TESTS_CEFCLIENT_BROWSER_BROWSER_WINDOW_H_
#pragma once

#include <functional>
#include "include/base/cef_scoped_ptr.h"
#include "include/cef_browser.h"
#include "client_handler.h"
#include "client_types.h"
#include "main_message_loop.h"

namespace cef3 {

typedef std::function<void(CefRefPtr<CefProcessMessage>, int browser_id)> CefMSgCallBack;

// Represents a native child window hosting a single browser instance. The
// methods of this class must be called on the main thread unless otherwise
// indicated.
class BrowserWindow : public base::RefCountedThreadSafe<BrowserWindow, DeleteOnMainThread>,
					  public ClientHandler::Delegate {
 public:
  // This interface is implemented by the owner of the BrowserWindow. The
  // methods of this class will be called on the main thread.
  class Delegate {
   public:
    // Called when the browser has been created.
    //virtual void OnBrowserCreated(CefRefPtr<CefBrowser> browser) = 0;

    // Called when the BrowserWindow has been destroyed.
    virtual void OnBrowserWindowDestroyed(int browser_id) = 0;

    // Set the loading state.
    virtual void OnSetLoadingState(bool isLoading,
                                   bool canGoBack,
                                   bool canGoForward) = 0;

    // Set the draggable regions.
    virtual void OnSetDraggableRegions(
        const std::vector<CefDraggableRegion>& regions) = 0;

   protected:
    virtual ~Delegate() {}
  };

  // Create a new browser and native window.
  virtual void CreateBrowser(ClientWindowHandle parent_handle,
                             const CefRect& rect,
                             const CefBrowserSettings& settings,
                             CefRefPtr<CefRequestContext> request_context) = 0;

  // Retrieve the configuration that will be used when creating a popup window.
  // The popup browser will initially be parented to |temp_handle| which should
  // be a pre-existing hidden window. The native window will be created later
  // after the browser has been created. This method may be called on any
  // thread.
  virtual void GetPopupConfig(CefWindowHandle temp_handle,
                              CefWindowInfo& windowInfo,
                              CefRefPtr<CefClient>& client,
                              CefBrowserSettings& settings) = 0;

  // Show the popup window with correct parent and bounds in parent coordinates.
  virtual void ShowPopup(ClientWindowHandle parent_handle,
                         int x,
                         int y,
                         size_t width,
                         size_t height) = 0;

  // Show the window.
  virtual void Show() = 0;

  // Hide the window.
  virtual void Hide() = 0;

  // Set the window bounds in parent coordinates.
  virtual void SetBounds(int x, int y, size_t width, size_t height) = 0;

  // Set focus to the window.
  virtual void SetFocus(bool focus) = 0;

  // Set the device scale factor. Only used in combination with off-screen
  // rendering.
  virtual void SetDeviceScaleFactor(float device_scale_factor);

  // Returns the device scale factor. Only used in combination with off-screen
  // rendering.
  virtual float GetDeviceScaleFactor() const;

  // Returns the window handle.
  virtual ClientWindowHandle GetWindowHandle() const = 0;

  // Returns the browser owned by the window.
  CefRefPtr<CefBrowser> GetBrowser() const;

  void Close();

  // Returns true if the browser is closing.
  bool IsClosing() const;

  void RegisterMessageHandler(CefMSgCallBack handler, bool is_osr = false) {
	  cef_js_msg_cb_ = handler; 
	  is_osr_ = is_osr;
  }

  bool IsEditing() const {
	  return client_handler_->focus_on_editable_field_;
  };

 protected:
  // Allow deletion via scoped_refptr only.
  friend struct DeleteOnMainThread;
  friend class base::RefCountedThreadSafe<BrowserWindow, DeleteOnMainThread>;

  // Constructor may be called on any thread.
  // |delegate| must outlive this object.
  explicit BrowserWindow(Delegate* delegate);

  // ClientHandler::Delegate methods.
  void OnBrowserCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
  void OnBrowserClosing(CefRefPtr<CefBrowser> browser) OVERRIDE;
  void OnBrowserClosed(CefRefPtr<CefBrowser> browser) OVERRIDE;
  void OnSetLoadingState(bool isLoading,
                         bool canGoBack,
                         bool canGoForward) OVERRIDE;
  void OnSetDraggableRegions(
      const std::vector<CefDraggableRegion>& regions) OVERRIDE;

  void OnSetTitle(const std::string& title) OVERRIDE;
  void OnPopup(CefRefPtr<CefBrowser> browser,
	  CefRefPtr<CefFrame> frame, const std::string& target_url) OVERRIDE;

  bool OnProcessMessageReceived(CefRefPtr<CefProcessMessage> message) OVERRIDE;
  void OnLoadOver(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
				  int httpCode,
				  const CefString& errorText,
				  const CefString& failedUrl) OVERRIDE;
  void DisPatchProcessMsg(CefRefPtr<CefProcessMessage> message);
  void OnSetFavicon(CefRefPtr<CefImage> image);

  Delegate* delegate_;
  CefRefPtr<CefBrowser> browser_;
  CefRefPtr<ClientHandler> client_handler_;
  CefMSgCallBack cef_js_msg_cb_;

  bool is_closing_;
  bool is_osr_;

};

}  // namespace client

#endif  // CEF_TESTS_CEFCLIENT_BROWSER_BROWSER_WINDOW_H_
