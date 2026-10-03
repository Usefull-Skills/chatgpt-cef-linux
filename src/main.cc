#if defined(_WIN32)
#include <windows.h>
#include <cstdlib>
#else
#include <X11/Xlib.h>
#include <csignal>
#include <cstdlib>
#endif

#include <filesystem>
#include <string>
#include <system_error>

#include "app.h"
#include "include/base/cef_compiler_specific.h"
#include "include/base/cef_logging.h"
#include "include/cef_app.h"
#include "include/cef_command_line.h"
#if defined(_WIN32) && defined(CEF_USE_SANDBOX)
#include "include/cef_sandbox_win.h"
#endif

namespace {

bool IsSubprocess(CefRefPtr<CefCommandLine> command_line) {
  return command_line && command_line->HasSwitch("type");
}

void ConfigureProfileOverride(CefRefPtr<CefCommandLine> command_line) {
  if (!command_line || !command_line->HasSwitch("cgwa-profile-dir"))
    return;
  const std::string raw =
      command_line->GetSwitchValue("cgwa-profile-dir").ToString();
  if (raw.empty()) return;
  std::filesystem::path path = std::filesystem::u8path(raw);
  if (!path.is_absolute()) path = std::filesystem::absolute(path);
  path = path.lexically_normal();
#if defined(_WIN32)
  const std::string normalized = path.u8string();
  _putenv_s("CGWA_PROFILE_ROOT", normalized.c_str());
#else
  setenv("CGWA_PROFILE_ROOT", path.c_str(), 1);
#endif
}

std::string ConfigRoot() {
  if (const char* override_root = std::getenv("CGWA_PROFILE_ROOT");
      override_root && *override_root)
    return override_root;
#if defined(_WIN32)
  const char* local = std::getenv("LOCALAPPDATA");
  if (!local || !*local) local = std::getenv("TEMP");
  std::filesystem::path base =
      local && *local ? std::filesystem::u8path(local)
                      : std::filesystem::temp_directory_path();
  return (base / "chatgpt-cef-v2").u8string();
#else
  const char* home = std::getenv("HOME");
  std::string base = home ? home : "/tmp";
  return base + "/.config/chatgpt-cef-v2";
#endif
}

#if !defined(_WIN32)
int XErrorHandlerImpl(Display* display, XErrorEvent* event) {
  LOG(WARNING) << "X11 error code=" << static_cast<int>(event->error_code)
               << " request=" << static_cast<int>(event->request_code)
               << " minor=" << static_cast<int>(event->minor_code);
  return 0;
}

int XIOErrorHandlerImpl(Display* display) {
  return 0;
}
#endif

int RunBrowser(const CefMainArgs& main_args,
               CefRefPtr<CefApp> app,
               void* sandbox_info) {
  const int exit_code = CefExecuteProcess(main_args, app, sandbox_info);
  if (exit_code >= 0)
    return exit_code;

#if !defined(_WIN32)
  XSetErrorHandler(XErrorHandlerImpl);
  XSetIOErrorHandler(XIOErrorHandlerImpl);
#endif

  const std::filesystem::path root = std::filesystem::u8path(ConfigRoot());
  const std::filesystem::path cache = root / "cache";
  std::error_code ec;
  std::filesystem::create_directories(cache, ec);
  if (ec) {
    LOG(ERROR) << "Failed creating CEF profile: " << ec.message();
    return 2;
  }

  CefSettings settings;
  CefString(&settings.root_cache_path) = root.u8string();
  CefString(&settings.cache_path) = cache.u8string();
  settings.persist_session_cookies = 1;
  settings.log_severity = LOGSEVERITY_WARNING;
  settings.background_color = CefColorSetARGB(255, 0, 0, 0);

  // False can indicate single-instance early exit after relaunch forwarding.
  if (!CefInitialize(main_args, settings, app, sandbox_info))
    return 0;

  CefRunMessageLoop();
  CefShutdown();
  return 0;
}

}  // namespace

#if defined(_WIN32)
NO_STACK_PROTECTOR int APIENTRY wWinMain(HINSTANCE hInstance,
                                         HINSTANCE hPrevInstance,
                                         LPTSTR lpCmdLine,
                                         int nCmdShow) {
  CefMainArgs main_args(hInstance);
  auto command_line = CefCommandLine::CreateCommandLine();
  command_line->InitFromString(::GetCommandLineW());
  ConfigureProfileOverride(command_line);

  CefRefPtr<CefApp> app;
  if (!IsSubprocess(command_line))
    app = new BrowserApp();

  void* sandbox_info = nullptr;
#if defined(CEF_USE_SANDBOX)
  CefScopedSandboxInfo scoped_sandbox;
  sandbox_info = scoped_sandbox.sandbox_info();
#endif
  return RunBrowser(main_args, app, sandbox_info);
}
#else
NO_STACK_PROTECTOR int main(int argc, char* argv[]) {
  auto command_line = CefCommandLine::CreateCommandLine();
  command_line->InitFromArgv(argc, argv);
  ConfigureProfileOverride(command_line);

  CefMainArgs main_args(argc, argv);
  CefRefPtr<CefApp> app;
  if (!IsSubprocess(command_line))
    app = new BrowserApp();

  return RunBrowser(main_args, app, nullptr);
}
#endif
