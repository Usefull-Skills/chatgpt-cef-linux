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

### 2026-10-06 — Windows standalone installer and session-safe rc.5 qualification

- **Objective:** make Remote Commander Browser installable as one standalone Windows Setup and as an optional Commander component without mutating the authenticated Chromium profile.
- **Delta:** Windows CEF extraction no longer requires 7-Zip; `tar.exe` is the built-in fallback. `scripts/install-windows.ps1` accepts explicit source/version/install/companion paths and a QA-only `-NoPublicIntegration` mode while preserving production defaults. The standalone Inno Setup embeds the native payload, verified PowerShell prerequisite bootstrap, installer script and branding.
- **Security/session evidence:** isolated script-level install and the actual Setup EXE both completed with protected companion ACL and exact product manifest. Fingerprints for production `Local State`, `Default\\Network\\Cookies`, and `Default\\Login Data` remained unchanged. No authenticated profile was launched or copied during qualification.
- **Local rc.5 qualification:** CEF-free companion contract 1/1 PASS; Windows private-file guard PASS; native lifecycle self-test PASS; Windows ZIP package PASS; standalone Setup compile PASS; silent isolated Setup acceptance PASS. Native exe SHA-256 `1b43d6ef084c24baa42566df8e1910f62dff665b0717fb3f2a0e3f587b706e34`; rc.5 Windows ZIP SHA-256 `2e91ba31a316469b833cfc50ea472b9f8a8bfc0bd6f47fb435686d5888a5abbe`; standalone Setup SHA-256 `86162caa43a6ad265b677636072badaf49311b9abe476188d7943c76a592288f`.
- **CI prevention:** Windows workflow now builds the Setup and performs a silent isolated install with manifest and ACL readback. Windows and Linux native gates remain required on exact head before promotion.
- **Signing status:** no valid local Code Signing certificate with private key is installed. The Setup is therefore `NotSigned`; self-signing is not treated as a substitute for public publisher trust. This is an **EXTERNAL/MISSING signing authority**, not a hidden PASS.
- **Status:** local implementation = CONFIRMED PASS; hosted exact-head CI/release = OPEN.

### 2026-10-06 — Hosted Windows Setup blocker narrowed to child installer exit

- **Context:** PR #65 exact head `a324b8c` reran Windows/Linux hosted gates after adding Setup diagnostics. Linux regression, Linux release build, and companion contract passed; Windows native failed only in the silent isolated Setup acceptance stage.
- **Confirmed evidence:** Inno Setup compiled the rc.5 installer successfully. The Setup log shows all custom parameters (`-InstallRoot`, `-CompanionRoot`, `-NoPublicIntegration`) were passed exactly to `install-windows.ps1`. The PowerShell child exited with code **1**; both QA and default install roots remained absent. Inno itself returned success despite the child failure, so Setup fail-closed semantics are also an open defect.
- **Method evidence:** current Inno Setup documentation/revision history explicitly provides the `logoutput` flag and `ExecAndLogOutput` for capturing child process output in Setup logs. The minimum next control is therefore to enable `logoutput` on the existing [Run] entry and rerun the same hosted gate before changing ACL/install logic.
- **Status:** root cause of the child exit remains **UNVERIFIED**; parameter propagation is **CONFIRMED GOOD**. Browser rc.5 remains blocked from merge/release.
- **Reuse targets:** Windows installer runbook, CI failure-prevention, release checklist.

### 2026-10-06 — Hosted Windows Browser Setup root cause confirmed and fail-closed repair

- **Confirmed root cause:** exact-head hosted log with child-output capture shows `install-windows.ps1` exits 1 at `BROWSER_COMPANION_ACL_OWNER_MISMATCH`. Custom Inno parameters are correct; the failure occurs because the hosted QA directory inherits an owner different from the current runner identity, while the native Windows private-file guard intentionally requires owner == current user.
- **Repair:** preserve the strict native owner invariant. When the installer detects an owner mismatch, it uses Windows `icacls /setowner` with the current identity, verifies the resulting owner SID, then applies the existing inheritance-removal/current-user+SYSTEM+Administrators full-control DACL and performs the existing private-ACL readback. No broad principal is added.
- **Fail-closed Setup:** the embedded PowerShell installer is no longer a [Run] entry whose nonzero result can be masked. Inno Pascal `ExecAndLogOutput` now runs it during `ssPostInstall`, logs child output, and raises a fatal Setup exception when the child exit code is nonzero.
- **Evidence hierarchy:** Microsoft documents `icacls /setowner`; current Inno Setup documents `logoutput`/`ExecAndLogOutput`. Hosted rerun on the repaired exact head remains required before promotion.
- **Status:** local code repair = PROPOSAL pending parser/build/isolated-install regression and hosted Windows PASS. Browser rc.5 remains UNPROVEN for release.

