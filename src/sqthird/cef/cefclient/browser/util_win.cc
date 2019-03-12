// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cefclient/browser/util_win.h"

#include "include/base/cef_logging.h"
#include "include/cef_parser.h"

namespace cef3 {

void SetUserDataPtr(HWND hWnd, void* ptr) {
  SetLastError(ERROR_SUCCESS);
  LONG_PTR result =
      ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(ptr));
  CHECK(result != 0 || GetLastError() == ERROR_SUCCESS);
}

WNDPROC SetWndProcPtr(HWND hWnd, WNDPROC wndProc) {
  WNDPROC old =
      reinterpret_cast<WNDPROC>(::GetWindowLongPtr(hWnd, GWLP_WNDPROC));
  CHECK(old != NULL);
  LONG_PTR result = ::SetWindowLongPtr(hWnd, GWLP_WNDPROC,
                                       reinterpret_cast<LONG_PTR>(wndProc));
  CHECK(result != 0 || GetLastError() == ERROR_SUCCESS);
  return old;
}

std::wstring GetResourceString(UINT id) {
#define MAX_LOADSTRING 100
  TCHAR buff[MAX_LOADSTRING] = {0};
  LoadString(::GetModuleHandle(NULL), id, buff, MAX_LOADSTRING);
  return buff;
}

int GetCefMouseModifiers(WPARAM wparam) {
  int modifiers = 0;
  if (wparam & MK_CONTROL)
    modifiers |= EVENTFLAG_CONTROL_DOWN;
  if (wparam & MK_SHIFT)
    modifiers |= EVENTFLAG_SHIFT_DOWN;
  if (IsKeyDown(VK_MENU))
    modifiers |= EVENTFLAG_ALT_DOWN;
  if (wparam & MK_LBUTTON)
    modifiers |= EVENTFLAG_LEFT_MOUSE_BUTTON;
  if (wparam & MK_MBUTTON)
    modifiers |= EVENTFLAG_MIDDLE_MOUSE_BUTTON;
  if (wparam & MK_RBUTTON)
    modifiers |= EVENTFLAG_RIGHT_MOUSE_BUTTON;

  // Low bit set from GetKeyState indicates "toggled".
  if (::GetKeyState(VK_NUMLOCK) & 1)
    modifiers |= EVENTFLAG_NUM_LOCK_ON;
  if (::GetKeyState(VK_CAPITAL) & 1)
    modifiers |= EVENTFLAG_CAPS_LOCK_ON;
  return modifiers;
}

int GetCefKeyboardModifiers(WPARAM wparam, LPARAM lparam) {
  int modifiers = 0;
  if (IsKeyDown(VK_SHIFT))
    modifiers |= EVENTFLAG_SHIFT_DOWN;
  if (IsKeyDown(VK_CONTROL))
    modifiers |= EVENTFLAG_CONTROL_DOWN;
  if (IsKeyDown(VK_MENU))
    modifiers |= EVENTFLAG_ALT_DOWN;

  // Low bit set from GetKeyState indicates "toggled".
  if (::GetKeyState(VK_NUMLOCK) & 1)
    modifiers |= EVENTFLAG_NUM_LOCK_ON;
  if (::GetKeyState(VK_CAPITAL) & 1)
    modifiers |= EVENTFLAG_CAPS_LOCK_ON;

  switch (wparam) {
    case VK_RETURN:
      if ((lparam >> 16) & KF_EXTENDED)
        modifiers |= EVENTFLAG_IS_KEY_PAD;
      break;
    case VK_INSERT:
    case VK_DELETE:
    case VK_HOME:
    case VK_END:
    case VK_PRIOR:
    case VK_NEXT:
    case VK_UP:
    case VK_DOWN:
    case VK_LEFT:
    case VK_RIGHT:
      if (!((lparam >> 16) & KF_EXTENDED))
        modifiers |= EVENTFLAG_IS_KEY_PAD;
      break;
    case VK_NUMLOCK:
    case VK_NUMPAD0:
    case VK_NUMPAD1:
    case VK_NUMPAD2:
    case VK_NUMPAD3:
    case VK_NUMPAD4:
    case VK_NUMPAD5:
    case VK_NUMPAD6:
    case VK_NUMPAD7:
    case VK_NUMPAD8:
    case VK_NUMPAD9:
    case VK_DIVIDE:
    case VK_MULTIPLY:
    case VK_SUBTRACT:
    case VK_ADD:
    case VK_DECIMAL:
    case VK_CLEAR:
      modifiers |= EVENTFLAG_IS_KEY_PAD;
      break;
    case VK_SHIFT:
      if (IsKeyDown(VK_LSHIFT))
        modifiers |= EVENTFLAG_IS_LEFT;
      else if (IsKeyDown(VK_RSHIFT))
        modifiers |= EVENTFLAG_IS_RIGHT;
      break;
    case VK_CONTROL:
      if (IsKeyDown(VK_LCONTROL))
        modifiers |= EVENTFLAG_IS_LEFT;
      else if (IsKeyDown(VK_RCONTROL))
        modifiers |= EVENTFLAG_IS_RIGHT;
      break;
    case VK_MENU:
      if (IsKeyDown(VK_LMENU))
        modifiers |= EVENTFLAG_IS_LEFT;
      else if (IsKeyDown(VK_RMENU))
        modifiers |= EVENTFLAG_IS_RIGHT;
      break;
    case VK_LWIN:
      modifiers |= EVENTFLAG_IS_LEFT;
      break;
    case VK_RWIN:
      modifiers |= EVENTFLAG_IS_RIGHT;
      break;
  }
  return modifiers;
}

