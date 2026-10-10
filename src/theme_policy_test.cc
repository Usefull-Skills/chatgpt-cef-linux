#include "theme_policy.h"
#include <cassert>
#include <iostream>
#include <fstream>
#include <iterator>
#include <string>

int main() {
  // OS-independent contract: explicit overrides never depend on host state.
  assert(theme::ResolveDark(0, false) == false);
  assert(theme::ResolveDark(0, true) == true);
  assert(theme::ResolveDark(1, false) == false);
  assert(theme::ResolveDark(1, true) == false);
  assert(theme::ResolveDark(2, false) == true);
  assert(theme::ResolveDark(2, true) == true);
  (void)theme::ReadSystemDark();  // Must never mutate GNOME/Windows preferences.
  // A recurring theme timer must never retain an AppController raw pointer.
  // Read source from the pinned CTest working directory on either OS.
  std::ifstream source("src/controller.cc",std::ios::binary);
  assert(source && "controller.cc missing from source-lifetime contract");
  const std::string code(std::istreambuf_iterator<char>{source},
                         std::istreambuf_iterator<char>{});
  const auto begin=code.find("void AppController::ThemeTick()");
  assert(begin!=std::string::npos);
  const auto end=code.find("void AppController::RebuildTabStrip()",begin);
  assert(end!=std::string::npos);
  const auto tick=code.substr(begin,end-begin);
  assert(tick.find("AppController::Get()) controller->ThemeTick()")!=std::string::npos);
  assert(tick.find("base::Unretained(this)") == std::string::npos);
  std::cout << "THEME_LIFETIME_SINGLETON_GUARD_PASS\n";
  std::cout << "THEME_POLICY_CONTRACT_6_OF_6_PASS\n";
  return 0;
}
