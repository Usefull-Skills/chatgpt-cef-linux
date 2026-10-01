#include "controller.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <algorithm>
#include <cerrno>
#include <cctype>
#include <cstdio>
#include <csignal>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "client.h"
#include "include/base/cef_bind.h"
#include "include/base/cef_callback.h"
#include "include/base/cef_logging.h"
#include "include/cef_app.h"
#include "include/cef_command_line.h"
#include "include/cef_task.h"
#include "include/views/cef_fill_layout.h"
#include "include/internal/cef_types_wrappers.h"
#include "include/wrapper/cef_helpers.h"
#include "include/wrapper/cef_closure_task.h"

namespace {
constexpr int kBrandButton = 10;
constexpr int kNewTabButton = 20;
constexpr int kFullscreenButton = 21;
constexpr int kMaximizeButton = 22;
constexpr int kMinimizeButton = 23;
constexpr int kCloseWindowButton = 24;
constexpr int kTabButtonBase = 1000;
constexpr int kTabCloseBase = 2000;

constexpr int kHeaderHeight = 38;
constexpr int kBrandWidth = 96;

constexpr int kAccelNewTab = 100;
constexpr int kAccelCloseTab = 101;
constexpr int kAccelNextTab = 102;
constexpr int kAccelPrevTab = 103;
constexpr int kAccelFullscreen = 104;
constexpr int kAccelTab1 = 111;

constexpr int kVKeyTab = 0x09;
constexpr int kVKey1 = 0x31;
constexpr int kVKeyT = 0x54;
constexpr int kVKeyW = 0x57;
constexpr int kVKeyF11 = 0x7A;

// R07 modern visual system: light neutral surfaces + a single blue accent.
constexpr cef_color_t kHeaderBg = CefColorSetARGB(255, 248, 250, 252);      // #F8FAFC
constexpr cef_color_t kTabBg = CefColorSetARGB(255, 248, 250, 252);         // ghost
constexpr cef_color_t kTabHoverBg = CefColorSetARGB(255, 238, 242, 247);    // #EEF2F7
constexpr cef_color_t kTabActiveBg = CefColorSetARGB(255, 255, 255, 255);   // #FFFFFF
constexpr cef_color_t kButtonBg = CefColorSetARGB(255, 248, 250, 252);      // ghost
constexpr cef_color_t kButtonHoverBg = CefColorSetARGB(255, 238, 242, 247); // #EEF2F7
constexpr cef_color_t kSoftAccentBg = CefColorSetARGB(255, 239, 246, 255);  // #EFF6FF
constexpr cef_color_t kPrimaryBg = CefColorSetARGB(255, 37, 99, 235);       // #2563EB
constexpr cef_color_t kPrimaryHoverBg = CefColorSetARGB(255, 29, 78, 216); // #1D4ED8
constexpr cef_color_t kText = CefColorSetARGB(255, 15, 23, 42);             // #0F172A
constexpr cef_color_t kMutedText = CefColorSetARGB(255, 100, 116, 139);     // #64748B
constexpr cef_color_t kActiveText = CefColorSetARGB(255, 29, 78, 216);      // #1D4ED8
constexpr cef_color_t kDangerBg = CefColorSetARGB(255, 248, 250, 252);      // ghost normal
constexpr cef_color_t kDangerHoverBg = CefColorSetARGB(255, 254, 226, 226); // #FEE2E2
constexpr cef_color_t kDangerText = CefColorSetARGB(255, 220, 38, 38);      // #DC2626
constexpr char kUIFont[] = "Vazirmatn, Noto Sans Arabic, DejaVu Sans, 12px";
constexpr char kUIBoldFont[] = "Vazirmatn, Noto Sans Arabic, DejaVu Sans, Bold 12px";

std::string StatePath() {
  if (const char* override_root = std::getenv("CGWA_PROFILE_ROOT");
      override_root && *override_root)
    return std::string(override_root) + "/tabs.state";
  const char* home = std::getenv("HOME");
  std::string base = home ? home : "/tmp";
  return base + "/.config/chatgpt-cef-v2/tabs.state";
}

bool StartsWith(const std::string& s, const std::string& p) {
  return s.rfind(p, 0) == 0;
}

bool IsHttpsHost(const std::string& url, const std::string& host) {
  const std::string prefix = "https://" + host;
  if (!StartsWith(url, prefix)) return false;
  if (url.size() == prefix.size()) return true;
  const char boundary = url[prefix.size()];
  return boundary == '/' || boundary == '?' || boundary == '#';
}

std::string Utf8Ellipsize(const std::string& input, size_t max_chars,
                          size_t keep_chars) {
  size_t i = 0;
  size_t chars = 0;
  size_t keep_bytes = input.size();
  while (i < input.size()) {
    if (chars == keep_chars) keep_bytes = i;
    const unsigned char c = static_cast<unsigned char>(input[i]);
    size_t len = 1;
    if ((c & 0xE0) == 0xC0) len = 2;
    else if ((c & 0xF0) == 0xE0) len = 3;
    else if ((c & 0xF8) == 0xF0) len = 4;
    if (i + len > input.size()) len = 1;
    i += len;
    ++chars;
  }
  if (chars <= max_chars) return input;
  if (keep_bytes > input.size()) keep_bytes = input.size();
  return input.substr(0, keep_bytes) + "…";
}

void SetX11WindowClass(CefRefPtr<CefWindow> window) {
  if (!window) return;
  Display* display = XOpenDisplay(nullptr);
  if (!display) return;
  const ::Window xid = static_cast<::Window>(window->GetWindowHandle());
  XClassHint hint{};
  char res_name[] = "chatgpt-cef-v2";
  char res_class[] = "ChatGPT-CEF-V2";
  hint.res_name = res_name;
  hint.res_class = res_class;
  XSetClassHint(display, xid, &hint);
  XFlush(display);
  XCloseDisplay(display);
}

}  // namespace

