# Native Browser Navigation R1 — Candidate, Not Released
Date: 2026-10-09. Parent Browser PR #76 SHA 495048feeac78ca1da4693e0906d9c14f031e851.
Development workspace: C:\Users\Aa.Emad\source\repos\RC_BROWSER_NAV_R1. Not a production installer.

## Objective / DoD
Modern native, accessible browser navigation. No expansion of webpage-to-Commander authority or mutation of user profiles.

## Candidate functionality
- Separate 42px native CEF Views toolbar below tabs.
- Back/Forward gated by real browser history; Reload/Stop gated by loading; Home only https://chatgpt.com/.
- Native site identity only displays exact trusted ChatGPT/OpenAI auth origins; no arbitrary omnibox, URL-path disclosure, or web-to-OS bridge.
- Keyboard Alt+Left/Right, Ctrl+R, F5, Alt+Home; tooltips/accessibility names; tab, theme and fullscreen integration.
- STOP, companion, URL allowlist, session persistence and profile controls unchanged.

## Evidence and observed incidents
- Windows native Release CMake/Ninja/MSVC build PASS; candidate exe SHA256 8827d4bd3bb3e49712bd875d97298d10106e56539d842da1bf0b8bdea51946e0.
- 12 Python cross-platform source-contract tests PASS.
- Windows native private-file positive PASS; world-readable ACL negative rejection PASS; test-fixture ACL rollback PASS.
- Initial diff check found three newly-added CRLF lines in mixed-ending header; corrected and diff check PASS.
- CTest in full CEF configuration found no tests; this is not a test pass; run explicit tests.
- No production install, session/profile access, merge, tag or release.

## Open release gates
1. Exact commit hosted Windows/Linux CI, CEF test builds, verified native GUI on isolated profile.
2. Compatible Core STOP + Mutex release: Core #141 and #145 still Draft.
3. Authenticated Control Center profile/task CRUD; Smart Workflow runner/checkpoints/chat receipt.
4. Cross-platform versioned installer, SHA, signature, canary, rollback, release and cumulative main Brain.

## Classification and next action
DEVELOPMENT + WINDOWS NATIVE BUILD VERIFICATION, NOT FINAL. Push independent draft child PR for CI; never promote PR #76 or touch Browser v0.8.1 accepted installs before all gates pass.