### 2026-10-06 — Local Browser Setup owner-repair + fail-closed regression PASS

- **Exact artifact:** `Remote-Commander-Browser-Setup-v0.8.0-rc.5.exe`, SHA-256 `96793f036d62fe3ccad5dec2065e02c1dd97ecb6996d6e737fce01234ddcf396`.
- **Success-path evidence:** isolated silent Setup returned 0; installed `0.8.0-rc.5`; `publicIntegration=false`; companion root ACL inheritance is protected.
- **Failure-path evidence:** a deterministic invalid InstallRoot (regular file instead of directory) forced the embedded helper to fail. Setup returned custom exit code **200** via `GetCustomSetupExitCode`; no `product-install.json` was promoted under the bad root. This proves child failure is no longer masked as Setup success.
- **Status:** local Windows regression PASS. Hosted Windows exact-head PASS remains required before merge/release. Login/profile state was not touched by these isolated QA roots.
- **Reuse targets:** Browser release evidence, Commander bundled-browser contract, Windows installer regression suite.

### 2026-10-06 — Browser Windows ACL exact-writer follow-up

- Hosted head `93b98eb` proved two things: Setup fail-closed propagation now works (Setup exits 200 when child fails), and owner repair progresses past the previous owner mismatch. The remaining hosted failure is `BROWSER_COMPANION_ACL_VERIFY_FAILED`.
- Local script-level acceptance with the proposed exact `.NET DirectorySecurity` writer passed: owner=current user, inheritance protected, and exactly three explicit FullControl ACEs for current-user SID, SYSTEM and BUILTIN\Administrators.
- The DACL writer now creates a fresh protected `DirectorySecurity` rather than relying on cross-host `icacls /grant:r` normalization. `icacls` remains limited to owner repair. If verification still fails, the installer emits structured owner/protected/rule diagnostics before failing.
- Comparison research: official Remote Desktop Commander emphasizes OAuth/device pairing, file/terminal/process/session operations; Microsoft Playwright MCP explicitly models persistent vs isolated browser profiles and exclusive profile ownership. Current Remote Commander already has per-profile capability authority, durable workflows and isolated profile state; the release-critical gap is Windows Browser installation determinism rather than additional capability breadth.
- Status: exact-DACL repair locally PASS at script level; Setup artifact rebuild and hosted Windows rerun are required before Browser rc.5 merge/release.

### 2026-10-06 — Browser rc.5 exact-DACL local Setup acceptance

- **Local Setup artifact:** rebuilt `Remote-Commander-Browser-Setup-v0.8.0-rc.5.exe` with SHA-256 `a0a96b9059d2467b1eb669041b33b6f3c02cc7254ed32353374b0e01eee40c88`.
- **Acceptance:** silent isolated install through the actual Setup EXE completed exit 0; `product-install.json` reports `0.8.0-rc.5`, `publicIntegration=false`; companion directory readback shows protected ACL with exactly three explicit rules. Authenticated production profile/login state was not used or mutated.
- **Fail-closed evidence:** previous hosted head already proved the unchanged Inno fail-closed path by returning Setup exit 200 when the embedded installer failed. The only code delta since then is deterministic DACL writing/diagnostics.
- **Promotion gate:** hosted Windows exact-head must pass owner repair + exact DACL + Setup acceptance before merge/release.

### 2026-10-06 — Browser rc.5 release-note checksum authority correction

- **Issue:** pre-release notes carried hashes from an earlier local build. After the final ACL repair, those bytes are no longer release-authoritative and hosted/tag builds may differ from local qualification builds.
- **Decision:** remove hard-coded local artifact hashes from release notes. The immutable tag release workflow rebuilds artifacts, creates and verifies `SHA256SUMS.txt`, and that published checksum file is the only release artifact identity. Local hashes remain qualification evidence only.
- **Reason:** prevents stale provenance from being published while preserving reproducible tag→workflow→checksum authority.
- **Status:** documentation-only delta; exact-head CI remains required because release metadata is part of the product release contract.

### 2026-10-06 — rc.5 release attempt failed at mutable compiler feed; rc.6 provenance decision

