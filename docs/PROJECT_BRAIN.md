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
