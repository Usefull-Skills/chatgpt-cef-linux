System.Object[]


---

## Archived pre-reconcile feature brain snapshot

> Preserved verbatim for provenance during rc.3 reconciliation. Current authority is the main-derived content above.

System.Object[]
## 2026-10-05 — Remote Commander Browser rc.3 finalization

- Status: candidate, not yet stable.
- Reconciled feature/v0.8.0-rc.2-windows with current main in isolated finalization clone; only PROJECT_BRAIN.md conflicted and history was preserved append-only.
- Product identity selected: Remote Commander Browser; repository migration target: Usefull-Skills/remote-commander-browser.
- Compatibility boundary: legacy executable/profile/runtime identifiers chatgpt-cef-v2 remain unchanged in rc.3 to preserve login/session state.
- Windows build reproducibility root cause found: inherited CC/CXX pointed to system LLVM clang, causing CEF MSVC flags to be passed to GNU-mode clang. Prevention: build-windows.ps1 clears CC/CXX and pins CMAKE_C_COMPILER/CMAKE_CXX_COMPILER to Visual Studio cl.exe.
- Windows local gates on rc.3 candidate: native build PASS; private-file safe-read PASS; broad Everyone-read ACL rejection PASS; native lifecycle/self-test PASS; package PASS.
- Windows package: remote-commander-browser-v0.8.0-rc.3-windows-x86_64.zip; SHA-256 81b2e5e1683197bebdc51c56cf1e6efbf74f070d9ce1248f24666f405da27ec1.
- Remaining promotion gates: exact-head hosted Windows/Linux CI, Linux native lifecycle/package, real UI observation/input smoke, Commander integration/recovery/rollback acceptance.

## 2026-10-05 — Release-line correction to rc.4

- Confirmed remote tag v0.8.0-rc.3 already existed on historical commit 6064c0693f917ad40211715cd4500249053a48fd.
- Decision: never force or rewrite the existing tag. The reconciled Remote Commander Browser candidate advances to v0.8.0-rc.4.
- Prior rc.3 qualification evidence remains historical evidence for the same product tree lineage; rc.4 must receive fresh exact-head hosted CI before tagging/publishing.

## 2026-10-05 - rc.4 Windows branded-shell candidate

- Exact local head: 604401cd4444a20f64fda688e1893a630d225349.
- Change: Windows EXE icon/resource, per-user versioned installer + Start Menu registration, and separation of Commander companion trust-state from Chromium authenticated profile state.
- Safety rationale: browser cookies/login state are not cleared or migrated; companion trust data now defaults under ChatGPTRemoteCommander/browser-companion while CGWA_PROFILE_ROOT remains compatibility fallback for isolated tests.
- Exact-head local Windows native build: PASS (225/225).
- Exact-head Windows package: PASS; SHA-256 4869caaaf54ef2a9c8064874d44e9c9a26a38303102bf498beef0db616e3baa7.
- Promotion status: LOCAL QUALIFIED ONLY. Remote push/hosted exact-head Windows+Linux CI and real authenticated UI/Commander E2E remain OPEN.

## 2026-10-05 — Window drag / browser-engine architecture audit
- Context: user reported Remote Commander Browser opens centered and cannot be moved; requested deep review of browser-engine architecture and whether multiple engines should be combined.
- Fact/CONFIRMED: current Browser candidate uses CEF 154.0.32 + Chromium 154.0.8037.58. The shell is frameless and previously marked only the brand-button area as draggable; because that area is itself a clickable button, runtime movement was not reliable/usable.
- Method evidence: Chromium-based CEF already exposes Chromium multi-process isolation and DevTools/CDP capabilities. Current external evidence also shows WebView2 stores independent user data/profile state and that multi-profile WebView2 is its own runtime/profile architecture; adding it beside CEF would create another session/runtime test matrix rather than improve the existing CEF path. Playwright connectOverCDP is officially documented as lower fidelity than the native Playwright protocol connection; therefore Playwright/CDP is suitable as an optional control/test layer, not as a second rendering engine.
- Decision/PROBABLE: retain one rendering engine (CEF/Chromium) and add control/observability through CEF DevTools/CDP where needed. Do not add WebView2/Firefox/WebKit without a concrete compatibility requirement that CEF cannot satisfy.
- Change: branch fix/window-drag-cdp-20261005 adds a dedicated 72px non-interactive header drag handle and maps the frameless draggable region to its actual laid-out bounds.
- Verification: clean Windows native rebuild PASS, 225/225, output build-windows/bin/chatgpt-cef-v2.exe. First build failed because CefPanel has no SetTooltipText; root cause fixed by removing the invalid call; clean rebuild then PASS.
- Validation: real pointer-drag acceptance is still UNPROVEN because the shared GUI-control lease was unavailable during the acceptance step. Do not promote this patch as final until real window movement is observed.
- Safety: candidate launch used an isolated temporary browser profile; authenticated production browser state/cookies/password store were not touched.
- Reuse Targets: Browser release notes, architecture, window-shell QA, automation/control design.


