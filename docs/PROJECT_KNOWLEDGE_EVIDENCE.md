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


## 2026-10-05 — Remote Commander Browser rc.3 reconciliation evidence

- **Authority before change:** CEF stable `v0.7.1`; main `8689d4621b6fac77dbd087a4c385a3c34311100b`; Windows development PR #56 head `d20d2d64cc2e6ce8cbdfa30e67508a03c91aae58`.
- **Branch topology:** PR #56 was 48 commits ahead and 6 behind main, with merge base `d1a697422e0021985daa901b893947aaaea779c6`. A clean merge reproduced one conflict only: `docs/PROJECT_BRAIN.md`. No product source/build conflict occurred.
- **Historical exact-head CI:** `d20d2d64...` hosted Windows and Linux jobs both passed. Gates included static QA, companion contract, Linux native build/lifecycle, Windows native build, Windows private-file ACL guard, Windows lifecycle and Windows packaging.
- **Independent Windows rc.3 source test:** on an isolated Saeid clone, the pinned CEF Windows archive was reused only after exact size **172827308** and SHA-256 `aa1f7ab28005307edcc13f95e3ce5cd9d221894b00458fb5e368c00603204a2a` verification.
- **Windows CEF-free gate:** Visual Studio 2022 Build Tools configured successfully; companion contract built and CTest passed **1/1**.
- **Windows native build:** exact local rc.3 source built `chatgpt-cef-v2.exe` and `windows-private-file-test.exe` successfully using the pinned CEF dependency.
- **Windows private-file security gate:** private owner/SYSTEM/Administrators ACL fixture passed; a fresh Everyone-readable fixture was rejected. Result: `WINDOWS_PRIVATE_GUARD_PASS` + `WINDOWS_PRIVATE_GUARD_REJECT_PASS`.
- **Saeid connection boundary:** the Saeid Commander tunnel stopped polling before the planned local lifecycle/UI run. No tunnel restart, kill, Commander cutover or blind retry was performed. Native lifecycle/UI evidence for the modified rc.3 tree therefore remains OPEN until hosted Windows CI or a safe reconnected GUI host proves it.
- **Linux host qualification boundary:** Linux source hygiene, Bash syntax and project static QA passed. The current laptop lacks `cmake`, `ninja` and a C++ compiler, so local native build is **NOT RUN / HOST TOOLCHAIN MISSING**, not a product failure. Hosted Ubuntu exact-head qualification is required.
- **Branding decision:** product becomes **Remote Commander Browser**; target repository name is `remote-commander-browser`. Legacy binary/profile/WM-class identifiers remain compatible through rc.3 to avoid implicit authenticated-session migration.
- **Commander evidence boundary:** Saeid's separate Codex line passed direct DOM and root-WebSocket renderer-constant proofs, but both receipts explicitly remain `whole=NOT_FINAL`. No autonomous/test-chat acceptance is inferred.
- **Status:** rc.3 source candidate is locally reconciled and partially qualified; stable `v0.7.1` remains rollback authority pending exact-head hosted Windows/Linux and remaining E2E/UI gates.