AppController* AppController::instance_ = nullptr;

class AppController::FixedPanelDelegate : public CefPanelDelegate {
 public:
  FixedPanelDelegate(int width, int height) : width_(width), height_(height) {}
  CefSize GetPreferredSize(CefRefPtr<CefView> view) override {
    return CefSize(width_, height_);
  }
  void OnThemeChanged(CefRefPtr<CefView> view) override {
    if (view) view->SetBackgroundColor(kHeaderBg);
  }
 private:
  int width_;
  int height_;
  IMPLEMENT_REFCOUNTING(FixedPanelDelegate);
};

class AppController::ButtonDelegateImpl : public CefButtonDelegate {
 public:
  explicit ButtonDelegateImpl(AppController* owner) : owner_(owner) {}
  void OnButtonPressed(CefRefPtr<CefButton> button) override {
    if (owner_) owner_->OnButtonPressed(button);
  }
  void OnButtonStateChanged(CefRefPtr<CefButton> button) override {
    if (owner_) owner_->OnButtonStateChanged(button);
  }
  void OnThemeChanged(CefRefPtr<CefView> view) override {
    if (owner_ && view) {
      auto button = view->AsButton();
      if (button) owner_->OnButtonStateChanged(button);
    }
  }
  CefSize GetPreferredSize(CefRefPtr<CefView> view) override {
    const int id = view->GetID();
    if (id == kBrandButton) return CefSize(kBrandWidth, 30);
    if (id >= kTabCloseBase) return CefSize(24, 28);
    if (id >= kTabButtonBase) return CefSize(148, 28);
    if (id >= kNewTabButton && id <= kCloseWindowButton) return CefSize(32, 30);
    return CefSize(80, 34);
  }
 private:
  AppController* owner_;
  IMPLEMENT_REFCOUNTING(ButtonDelegateImpl);
};

class AppController::BrowserViewDelegateImpl : public CefBrowserViewDelegate {
 public:
  explicit BrowserViewDelegateImpl(AppController* owner) : owner_(owner) {}
  cef_runtime_style_t GetBrowserRuntimeStyle() override {
    return CEF_RUNTIME_STYLE_ALLOY;
  }
  bool OnPopupBrowserViewCreated(CefRefPtr<CefBrowserView> browser_view,
                                 CefRefPtr<CefBrowserView> popup_browser_view,
                                 bool is_devtools) override {
    return false;
  }
 private:
  AppController* owner_;
  IMPLEMENT_REFCOUNTING(BrowserViewDelegateImpl);
};

class AppController::WindowDelegateImpl : public CefWindowDelegate {
 public:
  explicit WindowDelegateImpl(AppController* owner) : owner_(owner) {}
  void OnWindowCreated(CefRefPtr<CefWindow> window) override {
    if (owner_) owner_->OnWindowCreated(window);
  }
  void OnWindowBoundsChanged(CefRefPtr<CefWindow> window,
                             const CefRect& new_bounds) override {
    if (owner_) owner_->OnWindowBoundsChanged(new_bounds);
  }
  void OnWindowFullscreenTransition(CefRefPtr<CefWindow> window,
                                    bool is_completed) override {
    if (owner_) owner_->OnWindowFullscreenTransition(is_completed);
  }
  void OnWindowDestroyed(CefRefPtr<CefWindow> window) override {
    if (owner_) owner_->OnWindowDestroyed();
  }
  CefRect GetInitialBounds(CefRefPtr<CefWindow> window) override {
    return CefRect(120, 70, 1180, 620);
  }
  cef_show_state_t GetInitialShowState(CefRefPtr<CefWindow> window) override {
    auto command_line = CefCommandLine::GetGlobalCommandLine();
    if (command_line && command_line->HasSwitch("background-test"))
      return CEF_SHOW_STATE_MINIMIZED;
    return CEF_SHOW_STATE_NORMAL;
  }
  bool IsFrameless(CefRefPtr<CefWindow> window) override { return true; }
  bool CanResize(CefRefPtr<CefWindow> window) override { return true; }
  bool CanMaximize(CefRefPtr<CefWindow> window) override { return true; }
  bool CanMinimize(CefRefPtr<CefWindow> window) override { return true; }
  bool CanClose(CefRefPtr<CefWindow> window) override {
    return owner_ ? owner_->CanWindowClose() : true;
  }
  bool OnAccelerator(CefRefPtr<CefWindow> window, int command_id) override {
    return owner_ ? owner_->OnAccelerator(command_id) : false;
  }
  cef_runtime_style_t GetWindowRuntimeStyle() override {
    return CEF_RUNTIME_STYLE_ALLOY;
  }
 private:
  AppController* owner_;
  IMPLEMENT_REFCOUNTING(WindowDelegateImpl);
};

