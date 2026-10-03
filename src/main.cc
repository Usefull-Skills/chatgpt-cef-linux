#if defined(_WIN32)
#include <windows.h>
#include "include/cef_sandbox_win.h"
#else
#include <X11/Xlib.h>
#endif

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

void SetEnv(const char* name, const std::string& value) {
#if defined(_WIN32)
  _putenv_s(name, value.c_str());
#else
  setenv(name, value.c_str(), 1);
#endif
}

void ConfigureProfileOverride(CefRefPtr<CefCommandLine> command_line) {
  if (!command_line || !command_line->HasSwitch("cgwa-profile-dir")) return;
  const std::string raw = command_line->GetSwitchValue("cgwa-profile-dir").ToString();
  if (raw.empty()) return;
  std::filesystem::path path(raw);
  if (!path.is_absolute()) path = std::filesystem::absolute(path);
  SetEnv("CGWA_PROFILE_ROOT", path.lexically_normal().string());
}

std::string ConfigRoot() {
  if (const char* override_root = std::getenv("CGWA_PROFILE_ROOT");
      override_root && *override_root)
    return override_root;
#if defined(_WIN32)
  if (const char* local = std::getenv("LOCALAPPDATA"); local && *local)
    return (std::filesystem::path(local) / "chatgpt-cef-v2").string();
  if (const char* profile = std::getenv("USERPROFILE"); profile && *profile)
    return (std::filesystem::path(profile) / "AppData" / "Local" / "chatgpt-cef-v2").string();
  return (std::filesystem::temp_directory_path() / "chatgpt-cef-v2").string();
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
int XIOErrorHandlerImpl(Display* display) { return 0; }
#endif

int RunMain(const CefMainArgs& main_args, void* sandbox_info) {
  auto command_line = CefCommandLine::CreateCommandLine();
#if defined(_WIN32)
  command_line->InitFromString(::GetCommandLineW());
#else
  command_line->InitFromArgv(main_args.argc, main_args.argv);
#endif
  ConfigureProfileOverride(command_line);

  CefRefPtr<CefApp> app;
  if (!command_line->HasSwitch("type"))
    app = new BrowserApp();

  const int exit_code = CefExecuteProcess(main_args, app, sandbox_info);
  if (exit_code >= 0)
    return exit_code;

#if !defined(_WIN32)
  XSetErrorHandler(XErrorHandlerImpl);
  XSetIOErrorHandler(XIOErrorHandlerImpl);
#endif

  const std::string root = ConfigRoot();
  const std::string cache = (std::filesystem::path(root) / "cache").string();
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
  if (!sandbox_info)
    settings.no_sandbox = 1;

  if (!CefInitialize(main_args, settings, app, sandbox_info))
    return CefGetExitCode();

  CefRunMessageLoop();
  CefShutdown();
  return 0;
}

}  // namespace

#if defined(_WIN32)
#if defined(CEF_USE_BOOTSTRAP)
CEF_BOOTSTRAP_EXPORT int RunWinMain(HINSTANCE hInstance,
                                    LPWSTR lpCmdLine,
                                    int nCmdShow,
                                    void* sandbox_info,
                                    cef_version_info_t* /*version_info*/) {
  return RunMain(CefMainArgs(hInstance), sandbox_info);
}
#else
int APIENTRY wWinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPWSTR lpCmdLine,
                      int nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);
  UNREFERENCED_PARAMETER(nCmdShow);
  void* sandbox_info = nullptr;
#if defined(CEF_USE_SANDBOX)
  CefScopedSandboxInfo scoped_sandbox;
  sandbox_info = scoped_sandbox.sandbox_info();
#endif
  return RunMain(CefMainArgs(hInstance), sandbox_info);
}
#endif
#else
NO_STACK_PROTECTOR int main(int argc, char* argv[]) {
  return RunMain(CefMainArgs(argc, argv), nullptr);
}
#endif
