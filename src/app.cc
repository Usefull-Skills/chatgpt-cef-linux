#include "app.h"

#include <memory>

#include "controller.h"
#include "include/base/cef_bind.h"
#include "include/cef_task.h"
#include "include/wrapper/cef_helpers.h"

BrowserApp::BrowserApp() : controller_(std::make_unique<AppController>()) {}
BrowserApp::~BrowserApp() = default;

void BrowserApp::OnBeforeCommandLineProcessing(
    const CefString& process_type,
    CefRefPtr<CefCommandLine> command_line) {
  if (process_type.empty()) {
    // Stay on X11 to match the user's current desktop session and the tested
    // Brave baseline. Do not disable the Chromium sandbox.
    command_line->AppendSwitchWithValue("ozone-platform", "x11");
    command_line->AppendSwitch("enable-gpu-rasterization");
    command_line->AppendSwitch("enable-zero-copy");
  }
}

void BrowserApp::OnContextInitialized() {
  CEF_REQUIRE_UI_THREAD();
  controller_->CreateMainWindow();
}

bool BrowserApp::OnAlreadyRunningAppRelaunch(
    CefRefPtr<CefCommandLine> command_line,
    const CefString& current_directory) {
  CEF_REQUIRE_UI_THREAD();
  if (controller_ && command_line &&
      command_line->HasSwitch("shutdown-self-test")) {
    controller_->RequestCloseWindow();
    return true;
  }
  const bool silent = command_line &&
      (command_line->HasSwitch("background-test") ||
       command_line->HasSwitch("self-test"));
  if (controller_ && !silent)
    controller_->ActivateMainWindow();
  return true;
}
