#!/usr/bin/env python3
"""Source contract for explicit, fail-closed Browser QA profile selection.
Static regression proof only; runtime positive/negative tests are independent.
"""
from pathlib import Path
import unittest

ROOT=Path(__file__).resolve().parents[1]
SRC=(ROOT/"src/main.cc").read_text(encoding="utf-8")

def method(signature):
    pos=SRC.find(signature)
    if pos<0: raise AssertionError("function missing: "+signature)
    start=SRC.index("{",pos)
    depth=0
    for end in range(start,len(SRC)):
        if SRC[end]=="{": depth+=1
        if SRC[end]=="}":
            depth-=1
            if depth==0: return SRC[pos:end+1]
    raise AssertionError("unbalanced braces "+signature)

class ProfileIsolation(unittest.TestCase):
    def test_profile_selector_returns_failure_not_silent_default(self):
        s=method("bool ConfigureProfileOverride(")
        self.assertIn('GetSwitchValue("cgwa-profile-dir")',s)
        self.assertIn("if (raw.empty()) return false;",s)
        self.assertIn("if (!path.is_absolute()) return false;",s)
        self.assertIn("return !qa_mode;",s)
    def test_all_testing_flags_require_explicit_qa_root(self):
        s=method("bool ConfigureProfileOverride(")
        for switch in ('"background-test"','"self-test"','"companion-self-test"','"shutdown-self-test"'):
            self.assertIn(switch,s)
        self.assertIn('HasSwitch("cgwa-profile-dir")',s)
    def test_qa_cannot_alias_production_default_profile(self):
        s=method("bool ConfigureProfileOverride(")
        self.assertIn("weakly_canonical(path",s)
        self.assertIn("weakly_canonical(default_root",s)
        self.assertIn("_wcsicmp(",s)
    def test_windows_and_linux_entrypoints_enforce_guard_before_cef(self):
        for signature in ("NO_STACK_PROTECTOR int APIENTRY wWinMain(","NO_STACK_PROTECTOR int main("):
            s=method(signature)
            self.assertIn("if (!ConfigureProfileOverride(command_line)) return 30;",s)
            self.assertLess(s.index("if (!ConfigureProfileOverride(command_line))"),s.index("RunBrowser("))
    def test_profile_override_environment_is_checked(self):
        s=method("bool ConfigureProfileOverride(")
        self.assertIn("_wputenv_s(",s)
        self.assertIn("setenv(",s)
        self.assertIn("== 0",s)
    def test_stable_production_without_qa_flags_is_unchanged(self):
        s=method("bool ConfigureProfileOverride(")
        self.assertIn("return !qa_mode;",s)
        self.assertIn("ConfigRoot()",SRC)
        self.assertIn("CefExecuteProcess(",SRC)
        self.assertIn("settings.persist_session_cookies = 1;",SRC)

if __name__=="__main__":
    unittest.main(verbosity=2)
