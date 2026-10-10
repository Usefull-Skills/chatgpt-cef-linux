#include "theme_policy.h"
#include <string>
#include <algorithm>
#include <cctype>
#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <cstdio>
#include <cstdlib>
#endif

namespace theme {
bool ResolveDark(int mode, bool os_dark) {
  return mode == 2 ? true : mode == 1 ? false : os_dark;
}
#if defined(_WIN32)
bool ReadSystemDark() {
  DWORD light=1,cb=sizeof(light);
  const LONG code=RegGetValueW(HKEY_CURRENT_USER,
    L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
    L"AppsUseLightTheme", RRF_RT_REG_DWORD,nullptr,&light,&cb);
  // If no OS preference can be read, use a legible light fallback.
  return code==ERROR_SUCCESS && light==0;
}
#else
namespace {
std::string ReadSetting(const char* fixed_command) {
  FILE* pipe=popen(fixed_command,"r"); if(!pipe)return {};
  char text[128]={};
  const bool ok=fgets(text,sizeof(text),pipe)!=nullptr;
  pclose(pipe);
  if(!ok)return {};
  std::string value(text);
  std::transform(value.begin(),value.end(),value.begin(),
    [](unsigned char c){return static_cast<char>(std::tolower(c));});
  return value;
}
}
bool ReadSystemDark() {
  // Commands are fixed absolute system binaries; never accept page URL,
  // profile name, or any user-provided argument. GNOME 50 supports both.
  const auto pref=ReadSetting("/usr/bin/gsettings get org.gnome.desktop.interface color-scheme 2>/dev/null");
  if(pref.find("prefer-dark")!=std::string::npos)return true;
  if(pref.find("prefer-light")!=std::string::npos)return false;
  const auto gtk=ReadSetting("/usr/bin/gsettings get org.gnome.desktop.interface gtk-theme 2>/dev/null");
  if(gtk.find("dark")!=std::string::npos)return true;
  if(const char* fallback=std::getenv("GTK_THEME")){
    std::string name(fallback);
    std::transform(name.begin(),name.end(),name.begin(),
      [](unsigned char c){return static_cast<char>(std::tolower(c));});
    return name.find("dark")!=std::string::npos;
  }
  return false;
}
#endif
} // namespace theme