AppController::AppController() {
  DCHECK(!instance_);
  instance_ = this;
  client_ = new AppClient();
  window_delegate_ = new WindowDelegateImpl(this);
  button_delegate_ = new ButtonDelegateImpl(this);
  browser_view_delegate_ = new BrowserViewDelegateImpl(this);
  header_delegate_ = new FixedPanelDelegate(1180, kHeaderHeight);
}

AppController::~AppController() {
  if (instance_ == this) instance_ = nullptr;
}

AppController* AppController::Get() { return instance_; }

void AppController::CreateMainWindow() {
  CEF_REQUIRE_UI_THREAD();
  if (window_created_ && window_) {
    ActivateMainWindow();
    return;
  }
  CefWindow::CreateTopLevelWindow(window_delegate_);
}

void AppController::ActivateMainWindow() {
  CEF_REQUIRE_UI_THREAD();
  if (!window_ || window_->IsClosed()) return;
  if (window_->IsMinimized()) window_->Restore();
  window_->Show();
  window_->BringToTop();
  window_->Activate();
}

void AppController::OnWindowCreated(CefRefPtr<CefWindow> window) {
  CEF_REQUIRE_UI_THREAD();
  window_ = window;
  window_created_ = true;
  window_->SetTitle("ChatGPT");
  SetX11WindowClass(window_);
  BuildWindowUI();
  window_->CenterWindow(CefSize(1180, 620));
  window_->Show();
  auto command_line = CefCommandLine::GetGlobalCommandLine();
  const bool background_test =
      command_line && command_line->HasSwitch("background-test");
  if (background_test)
    window_->Minimize();
  else
    window_->Activate();

  // Restore simple URL session state. Each line is a URL. The first line may be
  // "active=N". Never restore more than 8 tabs.
  std::ifstream in(StatePath());
  std::vector<std::string> urls;
  int active_index = 0;
  std::string line;
  if (in && std::getline(in, line) && StartsWith(line, "active=")) {
    active_index = std::max(0, std::atoi(line.substr(7).c_str()));
  } else if (!line.empty()) {
    urls.push_back(line);
  }
  while (in && std::getline(in, line) && urls.size() < 8) {
    if (StartsWith(line, "https://")) urls.push_back(line);
  }
  if (urls.empty()) urls.push_back("https://chatgpt.com/");
  for (const auto& url : urls) NewTab(url);
  if (active_index >= 0 && active_index < static_cast<int>(tabs_.size()))
    SetActiveTabInternal(tabs_[active_index].id);

  if (command_line && command_line->HasSwitch("self-test"))
    RunBackgroundSelfTest();

  if (command_line && command_line->HasSwitch("shutdown-self-test")) {
    LOG(WARNING) << "CGWA_SHUTDOWN SELFTEST_SCHEDULED";
    CefPostDelayedTask(
        TID_UI,
        base::BindOnce(&AppController::RequestCloseWindow,
                       base::Unretained(this)),
        7000);
  }
}

void AppController::RunBackgroundSelfTest() {
  CEF_REQUIRE_UI_THREAD();
  self_test_base_tabs_ = tabs_.size();
  NewTab("https://chatgpt.com/");
  self_test_tab_id_ = active_tab_id_;
  Tab* tab = FindTabById(self_test_tab_id_);
  if (tab && tab->view && tab->view->GetBrowser())
    self_test_browser_id_ = tab->view->GetBrowser()->GetIdentifier();
  LOG(WARNING) << "CGWA_SELFTEST TAB_CREATE before=" << self_test_base_tabs_
               << " after=" << tabs_.size()
               << " browser=" << self_test_browser_id_;
  CefPostDelayedTask(
      TID_UI,
      base::BindOnce(&AppController::BackgroundSelfTestClose,
                     base::Unretained(this)),
      1800);
}

void AppController::BackgroundSelfTestClose() {
  CEF_REQUIRE_UI_THREAD();
  Tab* tab = FindTabById(self_test_tab_id_);
  if (tab && tab->view && tab->view->GetBrowser())
    self_test_browser_id_ = tab->view->GetBrowser()->GetIdentifier();
  CloseTab(self_test_tab_id_);
  LOG(WARNING) << "CGWA_SELFTEST TAB_CLOSE_REQUEST current=" << tabs_.size()
               << " browser=" << self_test_browser_id_;
  CefPostDelayedTask(
      TID_UI,
      base::BindOnce(&AppController::BackgroundSelfTestVerify,
                     base::Unretained(this)),
      2500);
}

