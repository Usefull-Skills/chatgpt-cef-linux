#include "theme_policy.h"
#include <cassert>
#include <iostream>

int main() {
  // OS-independent contract: explicit overrides never depend on host state.
  assert(theme::ResolveDark(0, false) == false);
  assert(theme::ResolveDark(0, true) == true);
  assert(theme::ResolveDark(1, false) == false);
  assert(theme::ResolveDark(1, true) == false);
  assert(theme::ResolveDark(2, false) == true);
  assert(theme::ResolveDark(2, true) == true);
  (void)theme::ReadSystemDark();  // Must never mutate GNOME/Windows preferences.
  std::cout << "THEME_POLICY_CONTRACT_6_OF_6_PASS\n";
  return 0;
}
