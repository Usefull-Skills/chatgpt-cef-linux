#include <X11/Xlib.h>

#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <system_error>

#include "app.h"
#include "include/base/cef_compiler_specific.h"
#include "include/base/cef_logging.h"
#include "include/cef_app.h"
#include "include/cef_command_line.h"

namespace {

bool IsSubprocess(int argc, char* argv[]) {
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i] ? argv[i] : "";
    if (arg.rfind("--type=", 0) == 0)
      return true;
  }
  return false;
}

void ConfigureProfileOverride(int argc, char* argv[]) {
  constexpr char kPrefix[] = "--cgwa-profile-dir=";
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i] ? argv[i] : "";
    if (arg.rfind(kPrefix, 0) != 0) continue;
    const std::string raw = arg.substr(sizeof(kPrefix) - 1);
    if (raw.empty()) continue;
    std::filesystem::path path(raw);
    if (!path.is_absolute()) path = std::filesystem::absolute(path);
    path = path.lexically_normal();
    setenv("CGWA_PROFILE_ROOT", path.c_str(), 1);
    break;
  }
}

std::string ConfigRoot() {
  if (const char* override_root = std::getenv("CGWA_PROFILE_ROOT");
      override_root && *override_root)
    return override_root;
  const char* home = std::getenv("HOME");
  std::string base = home ? home : "/tmp";
  return base + "/.config/chatgpt-cef-v2";
}

int XErrorHandlerImpl(Display* display, XErrorEvent* event) {
  LOG(WARNING) << "X11 error code=" << static_cast<int>(event->error_code)
               << " request=" << static_cast<int>(event->request_code)
               << " minor=" << static_cast<int>(event->minor_code);
  return 0;
}

int XIOErrorHandlerImpl(Display* display) {
  return 0;
}

}  // namespace

NO_STACK_PROTECTOR int main(int argc, char* argv[]) {
  ConfigureProfileOverride(argc, argv);
  CefMainArgs main_args(argc, argv);
  CefRefPtr<CefApp> app;
  if (!IsSubprocess(argc, argv))
    app = new BrowserApp();

  const int exit_code = CefExecuteProcess(main_args, app, nullptr);
  if (exit_code >= 0)
    return exit_code;

  XSetErrorHandler(XErrorHandlerImpl);
  XSetIOErrorHandler(XIOErrorHandlerImpl);

  const std::string root = ConfigRoot();
  const std::string cache = root + "/cache";
  std::error_code ec;
  std::filesystem::create_directories(cache, ec);
  if (ec) {
    LOG(ERROR) << "Failed creating CEF profile: " << ec.message();
    return 2;
  }

  CefSettings settings;
  CefString(&settings.root_cache_path) = root;
  CefString(&settings.cache_path) = cache;
  settings.persist_session_cookies = 1;
  settings.log_severity = LOGSEVERITY_WARNING;
  settings.background_color = CefColorSetARGB(255, 0, 0, 0);

  // False can indicate single-instance early exit after relaunch forwarding.
  if (!CefInitialize(main_args, settings, app, nullptr))
    return 0;

  CefRunMessageLoop();
  CefShutdown();
  return 0;
}