bool IsKeyDown(WPARAM wparam) {
  return (GetKeyState(wparam) & 0x8000) != 0;
}

float GetDeviceScaleFactor() {
  static float scale_factor = 1.0;
  static bool initialized = false;

  if (!initialized) {
    // This value is safe to cache for the life time of the app since the user
    // must logout to change the DPI setting. This value also applies to all
    // screens.
    HDC screen_dc = ::GetDC(NULL);
    int dpi_x = GetDeviceCaps(screen_dc, LOGPIXELSX);
    scale_factor = static_cast<float>(dpi_x) / 96.0f;
    ::ReleaseDC(NULL, screen_dc);
    initialized = true;
  }

  return scale_factor;
}

std::string GetDataURI(const std::string& data, const std::string& mime_type) {
	return "data:" + mime_type + ";base64," +
		CefURIEncode(CefBase64Encode(data.data(), data.size()), false)
		.ToString();
}

std::string GetErrorString(cef_errorcode_t code) {
	// Case condition that returns |code| as a string.
#define CASE(code) \
  case code:       \
    return #code

	switch (code) {
		CASE(ERR_NONE);
		CASE(ERR_FAILED);
		CASE(ERR_ABORTED);
		CASE(ERR_INVALID_ARGUMENT);
		CASE(ERR_INVALID_HANDLE);
		CASE(ERR_FILE_NOT_FOUND);
		CASE(ERR_TIMED_OUT);
		CASE(ERR_FILE_TOO_BIG);
		CASE(ERR_UNEXPECTED);
		CASE(ERR_ACCESS_DENIED);
		CASE(ERR_NOT_IMPLEMENTED);
		CASE(ERR_CONNECTION_CLOSED);
		CASE(ERR_CONNECTION_RESET);
		CASE(ERR_CONNECTION_REFUSED);
		CASE(ERR_CONNECTION_ABORTED);
		CASE(ERR_CONNECTION_FAILED);
		CASE(ERR_NAME_NOT_RESOLVED);
		CASE(ERR_INTERNET_DISCONNECTED);
		CASE(ERR_SSL_PROTOCOL_ERROR);
		CASE(ERR_ADDRESS_INVALID);
		CASE(ERR_ADDRESS_UNREACHABLE);
		CASE(ERR_SSL_CLIENT_AUTH_CERT_NEEDED);
		CASE(ERR_TUNNEL_CONNECTION_FAILED);
		CASE(ERR_NO_SSL_VERSIONS_ENABLED);
		CASE(ERR_SSL_VERSION_OR_CIPHER_MISMATCH);
		CASE(ERR_SSL_RENEGOTIATION_REQUESTED);
		CASE(ERR_CERT_COMMON_NAME_INVALID);
		CASE(ERR_CERT_DATE_INVALID);
		CASE(ERR_CERT_AUTHORITY_INVALID);
		CASE(ERR_CERT_CONTAINS_ERRORS);
		CASE(ERR_CERT_NO_REVOCATION_MECHANISM);
		CASE(ERR_CERT_UNABLE_TO_CHECK_REVOCATION);
		CASE(ERR_CERT_REVOKED);
		CASE(ERR_CERT_INVALID);
		CASE(ERR_CERT_END);
		CASE(ERR_INVALID_URL);
		CASE(ERR_DISALLOWED_URL_SCHEME);
		CASE(ERR_UNKNOWN_URL_SCHEME);
		CASE(ERR_TOO_MANY_REDIRECTS);
		CASE(ERR_UNSAFE_REDIRECT);
		CASE(ERR_UNSAFE_PORT);
		CASE(ERR_INVALID_RESPONSE);
		CASE(ERR_INVALID_CHUNKED_ENCODING);
		CASE(ERR_METHOD_NOT_SUPPORTED);
		CASE(ERR_UNEXPECTED_PROXY_AUTH);
		CASE(ERR_EMPTY_RESPONSE);
		CASE(ERR_RESPONSE_HEADERS_TOO_BIG);
		CASE(ERR_CACHE_MISS);
		CASE(ERR_INSECURE_RESPONSE);
	default:
		return "UNKNOWN";
	}
}

// Replace all instances of |from| with |to| in |str|.
std::string StringReplace(const std::string& str,
	const std::string& from,
	const std::string& to) {
	std::string result = str;
	std::string::size_type pos = 0;
	std::string::size_type from_len = from.length();
	std::string::size_type to_len = to.length();
	do {
		pos = result.find(from, pos);
		if (pos != std::string::npos) {
			result.replace(pos, from_len, to);
			pos += to_len;
		}
	} while (pos != std::string::npos);
	return result;
}

void Alert(CefRefPtr<CefBrowser> browser, const std::string& message) {
	// Escape special characters in the message.
	std::string msg = StringReplace(message, "\\", "\\\\");
	msg = StringReplace(msg, "'", "\\'");

	// Execute a JavaScript alert().
	CefRefPtr<CefFrame> frame = browser->GetMainFrame();
	frame->ExecuteJavaScript("alert('" + msg + "');", frame->GetURL(), 0);
}

}  // namespace client
