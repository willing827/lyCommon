// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_SHARED_BROWSER_UTIL_WIN_H_
#define CEF_TESTS_SHARED_BROWSER_UTIL_WIN_H_
#pragma once

#include <windows.h>
#include <string>
#include <set>

#include "include/wrapper/cef_message_router.h"
#include "include/internal/cef_types_wrappers.h"
#include "include/cef_browser.h"

namespace cef3 {

typedef std::set<CefMessageRouterBrowserSide::Handler*> MessageHandlerSet;

// Set the window's user data pointer.
void SetUserDataPtr(HWND hWnd, void* ptr);

// Return the window's user data pointer.
template <typename T>
T GetUserDataPtr(HWND hWnd) {
  return reinterpret_cast<T>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
}

// Set the window's window procedure pointer and return the old value.
WNDPROC SetWndProcPtr(HWND hWnd, WNDPROC wndProc);

// Return the resource string with the specified id.
std::wstring GetResourceString(UINT id);

int GetCefMouseModifiers(WPARAM wparam);
int GetCefKeyboardModifiers(WPARAM wparam, LPARAM lparam);
bool IsKeyDown(WPARAM wparam);

// Returns the device scale factor. For example, 200% display scaling will
// return 2.0.
float GetDeviceScaleFactor();

// Returns a data: URI with the specified contents.
std::string GetDataURI(const std::string& data, const std::string& mime_type);

// Returns the string representation of the specified error code.
std::string GetErrorString(cef_errorcode_t code);

// Show a JS alert message.
void Alert(CefRefPtr<CefBrowser> browser, const std::string& message);

}  // namespace client

#endif  // CEF_TESTS_SHARED_BROWSER_UTIL_WIN_H_