void AppController::BackgroundSelfTestVerify() {
  CEF_REQUIRE_UI_THREAD();
  auto browser = self_test_browser_id_ > 0
                     ? CefBrowserHost::GetBrowserByIdentifier(self_test_browser_id_)
                     : nullptr;
  const bool pass = tabs_.size() == self_test_base_tabs_ && !browser;
  LOG(WARNING) << "CGWA_SELFTEST " << (pass ? "PASS" : "FAIL")
               << " final_tabs=" << tabs_.size()
               << " browser=" << self_test_browser_id_
               << " still_exists=" << (browser ? 1 : 0);
}

void AppController::OnWindowBoundsChanged(const CefRect& new_bounds) {
  CEF_REQUIRE_UI_THREAD();
  LayoutTabOverlays();
}

void AppController::OnWindowFullscreenTransition(bool is_completed) {
  CEF_REQUIRE_UI_THREAD();
  if (!is_completed || !window_) return;
  fullscreen_ = window_->IsFullscreen();
  if (header_) header_->SetVisible(!fullscreen_);
  window_->Layout();
  LayoutTabOverlays();
}

void AppController::BuildWindowUI() {
  CefBoxLayoutSettings vertical;
  vertical.horizontal = false;
  vertical.between_child_spacing = 0;
  vertical.inside_border_horizontal_spacing = 0;
  vertical.inside_border_vertical_spacing = 0;
  vertical.cross_axis_alignment = CEF_AXIS_ALIGNMENT_STRETCH;
  vertical.default_flex = 0;
  window_layout_ = window_->SetToBoxLayout(vertical);

  header_ = CefPanel::CreatePanel(header_delegate_);
  header_->SetBackgroundColor(kHeaderBg);
  CefBoxLayoutSettings horizontal;
  horizontal.horizontal = true;
  horizontal.between_child_spacing = 2;
  horizontal.inside_border_horizontal_spacing = 8;
  horizontal.inside_border_vertical_spacing = 4;
  horizontal.cross_axis_alignment = CEF_AXIS_ALIGNMENT_CENTER;
  horizontal.default_flex = 0;
  header_layout_ = header_->SetToBoxLayout(horizontal);

  tab_strip_ = CefPanel::CreatePanel(nullptr);
  tab_strip_->SetBackgroundColor(kHeaderBg);
  CefBoxLayoutSettings tabs_layout;
  tabs_layout.horizontal = true;
  tabs_layout.between_child_spacing = 2;
  tabs_layout.cross_axis_alignment = CEF_AXIS_ALIGNMENT_CENTER;
  tabs_layout.default_flex = 0;
  tab_layout_ = tab_strip_->SetToBoxLayout(tabs_layout);

  content_ = CefPanel::CreatePanel(nullptr);
  content_->SetToFillLayout();
  content_->SetBackgroundColor(CefColorSetARGB(255, 0, 0, 0));

  window_->AddChildView(header_);
  window_->AddChildView(content_);
  window_layout_->SetFlexForView(content_, 1);
  BuildHeaderControls();

  std::vector<CefDraggableRegion> regions;
  regions.emplace_back(CefRect(0, 0, kBrandWidth, kHeaderHeight), true);
  window_->SetDraggableRegions(regions);

  // High-priority native accelerators work regardless of page focus.
  window_->SetAccelerator(kAccelNewTab, kVKeyT, false, true, false, true);
  window_->SetAccelerator(kAccelCloseTab, kVKeyW, false, true, false, true);
  window_->SetAccelerator(kAccelNextTab, kVKeyTab, false, true, false, true);
  window_->SetAccelerator(kAccelPrevTab, kVKeyTab, true, true, false, true);
  window_->SetAccelerator(kAccelFullscreen, kVKeyF11, false, false, false, true);
  for (int i = 0; i < 8; ++i)
    window_->SetAccelerator(kAccelTab1 + i, kVKey1 + i, false, true, false, true);
}

void AppController::BuildHeaderControls() {
  auto make_button = [&](int id, const std::string& text, cef_color_t bg) {
    auto button = CefLabelButton::CreateLabelButton(button_delegate_, text);
    button->SetID(id);
    button->SetBackgroundColor(bg);
    button->SetEnabledTextColors(kText);
    button->SetFontList(kUIFont);
    button->SetInkDropEnabled(true);
    return button;
  };

  auto brand = make_button(kBrandButton, "ChatGPT", kHeaderBg);
  brand->SetFontList(kUIBoldFont);
  brand->SetEnabledTextColors(kText);
  brand->SetAccessibleName("ChatGPT application");
  header_->AddChildView(brand);
  header_->AddChildView(tab_strip_);
  header_layout_->SetFlexForView(tab_strip_, 1);

  auto add = make_button(kNewTabButton, "＋", kButtonBg);
  add->SetEnabledTextColors(kActiveText);
  add->SetTooltipText("New tab (Ctrl+T)");
  add->SetAccessibleName("New tab");
  header_->AddChildView(add);

  auto full = make_button(kFullscreenButton, "⛶", kButtonBg);
  full->SetTooltipText("Fullscreen (F11)");
  full->SetAccessibleName("Toggle fullscreen");
  header_->AddChildView(full);

  auto max = make_button(kMaximizeButton, "▢", kButtonBg);
  max->SetTooltipText("Maximize / Restore");
  max->SetAccessibleName("Maximize or restore");
  header_->AddChildView(max);

  auto min = make_button(kMinimizeButton, "−", kButtonBg);
  min->SetTooltipText("Minimize");
  min->SetAccessibleName("Minimize");
  header_->AddChildView(min);

  auto close = make_button(kCloseWindowButton, "×", kDangerBg);
  close->SetEnabledTextColors(kMutedText);
  close->SetTooltipText("Close");
  close->SetAccessibleName("Close application");
  header_->AddChildView(close);
}

