# Project Knowledge / Evidence — ChatGPT CEF Linux

## 2026-10-03 — Saeid development preview publication

- **Context:** version Saeid's CEF/companion development without falsely promoting unfinished Windows/end-to-end behavior.
- **Decision:** keep stable main/runtime at v0.7.1 and publish the exact candidate as Linux-only prerelease v0.8.0-rc.1.
- **Evidence / source:** exact candidate `1581c8012dbbb08b88517e01aa5e4f4ab2eb97f3`; publisher workflow rebuilt the candidate, ran static QA, fetched pinned CEF, built native release, verified sandbox ownership/mode, exercised companion/self/shutdown lifecycle under Xvfb, packaged assets, verified SHA256SUMS, then published.
- **Result:** workflow completed SUCCESS. Annotated tag points to the exact candidate. GitHub prerelease is draft=false / prerelease=true.
- **Asset provenance:** Linux x86_64 binary digest `781e6eaf19317c40b2566f5630a7415fe39ed5d6460117e50fdd6564e15d2f66`; source archive digest `58e1067b29e5c712af677d47382890f722cf66fbf432a35d7549a10c67484865`; checksum-file asset digest `684994107e226d38a077257973fffb7bd27712c374082b615a94d2fc9160b781`.
- **Confidence/status:** CONFIRMED / HIGH for Linux preview publication; UNPROVEN for Windows and full Commander↔CEF integration.
- **Limitation:** annotated tag is unsigned; no signed-tag claim is made.
- **Rejected option:** merging the RC candidate into stable main before Windows/end-to-end evidence. Rejected to preserve stable v0.7.1 and avoid universal-update claims.
- **Superseded state:** PR #51/#52 closed after preservation/versioning; Git history retained.
- **Reuse targets:** Windows port, release runbook, Commander integration, final v0.8.0 acceptance.

## 2026-10-05 — rc.3 Windows qualification and branding

- Claim/Decision: Adopt Remote Commander Browser as user-facing product identity while preserving legacy stateful identifiers for rc.3 compatibility.
- Evidence: reconciled isolated clone; git merge code auto-resolved with only PROJECT_BRAIN.md conflict; local Windows build completed with WINDOWS_BUILD_PASS.
- Root Cause: user environment exported CC/CXX to LLVM clang; CEF Windows CMake assumes MSVC-style flags. build-windows.ps1 now selects VS cl.exe explicitly and removes inherited CC/CXX.
- Regression: second build identified MSVC 19.51.36260.0 and completed 224/224; windows-private-file-test safe path PASS and broad ACL reject PASS; native lifecycle PASS; package PASS.
- Artifact SHA-256: 81b2e5e1683197bebdc51c56cf1e6efbf74f070d9ce1248f24666f405da27ec1.
- Confidence/Status: Confirmed / Candidate-qualified on local Windows host. Hosted and Linux gates still required.

## 2026-10-05 — Immutable tag collision prevention

- Evidence: refs/tags/v0.8.0-rc.3 peels to 6064c0693f917ad40211715cd4500249053a48fd, while reconciled main is 1afeff5276927ba4eeae0e003b8fffa852affef2.
- Decision: preserve the historical tag and advance the release line to v0.8.0-rc.4.
- Prevention: release automation must verify remote tag absence/peel before tag creation and must never force-update published tags.
- Status: Confirmed.

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