## 2026-10-05 — Full-header drag + authenticated-session safety adjudication
- Context: user reported the frameless Browser could not be moved from the full top bar and observed a logged-out ChatGPT window during QA.
- Fact/CONFIRMED — logout appearance: the logged-out window was the deliberately isolated QA instance launched with `--cgwa-profile-dir=C:\Users\Aa.Emad\AppData\Local\Temp\rc-browser-*-20261005`, not the installed production profile. The installed Start Menu shortcut has no profile override. Production state remained at `C:\Users\Aa.Emad\AppData\Local\chatgpt-cef-v2`; `Default\Network\Cookies`, `Default\Login Data`, and `Local State` remained present and their timestamps predated the isolated header QA. No production cookie/password-store mutation was performed.
- Root cause/CONFIRMED — window movement: the initial patch exposed only a 72 px dedicated drag spacer, which did not satisfy normal browser-title-bar behavior. CEF documents `CefWindow::SetDraggableRegions` as window-coordinate mouse-interception regions, so the corrected implementation marks the full visible header draggable and subtracts only real clickable controls and occupied tab items.
- Verification/PASS: clean Windows rebuild completed 225/225 build steps with exit 0; reproducible native `WM_NCHITTEST` scan on an isolated background profile returned `HTCAPTION` across 65 samples (large blank header range x=275..905) while 52 control/tab samples remained `HTCLIENT`; native lifecycle/companion/shutdown self-test returned `WINDOWS_NATIVE_LIFECYCLE_PASS`.
- Packaging/PASS: `remote-commander-browser-v0.8.0-rc.4-windows-x86_64.zip` SHA-256 `f99a8b483378899b5b2014b8fbc22ec279cde65114b4854c1144e675d19decb2`.
- Engine decision/CONFIRMED method choice: keep CEF/Chromium as the single rendering engine. Current CEF headers expose in-process `ExecuteDevToolsMethod` / `AddDevToolsMessageObserver` without requiring a remote-debugging session. Chrome explicitly warns that remote-debugging has been abused to extract cookies and recommends non-default user-data directories; therefore the authenticated production profile must not expose a remote-debugging port/pipe. Playwright documents CDP attachment as significantly lower fidelity than native Playwright protocol. WebView2 documentation confirms separate UDF/profile runtime state and extra resource cost for multiple UDF/runtime instances. Adding WebView2/Firefox/WebKit would therefore expand session/compatibility/test matrices without a demonstrated product outcome.
- External method sources: https://developer.chrome.com/blog/remote-debugging-port ; https://playwright.dev/docs/api/class-browsertype ; https://learn.microsoft.com/en-us/microsoft-edge/webview2/concepts/user-data-folder ; https://learn.microsoft.com/en-us/microsoft-edge/webview2/concepts/multi-profile-support
- Decision: no second rendering engine. Prefer CEF-native in-process DevTools APIs for any future visible-browser instrumentation; keep Commander background automation isolated from the authenticated Browser profile. A second engine requires a concrete compatibility failure plus evidence that it resolves the failure.
- Confidence/Status: CONFIRMED/HIGH for local Windows drag geometry, lifecycle, package and production-profile separation. Hosted exact-head Windows+Linux CI and release promotion remain OPEN.
- Reuse Targets: Browser architecture, release notes, session-safety policy, Commander Browser integration, QA.

### 2026-10-06 — Browser rc.5 standalone installer candidate

**Previous accepted state:** main/rc.4 had session-safe native Browser and read-only Commander companion, but no official standalone Setup EXE and Windows build required 7-Zip when extracting CEF.

**Current delta:** rc.5 candidate builds a native Windows Browser, ZIP package and standalone Inno Setup; the actual Setup completed isolated silent install with private companion ACL and zero production session-file changes. Windows CI now includes Setup build/install acceptance. 7-Zip is optional because Windows tar.exe is supported.

**Evidence:** contract/private-file/native lifecycle/package/Setup gates all PASS locally. Setup SHA-256 = `86162caa43a6ad265b677636072badaf49311b9abe476188d7943c76a592288f`.

**Open gate:** exact-head hosted Windows + Linux CI, merge/release, and production upgrade/readback. Authenticode signing is MISSING/EXTERNAL because no valid Code Signing certificate is installed.

**Exact next action:** commit/push rc.5 installer candidate, require exact-head CI PASS, then merge/tag/release and perform session-preserving live upgrade/readback.