- **rc.5 product qualification:** exact PR head `6ecba4a0079cecabc564f1526a2b8b0981ea077e` passed companion contract, hosted Windows native lifecycle/private ACL/standalone Setup/silent isolated Setup acceptance, Linux regression, and Linux release build. Merge commit `f34f01299b3ba2308c097ec7e93fd6bbdb95202f` has the same tree.
- **Release failure — CONFIRMED infrastructure only:** tag `v0.8.0-rc.5` triggered Release run `37433257939`. Linux release job passed; Windows stopped before product build because Chocolatey no longer served `innosetup 6.7.3`. Publish was skipped, so no rc.5 GitHub Release was created.
- **Provenance decision:** never move or overwrite the rc.5 tag. rc.5 remains an immutable failed release attempt. Advance to rc.6 with the same qualified Browser runtime plus a release-infrastructure correction.
- **Official compiler evidence:** `jrsoftware/issrc` release `is-6_7_3` exposes `innosetup-6.7.3.exe` with release-asset SHA-256 `9c73c3bae7ed48d44112a0f48e66742c00090bdb5bef71d9d3c056c66e97b732`.
- **Independent Windows audit:** downloaded that exact asset, SHA matched, Authenticode was `Valid`, signer was `Pyrsys B.V.`, silent portable install succeeded, and resulting `ISCC.exe` executed as Inno Setup 6 compiler.
- **Prevention:** release workflow now uses that immutable official asset, checks exact SHA-256, valid Authenticode and expected signer before installing it. Chocolatey is removed from Browser release compiler provisioning.
- **Open gate:** rc.6 exact-head hosted Windows/Linux qualification, merge-tree equivalence, immutable tag, Release workflow PASS, and published release checksum verification.

### 2026-10-06 — Browser rc.6 local qualification after compiler-bootstrap repair

- **Pinned compiler script:** `scripts/install-inno-setup.ps1` is the single provisioning path used by both hosted PR Windows qualification and tag Release Windows build. It downloads only the official `jrsoftware/issrc is-6_7_3` installer, enforces SHA-256 `9c73c3bae7ed48d44112a0f48e66742c00090bdb5bef71d9d3c056c66e97b732`, requires valid Authenticode and expected signer `Pyrsys B.V.`, then performs a no-restart portable install.
- **Script-level acceptance:** isolated `install-inno-setup.ps1 -InstallDir <temp>` returned `INNO_SETUP_PIN_PASS`; compiler existed and was removed with the test root.
- **rc.6 Setup build:** `Remote-Commander-Browser-Setup-v0.8.0-rc.6.exe` built successfully from the already-qualified Windows native payload; local qualification SHA-256 `73af9d4ccc87fca422a26aa1e3830a836b908b209d6cdb633a393d68c958e70c`; product Setup signing remains `NotSigned`.
- **Actual Setup acceptance:** silent isolated install returned 0; product record version `0.8.0-rc.6`, `publicIntegration=false`; companion ACL protected with exactly three expected explicit rules; no authenticated production browser profile was used or changed.
- **Static QA:** PASS after rc.6 metadata/workflow update.
- **Promotion gate:** exact-head hosted Windows must independently exercise the pinned compiler script + native build + Setup build + silent isolated Setup; Linux regression/release-build must also PASS.

### 2026-10-06 — rc.6 hosted Windows compiler bootstrap failure: path quoting

- **Occurrence:** both push and PR Windows jobs on exact head `e580f99` failed at the new shared compiler bootstrap; native build, ACL guard, lifecycle and Windows ZIP packaging had already passed. Linux regression/release-build and companion contract passed.
- **Root cause — CONFIRMED:** official Inno installer download/hash/Authenticode checks succeeded and installer returned success, but `ISCC.exe` was absent from the intended default directory. The default target `%LOCALAPPDATA%\Programs\Inno Setup 6` contains spaces, while `Start-Process -ArgumentList` was given an unquoted `/DIR=<path>`. Earlier local isolated acceptance used a no-space temp path and therefore did not expose this.
- **Fix/prevention:** pass the Inno `/DIR` value with embedded quotes. Keep the hosted PR gate using the real default path so this exact quoting contract is permanently exercised before tagging.
- **Post-fix local acceptance:** running `scripts/install-inno-setup.ps1` with its real default space-containing path returned `INNO_SETUP_PIN_PASS`, exact SHA-256 `9c73c3bae7ed48d44112a0f48e66742c00090bdb5bef71d9d3c056c66e97b732`, valid Pyrsys B.V. signer, and existing `ISCC.exe` at the expected path.
- **Status:** local PASS; fresh hosted exact-head Windows required, no blind rerun of the failed head.

### 2026-10-06 — rc.6 Release succeeded but omitted standalone Setup; rc.7 layout correction

