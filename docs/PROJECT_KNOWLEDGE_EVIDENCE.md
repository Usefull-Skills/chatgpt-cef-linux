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