void AppController::NewTab(const std::string& url) {
  CEF_REQUIRE_UI_THREAD();
  if (!window_ || tabs_.size() >= 8) return;

  const std::string initial_url = url.empty() ? "https://chatgpt.com/" : url;
  CefBrowserSettings settings;
  auto view = CefBrowserView::CreateBrowserView(
      client_, initial_url, settings, nullptr, nullptr, browser_view_delegate_);

  Tab tab;
  tab.id = next_tab_id_++;
  tab.view = view;
  tab.url = initial_url;
  tab.loading = true;
  tabs_.push_back(tab);

  Tab& added = tabs_.back();
  added.overlay = window_->AddOverlayView(
      added.view, CEF_DOCKING_MODE_CUSTOM, true);
  if (!added.overlay || !added.overlay->IsValid()) {
    LOG(ERROR) << "Failed to create overlay tab id=" << added.id;
    tabs_.pop_back();
    return;
  }
  // Overlay BrowserViews are hidden by default. Only the active tab is shown.
  added.overlay->SetVisible(false);
  LayoutTabOverlays();
  RebuildTabStrip();
  SetActiveTabInternal(added.id);
}

void AppController::ActivateTab(int tab_id) {
  CEF_REQUIRE_UI_THREAD();
  SetActiveTabInternal(tab_id);
}

void AppController::SetActiveTabInternal(int tab_id) {
  Tab* target = FindTabById(tab_id);
  if (!target) return;
  active_tab_id_ = tab_id;
  for (auto& tab : tabs_) {
    const bool active = tab.id == tab_id;
    if (tab.overlay && tab.overlay->IsValid())
      tab.overlay->SetVisible(active);
    UpdateTabButton(tab);
  }
  if (target->view) target->view->RequestFocus();
  SaveSessionState();
}

void AppController::LayoutTabOverlays() {
  CEF_REQUIRE_UI_THREAD();
  if (!window_ || window_->IsClosed()) return;
  const CefRect client_bounds = window_->GetClientAreaBoundsInScreen();
  const bool header_visible = header_ && header_->IsVisible();
  const int top = header_visible ? kHeaderHeight : 0;
  const int width = std::max(1, client_bounds.width);
  const int height = std::max(1, client_bounds.height - top);
  const CefRect bounds(0, top, width, height);
  for (auto& tab : tabs_) {
    if (tab.overlay && tab.overlay->IsValid())
      tab.overlay->SetBounds(bounds);
  }
}

void AppController::SaveSessionState() {
  if (tabs_.empty()) return;
  const std::string path = StatePath();
  const std::string temp = path + ".tmp";
  std::ofstream out(temp, std::ios::trunc);
  if (!out) return;
  int active_index = 0;
  for (size_t i = 0; i < tabs_.size(); ++i)
    if (tabs_[i].id == active_tab_id_) active_index = static_cast<int>(i);
  out << "active=" << active_index << "\n";
  for (auto& tab : tabs_) {
    std::string url = tab.url.empty() ? "https://chatgpt.com/" : tab.url;
    if (tab.view && tab.view->GetBrowser() && tab.view->GetBrowser()->GetMainFrame()) {
      const std::string current = tab.view->GetBrowser()->GetMainFrame()->GetURL().ToString();
      if (IsChatGPTURL(current)) {
        url = current;
        tab.url = current;
      }
    }
    if (IsChatGPTURL(url)) out << url << "\n";
  }
  out.flush();
  const bool good = out.good();
  out.close();
  if (!good) {
    ::unlink(temp.c_str());
    return;
  }
  ::chmod(temp.c_str(), 0600);
  if (::rename(temp.c_str(), path.c_str()) != 0)
    ::unlink(temp.c_str());
}

void AppController::CloseTab(int tab_id) {
  CEF_REQUIRE_UI_THREAD();
  auto it = std::find_if(tabs_.begin(), tabs_.end(),
                         [tab_id](const Tab& t) { return t.id == tab_id; });
  if (it == tabs_.end() || it->closing) return;
  if (tabs_.size() == 1) {
    RequestCloseWindow();
    return;
  }

  const size_t index = static_cast<size_t>(std::distance(tabs_.begin(), it));
  const bool was_active = it->id == active_tab_id_;
  const int browser_id = it->browser_id > 0
                             ? it->browser_id
                             : (it->view && it->view->GetBrowser()
                                    ? it->view->GetBrowser()->GetIdentifier()
                                    : -1);
  auto overlay = it->overlay;
  it->closing = true;
  if (overlay && overlay->IsValid()) overlay->SetVisible(false);

  tabs_.erase(it);
  RebuildTabStrip();
  if (was_active && !tabs_.empty()) {
    const size_t next = std::min(index, tabs_.size() - 1);
    SetActiveTabInternal(tabs_[next].id);
  } else if (!tabs_.empty()) {
    SetActiveTabInternal(active_tab_id_);
  }

  LOG(WARNING) << "CGWA_TAB DESTROY tab=" << tab_id
               << " browser=" << browser_id
               << " remaining=" << tabs_.size();
  if (overlay && overlay->IsValid()) overlay->Destroy();
}

