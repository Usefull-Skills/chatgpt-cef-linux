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

bool ConfigureProfileOverride(CefRefPtr<CefCommandLine> command_line) {
  if (!command_line) return false;
  const bool qa_mode =
      command_line->HasSwitch("background-test") ||
      command_line->HasSwitch("self-test") ||
      command_line->HasSwitch("companion-self-test") ||
      command_line->HasSwitch("shutdown-self-test");
  // A test that omits an explicit profile must never fall back to live
  // ChatGPT profile data. Production without QA flags remains unchanged.
  if (!command_line->HasSwitch("cgwa-profile-dir")) return !qa_mode;
  const CefString raw = command_line->GetSwitchValue("cgwa-profile-dir");
  // CEF parses switch values as --cgwa-profile-dir=PATH, not a separate argv.
  if (raw.empty()) return false;
#if defined(_WIN32)
  std::filesystem::path path(raw.ToWString());
#else
  std::filesystem::path path = std::filesystem::u8path(raw.ToString());
#endif
  if (!path.is_absolute()) return false;
  path = path.lexically_normal();
  if (qa_mode) {
    // Reject exact or descendant aliases of the live profile, including
    // reparse-point/symlink aliases. Invalid filesystem queries fail closed.
#if defined(_WIN32)
    const wchar_t* local = _wgetenv(L"LOCALAPPDATA");
    if (!local || !*local) local = _wgetenv(L"TEMP");
    const std::filesystem::path default_root =
        (local && *local ? std::filesystem::path(local)
                         : std::filesystem::temp_directory_path()) /
        L"chatgpt-cef-v2";
#else
    const char* home = std::getenv("HOME");
    const std::filesystem::path default_root =
        std::filesystem::u8path(home ? home : "/tmp") /
        ".config/chatgpt-cef-v2";
#endif
    std::error_code qa_error, prod_error;
    const auto canonical_qa = std::filesystem::weakly_canonical(path, qa_error);
    const auto canonical_prod =
        std::filesystem::weakly_canonical(default_root, prod_error);
    if (qa_error || prod_error) return false;
#if defined(_WIN32)
    const auto qa = canonical_qa.wstring();
    const auto prod = canonical_prod.wstring();
    if (_wcsicmp(qa.c_str(), prod.c_str()) == 0 ||
        (qa.size() > prod.size() && qa[prod.size()] == L'\\' &&
         _wcsnicmp(qa.c_str(), prod.c_str(), prod.size()) == 0))
      return false;
#else
    const auto qa = canonical_qa.string();
    const auto prod = canonical_prod.string();
    if (qa == prod ||
        (qa.size() > prod.size() && qa[prod.size()] == '/' &&
         qa.compare(0, prod.size(), prod) == 0))
      return false;
#endif
  }
#if defined(_WIN32)
  if (_wputenv_s(L"CGWA_PROFILE_ROOT", path.wstring().c_str()) != 0)
    return false;
#else
  if (setenv("CGWA_PROFILE_ROOT", path.c_str(), 1) != 0)
    return false;
#endif
  return true;
}

std::filesystem::path ConfigRoot() {
#if defined(_WIN32)
  if (const wchar_t* override_root = _wgetenv(L"CGWA_PROFILE_ROOT");
      override_root && *override_root)
    return std::filesystem::path(override_root);
  const wchar_t* local = _wgetenv(L"LOCALAPPDATA");
  if (!local || !*local) local = _wgetenv(L"TEMP");
  std::filesystem::path base =
      local && *local ? std::filesystem::path(local)
                      : std::filesystem::temp_directory_path();
  return base / L"chatgpt-cef-v2";
#else
  if (const char* override_root = std::getenv("CGWA_PROFILE_ROOT");
      override_root && *override_root)
    return std::filesystem::u8path(override_root);
  const char* home = std::getenv("HOME");
  return std::filesystem::u8path(home ? home : "/tmp") /
         ".config/chatgpt-cef-v2";
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

  const std::filesystem::path root = ConfigRoot();
  const std::filesystem::path cache = root / "cache";
  std::error_code ec;
  std::filesystem::create_directories(cache, ec);
  if (ec) {
    LOG(ERROR) << "Failed creating CEF profile: " << ec.message();
    return 2;
  }

  CefSettings settings;
#if defined(_WIN32)
  CefString(&settings.root_cache_path) = root.wstring();
  CefString(&settings.cache_path) = cache.wstring();
#else
  CefString(&settings.root_cache_path) = root.string();
  CefString(&settings.cache_path) = cache.string();
#endif
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
  if (!ConfigureProfileOverride(command_line)) return 30;

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
  if (!ConfigureProfileOverride(command_line)) return 30;

  CefMainArgs main_args(argc, argv);
  CefRefPtr<CefApp> app;
  if (!IsSubprocess(command_line))
    app = new BrowserApp();

  return RunBrowser(main_args, app, nullptr);
}
#endif
