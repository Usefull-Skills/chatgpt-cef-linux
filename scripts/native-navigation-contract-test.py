#!/usr/bin/env python3
"""Read-only cross-platform contract check for Browser native navigation candidate.
Passing static assertions is not proof of GUI acceptance or release safety.
"""
import re
import subprocess
import unittest
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
SRC=(ROOT/"src/controller.cc").read_text(encoding="utf-8")
HDR=(ROOT/"src/controller.h").read_text(encoding="utf-8")
BASE=subprocess.check_output(["git","-C",str(ROOT),"show","495048feeac78ca1da4693e0906d9c14f031e851:src/controller.cc"]).decode("utf-8")

def method(source, signature):
    start=source.find(signature)
    if start<0: raise AssertionError("missing method: "+signature)
    brace=source.index("{",start)
    depth=0
    for i in range(brace,len(source)):
        if source[i]=="{": depth+=1
        if source[i]=="}":
            depth-=1
            if depth==0: return re.sub(r"\s+"," ",source[start:i+1]).strip()
    raise AssertionError("unbalanced method: "+signature)

class NativeNavigationContract(unittest.TestCase):
    def test_isolated_native_chrome(self):
        self.assertIn("window_->AddChildView(nav_bar_);",SRC)
        self.assertIn("nav_bar_->SetToBoxLayout(nav_settings)",SRC)
        self.assertIn("nav_site_indicator_->SetFocusable(false)",SRC)
    def test_layout_honors_toolbar(self):
        self.assertIn("kChromeHeight = kHeaderHeight + kNavigationHeight",SRC)
        self.assertIn("const int top = header_visible ? kChromeHeight : 0;",SRC)
        self.assertEqual(SRC.count("if (nav_bar_) nav_bar_->SetVisible(!fullscreen_);"),2)
    def test_button_and_keyboard_parity(self):
        for action in ("NavigateBack","NavigateForward","ReloadOrStop","NavigateHome"):
            self.assertIn("void AppController::"+action+"()",SRC)
            self.assertIn("void "+action+"();",HDR)
        for key in ("kAccelBack","kAccelForward","kAccelReload","kAccelReloadF5","kAccelHome"):
            self.assertIn("window_->SetAccelerator("+key,SRC)
    def test_loading_and_tab_switch_update_nav(self):
        self.assertIn("UpdateNavigationControls();\n  UpdateCompanionPanel();",SRC)
        self.assertIn("if (tab && tab->id == active_tab_id_) UpdateNavigationControls();",SRC)
        self.assertIn("if (tab->id == active_tab_id_) {\n    UpdateNavigationControls();",SRC)
    def test_navigation_only_user_controls(self):
        self.assertIn("if (browser && browser->CanGoBack()) browser->GoBack();",SRC)
        self.assertIn("if (browser && browser->CanGoForward()) browser->GoForward();",SRC)
        self.assertIn("if (tab->loading) browser->StopLoad();",SRC)
        self.assertIn("else browser->Reload();",SRC)
    def test_home_fixed_to_trusted_host(self):
        s=method(SRC,"void AppController::NavigateHome()")
        self.assertIn('frame->LoadURL("https://chatgpt.com/")',s)
        self.assertNotIn("OpenExternal(",s)
        self.assertNotIn("std::getenv",s)
    def test_display_only_exact_origins(self):
        s=method(SRC,"void AppController::UpdateNavigationControls()")
        self.assertIn("IsChatGPTURL(url)",s)
        self.assertIn('IsHttpsHost(url, "auth.openai.com")',s)
        self.assertIn('label = "Site: unverified";',s)
        self.assertNotIn("url.substr(",s)
    def test_no_new_unsafe_web_bridge(self):
        s=method(SRC,"void AppController::BuildNavigationControls()")
        self.assertNotIn("ExecuteJavaScript(",s)
        self.assertNotIn("shell",s.lower().replace("native controls live outside the web renderer.",""))
        self.assertNotIn("OpenExternal(",s)
    def test_emergency_stop_logic_byte_semantics_unchanged(self):
        sig="void AppController::RequestNativeEmergencyStop()"
        self.assertEqual(method(SRC,sig),method(BASE,sig))
    def test_trusted_url_filter_unchanged(self):
        for sig in ("bool AppController::IsChatGPTURL(","bool AppController::IsInternalNavigationURL(",
                    "bool AppController::OpenExternal("):
            self.assertEqual(method(SRC,sig),method(BASE,sig))
    def test_profile_restore_policy_unchanged(self):
        self.assertEqual(method(SRC,"void AppController::SaveSessionState()"),
                         method(BASE,"void AppController::SaveSessionState()"))
    def test_existing_stop_and_theme_not_removed(self):
        for required in ("kCompanionEmergencyStopButton","kThemeButton",
                         "RequestNativeEmergencyStop()","RefreshTheme()"):
            self.assertIn(required,SRC)

if __name__=="__main__":
    unittest.main(verbosity=2)