void AppController::CloseActiveTab() { CloseTab(active_tab_id_); }

void AppController::CycleTab(int delta) {
  if (tabs_.empty()) return;
  int index = 0;
  for (size_t i = 0; i < tabs_.size(); ++i)
    if (tabs_[i].id == active_tab_id_) index = static_cast<int>(i);
  const int n = static_cast<int>(tabs_.size());
  index = (index + delta + n) % n;
  SetActiveTabInternal(tabs_[index].id);
}

void AppController::ToggleFullscreen() {
  CEF_REQUIRE_UI_THREAD();
  if (!window_) return;
  fullscreen_ = !window_->IsFullscreen();
  if (header_) header_->SetVisible(!fullscreen_);
  window_->SetFullscreen(fullscreen_);
  window_->Layout();
  LayoutTabOverlays();
  if (!fullscreen_) {
    std::vector<CefDraggableRegion> regions;
    regions.emplace_back(CefRect(0, 0, kBrandWidth, kHeaderHeight), true);
    window_->SetDraggableRegions(regions);
  } else {
    window_->SetDraggableRegions({});
  }
}

void AppController::ToggleMaximize() {
  if (!window_) return;
  if (window_->IsMaximized()) window_->Restore(); else window_->Maximize();
}

void AppController::Minimize() {
  if (window_) window_->Minimize();
}

void AppController::BeginShutdown() {
  CEF_REQUIRE_UI_THREAD();
  if (closing_) return;

  SaveSessionState();
  closing_ = true;
  LOG(WARNING) << "CGWA_SHUTDOWN BEGIN tabs=" << tabs_.size();

  std::vector<CefRefPtr<CefOverlayController>> overlays;
  for (auto& tab : tabs_) {
    tab.closing = true;
    if (tab.browser_id <= 0 && tab.view && tab.view->GetBrowser())
      tab.browser_id = tab.view->GetBrowser()->GetIdentifier();
    if (tab.overlay && tab.overlay->IsValid())
      overlays.push_back(tab.overlay);
    tab.overlay = nullptr;
    tab.view = nullptr;
  }

  // Destroying an overlay that has a live BrowserView causes the browser to
  // complete its normal OnBeforeClose lifecycle. Tabs without a created
  // browser can be removed immediately because no callback will arrive.
  for (auto& overlay : overlays)
    if (overlay && overlay->IsValid()) overlay->Destroy();

  tabs_.erase(std::remove_if(tabs_.begin(), tabs_.end(),
                             [](const Tab& t) { return t.browser_id <= 0; }),
              tabs_.end());

  if (tabs_.empty())
    CefPostTask(TID_UI, base::BindOnce(&AppController::FinalizeShutdown,
                                       base::Unretained(this)));
}

void AppController::FinalizeShutdown() {
  CEF_REQUIRE_UI_THREAD();
  if (!closing_ || !tabs_.empty()) return;
  LOG(WARNING) << "CGWA_SHUTDOWN FINALIZE";
  if (window_ && !window_->IsClosed())
    window_->Close();
  else
    CefQuitMessageLoop();
}

void AppController::RequestCloseWindow() {
  CEF_REQUIRE_UI_THREAD();
  if (!window_) return;
  if (!closing_) {
    BeginShutdown();
    return;
  }
  if (tabs_.empty()) FinalizeShutdown();
}

bool AppController::CanWindowClose() {
  CEF_REQUIRE_UI_THREAD();
  if (!closing_) {
    BeginShutdown();
    return false;
  }
  return tabs_.empty();
}

bool AppController::OnAccelerator(int command_id) {
  CEF_REQUIRE_UI_THREAD();
  if (command_id == kAccelNewTab) { NewTab(); return true; }
  if (command_id == kAccelCloseTab) { CloseActiveTab(); return true; }
  if (command_id == kAccelNextTab) { CycleTab(1); return true; }
  if (command_id == kAccelPrevTab) { CycleTab(-1); return true; }
  if (command_id == kAccelFullscreen) { ToggleFullscreen(); return true; }
  if (command_id >= kAccelTab1 && command_id < kAccelTab1 + 8) {
    const int index = command_id - kAccelTab1;
    if (index < static_cast<int>(tabs_.size())) SetActiveTabInternal(tabs_[index].id);
    return true;
  }
  return false;
}