- **Observed release:** `v0.8.0-rc.6` Release workflow run `37441230302` completed SUCCESS and published release id `404522059`. Published assets were Linux tar + checksum, Windows ZIP + checksum, and aggregate `SHA256SUMS.txt`; the standalone Windows Setup EXE was **missing**.
- **Root cause — CONFIRMED by workflow + release asset list:** Windows artifact upload included `dist/installer/Remote-Commander-Browser-Setup-v*.exe`, preserving the `installer/` subdirectory. Publish enumerated only `dist/release` files at `maxdepth=1`, so the nested Setup was excluded without failing. The release itself was therefore incomplete for Commander embedding even though product build gates passed.
- **Provenance decision:** do not move rc.6 tag and do not retrofit it as the authoritative Commander dependency. Advance to rc.7 with unchanged Browser runtime and a release-layout-only fix.
- **Repair:** copy the already-built Setup to `dist/Remote-Commander-Browser-Setup-v<version>.exe` before artifact upload; upload that top-level file; and add a publish-time assertion requiring the Setup at top level before checksums/release creation.
- **Prevention:** future tag releases fail before publication if the standalone Setup is absent. This is the minimum sufficient control; no recursive-release flattening or broader artifact restructuring is added.
- **Status:** local workflow/source delta pending exact-head hosted Windows/Linux gates and rc.7 tag Release verification.

### 2026-10-06 — Browser rc.7 local release-layout qualification PASS

- **Workflow syntax:** `.github/workflows/release.yml` and `windows-native.yml` parsed successfully as YAML; `git diff --check` PASS.
- **Actual Windows Setup build:** Inno Setup 6.7.3 compiled `Remote-Commander-Browser-Setup-v0.8.0-rc.7.exe` successfully from the already-qualified native payload. Local SHA-256: `4f21ca40b595637083342765de070e7e524ee5ecd80da9e097bf8048359fb181`; size `136261367` bytes; Authenticode status remains `NotSigned` (external signing gate unchanged).
- **Release-layout acceptance:** copied the built Setup from `dist/installer/` to the exact top-level release path `dist/Remote-Commander-Browser-Setup-v0.8.0-rc.7.exe`; source and release copy SHA-256 values matched exactly. This validates the narrow rc.7 mutation locally.
- **Proportional rigor:** no redundant local silent-install rerun was added because Browser runtime/installer semantics are unchanged from rc.6 and exact-head hosted Windows permanently performs the real standalone Setup build + isolated silent-install acceptance. The new release-layout contract is independently guarded by the top-level publish assertion.
- **Status:** local rc.7 release-layout PASS; hosted exact-head Windows + Linux/Build remain the promotion gate.

### 2026-10-06 — third release-infrastructure failure triggers release-path redesign (rc.8)

- **Trigger:** rc.7 exact-head PR gates PASS and tag Windows/Linux build jobs PASS, but tag Release run `37449262110` failed in publish. The Windows `upload-artifact` step reported only **2 files** despite four path patterns, and the new publish assertion correctly rejected the missing top-level Setup after download.
- **Failure family history:** rc.5 mutable Chocolatey compiler feed; rc.6 unquoted portable Inno target path; rc.7 release artifact layout. This is the third meaningful failure in the same release-infrastructure family, so further isolated patching was stopped.
- **External method evidence:** official `actions/upload-artifact` documentation states wildcard hierarchy is preserved after the first wildcard and multiple paths use their least common ancestor as artifact root; official migration/download guidance documents `merge-multiple` semantics. Source: `https://github.com/actions/upload-artifact` README and migration docs.
- **Root-cause class:** release correctness depended on implicit artifact path/glob/root behavior that was not exercised before tagging. Product runtime qualification remained green.
- **rc.8 design:** stage exactly four Windows release files in one dedicated directory, assert exact filename set/count, upload one staging pattern, and run the same Release Windows/Linux build jobs on release-related PRs while keeping publish tag-only.
- **Why this is minimum sufficient:** it removes implicit layout inference and moves the actual release implementation before promotion; no duplicate release workflow, recursive flattening, or extra runtime feature gate is introduced.
- **Status:** rc.8 implementation pending YAML/static validation and hosted exact-head PR release-build PASS.

### 2026-10-06 — rc.8 PR release gate first attempt: Windows directory wildcard rejected

- **Exact-head PR run:** Release run `37450317568` on `17a5dac` reached `WINDOWS_RELEASE_STAGE_PASS count=4`; Linux release job PASS. Windows failed only at `actions/upload-artifact` because `path: dist/release-windows/*` returned “No files were found”.
- **Interpretation:** file production/staging is proven; this is action path-selection behavior, not Browser build or Setup regression.
- **External evidence:** official upload-artifact usage explicitly supports uploading an entire directory by passing the directory path directly. The simpler contract avoids Windows wildcard matching entirely.
- **Fix:** use `path: dist/release-windows/` while retaining the preceding exact four-file filename/count guard. No runtime or artifact-content changes.
- **Status:** fresh exact-head Release PR run required; failed head is not rerun blindly.
