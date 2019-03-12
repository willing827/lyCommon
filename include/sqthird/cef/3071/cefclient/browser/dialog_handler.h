// Copyright (c) 2012 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFCLIENT_BROWSER_DIALOG_TEST_H_
#define CEF_TESTS_CEFCLIENT_BROWSER_DIALOG_TEST_H_
#pragma once
#include "util_win.h"

namespace cef3 {
namespace dialog {

// Create message handlers. Called from test_runner.cc.
void CreateMessageHandlers(MessageHandlerSet& handlers);

}  // namespace dialog_test
}  // namespace client

#endif  // CEF_TESTS_CEFCLIENT_BROWSER_DIALOG_TEST_H_