void AppController::OnButtonStateChanged(CefRefPtr<CefButton> button) {
  CEF_REQUIRE_UI_THREAD();
  if (!button) return;
  const int id = button->GetID();
  const auto state = button->GetState();
  const bool hot = state == CEF_BUTTON_STATE_HOVERED ||
                   state == CEF_BUTTON_STATE_PRESSED;

  cef_color_t bg = hot ? kButtonHoverBg : kButtonBg;
  cef_color_t fg = hot ? kText : kMutedText;

  if (id == kBrandButton) {
    bg = kHeaderBg;
    fg = kText;
  } else if (id == kNewTabButton) {
    bg = hot ? kSoftAccentBg : kButtonBg;
    fg = kActiveText;
  } else if (id == kCloseWindowButton) {
    bg = hot ? kDangerHoverBg : kDangerBg;
    fg = hot ? kDangerText : kMutedText;
  } else if (id >= kTabCloseBase) {
    const int tab_id = id - kTabCloseBase;
    const bool active = tab_id == active_tab_id_;
    bg = hot ? kDangerHoverBg : (active ? kTabActiveBg : kTabBg);
    fg = hot ? kDangerText : kMutedText;
  } else if (id >= kTabButtonBase) {
    const int tab_id = id - kTabButtonBase;
    const bool active = tab_id == active_tab_id_;
    bg = hot ? kTabHoverBg : (active ? kTabActiveBg : kTabBg);
    fg = active ? kActiveText : (hot ? kText : kMutedText);
  }

  button->SetBackgroundColor(bg);
  if (auto label = button->AsLabelButton())
    label->SetEnabledTextColors(fg);
}

void AppController::OnButtonPressed(CefRefPtr<CefButton> button) {
  CEF_REQUIRE_UI_THREAD();
  const int id = button->GetID();
  if (id == kBrandButton) return;
  if (id == kNewTabButton) { NewTab(); return; }
  if (id == kFullscreenButton) { ToggleFullscreen(); return; }
  if (id == kMaximizeButton) { ToggleMaximize(); return; }
  if (id == kMinimizeButton) { Minimize(); return; }
  if (id == kCloseWindowButton) { RequestCloseWindow(); return; }
  if (id >= kTabCloseBase) { CloseTab(id - kTabCloseBase); return; }
  if (id >= kTabButtonBase) { ActivateTab(id - kTabButtonBase); return; }
}

void AppController::RebuildTabStrip() {
  if (!tab_strip_) return;
  tab_strip_->SetBackgroundColor(kHeaderBg);
  tab_strip_->RemoveAllChildViews();
  for (auto& tab : tabs_) {
    auto item = CefPanel::CreatePanel(nullptr);
    CefBoxLayoutSettings settings;
    settings.horizontal = true;
    settings.between_child_spacing = 1;
    settings.cross_axis_alignment = CEF_AXIS_ALIGNMENT_CENTER;
    item->SetToBoxLayout(settings);
    item->SetBackgroundColor(kHeaderBg);

    auto title = CefLabelButton::CreateLabelButton(button_delegate_, ShortTitle(tab.title));
    title->SetID(kTabButtonBase + tab.id);
    title->SetBackgroundColor(tab.id == active_tab_id_ ? kTabActiveBg : kTabBg);
    title->SetEnabledTextColors(tab.id == active_tab_id_ ? kActiveText : kMutedText);
    title->SetFontList(tab.id == active_tab_id_ ? kUIBoldFont : kUIFont);
    title->SetInkDropEnabled(true);

    auto close = CefLabelButton::CreateLabelButton(button_delegate_, "×");
    close->SetID(kTabCloseBase + tab.id);
    close->SetBackgroundColor(tab.id == active_tab_id_ ? kTabActiveBg : kTabBg);
    close->SetEnabledTextColors(kMutedText);
    close->SetFontList(kUIFont);
    close->SetTooltipText("Close tab");
    close->SetAccessibleName("Close tab");
    close->SetInkDropEnabled(true);

    item->AddChildView(title);
    item->AddChildView(close);
    tab.ui_panel = item;
    tab.title_button = title;
    tab.close_button = close;
    tab_strip_->AddChildView(item);
  }
  tab_strip_->Layout();
  if (header_) header_->Layout();
}

void AppController::UpdateTabButton(Tab& tab) {
  if (!tab.title_button) return;
  std::string text = ShortTitle(tab.title);
  if (tab.loading) text = "• " + text;
  tab.title_button->SetText(text);
  const bool active = tab.id == active_tab_id_;
  tab.title_button->SetBackgroundColor(active ? kTabActiveBg : kTabBg);
  tab.title_button->SetEnabledTextColors(active ? kActiveText : kMutedText);
  tab.title_button->SetFontList(active ? kUIBoldFont : kUIFont);
  if (tab.close_button) {
    tab.close_button->SetBackgroundColor(active ? kTabActiveBg : kTabBg);
    tab.close_button->SetEnabledTextColors(kMutedText);
  }
}

AppController::Tab* AppController::FindTabById(int tab_id) {
  const auto it = std::find_if(tabs_.begin(), tabs_.end(),
                               [tab_id](const Tab& tab) { return tab.id == tab_id; });
  return it == tabs_.end() ? nullptr : &(*it);
}

AppController::Tab* AppController::FindTabByBrowser(CefRefPtr<CefBrowser> browser) {
  if (!browser) return nullptr;
  const int bid = browser->GetIdentifier();
  for (auto& tab : tabs_) {
    if (tab.browser_id == bid) return &tab;
    if (tab.view && tab.view->GetBrowser() &&
        tab.view->GetBrowser()->GetIdentifier() == bid) return &tab;
  }
  return nullptr;
}

void AppController::OnBrowserCreated(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();
  LOG(WARNING) << "CGWA_LIFE ControllerCreated browser=" << browser->GetIdentifier();
  Tab* tab = FindTabByBrowser(browser);
  if (!tab) {
    for (auto& candidate : tabs_) {
      if (candidate.browser_id < 0 && candidate.view &&
          candidate.view->GetBrowser() &&
          candidate.view->GetBrowser()->GetIdentifier() == browser->GetIdentifier()) {
        tab = &candidate;
        break;
      }
    }
  }
  if (tab) tab->browser_id = browser->GetIdentifier();
}

bool AppController::OnBrowserDoClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();
  LOG(WARNING) << "CGWA_LIFE DoClose passthrough browser="
               << (browser ? browser->GetIdentifier() : -1);
  return false;
}

void AppController::OnBrowserClosed(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();
  const int bid = browser ? browser->GetIdentifier() : -1;
  LOG(WARNING) << "CGWA_LIFE ControllerClosed browser=" << bid
               << " total_before=" << tabs_.size();

  auto it = std::find_if(tabs_.begin(), tabs_.end(),
                         [bid](const Tab& t) { return t.browser_id == bid; });
  if (it == tabs_.end()) return;

  const bool was_active = it->id == active_tab_id_;
  const size_t index = static_cast<size_t>(std::distance(tabs_.begin(), it));
  tabs_.erase(it);
  if (closing_) {
    LOG(WARNING) << "CGWA_SHUTDOWN BROWSER_CLOSED remaining=" << tabs_.size();
    if (tabs_.empty())
      CefPostTask(TID_UI, base::BindOnce(&AppController::FinalizeShutdown,
                                         base::Unretained(this)));
    return;
  }

  RebuildTabStrip();
  if (tabs_.empty()) {
    NewTab();
  } else if (was_active) {
    const size_t next = std::min(index, tabs_.size() - 1);
    SetActiveTabInternal(tabs_[next].id);
  } else {
    SetActiveTabInternal(active_tab_id_);
  }
}

void AppController::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                  const std::string& title) {
  CEF_REQUIRE_UI_THREAD();
  Tab* tab = FindTabByBrowser(browser);
  if (!tab) return;
  tab->title = title.empty() ? "ChatGPT" : title;
  UpdateTabButton(*tab);
}

void AppController::OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
                                         bool is_loading) {
  CEF_REQUIRE_UI_THREAD();
  Tab* tab = FindTabByBrowser(browser);
  if (!tab) return;
  tab->loading = is_loading;
  if (browser && browser->GetMainFrame()) {
    const std::string url = browser->GetMainFrame()->GetURL().ToString();
    if (IsChatGPTURL(url)) tab->url = url;
  }
  UpdateTabButton(*tab);
  if (!is_loading) SaveSessionState();
}

bool AppController::IsChatGPTURL(const std::string& url) const {
  return IsHttpsHost(url, "chatgpt.com");
}

bool AppController::IsInternalNavigationURL(const std::string& url) const {
  return IsChatGPTURL(url) || IsHttpsHost(url, "auth.openai.com");
}

bool AppController::HandlePopupURL(const std::string& url) {
  CEF_REQUIRE_UI_THREAD();
  if (url.empty()) return true;
  if (IsInternalNavigationURL(url)) NewTab(url);
  else OpenExternal(url);
  return true;
}

bool AppController::OpenExternal(const std::string& url) {
  if (!(StartsWith(url, "https://") || StartsWith(url, "http://"))) return false;
  pid_t pid = fork();
  if (pid < 0) return false;
  if (pid == 0) {
    pid_t grandchild = fork();
    if (grandchild == 0) {
      setsid();
      execlp("xdg-open", "xdg-open", url.c_str(), static_cast<char*>(nullptr));
      _exit(127);
    }
    _exit(grandchild < 0 ? 127 : 0);
  }
  int status = 0;
  while (waitpid(pid, &status, 0) < 0 && errno == EINTR) {}
  return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

void AppController::OnWindowDestroyed() {
  CEF_REQUIRE_UI_THREAD();
  LOG(WARNING) << "CGWA_SHUTDOWN WINDOW_DESTROYED";
  tabs_.clear();
  window_ = nullptr;
  header_ = nullptr;
  tab_strip_ = nullptr;
  content_ = nullptr;
  window_layout_ = nullptr;
  header_layout_ = nullptr;
  tab_layout_ = nullptr;
  window_created_ = false;
  CefQuitMessageLoop();
}

std::string AppController::ShortTitle(const std::string& title) const {
  std::string s = title.empty() ? "ChatGPT" : title;
  const std::string suffix1 = " - ChatGPT";
  if (s.size() > suffix1.size() &&
      s.compare(s.size() - suffix1.size(), suffix1.size(), suffix1) == 0)
    s.resize(s.size() - suffix1.size());
  return Utf8Ellipsize(s, 28, 25);
}
