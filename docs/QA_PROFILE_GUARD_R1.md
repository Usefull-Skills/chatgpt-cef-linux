# Browser QA Profile Isolation Guard R1 — Candidate, Not Released
Date: 2026-10-09. Parent Browser native navigation PR #79 head `988404efb4b44d90866a92477221bdda4ab32a5d`.
Branch: `fix/browser-qa-profile-isolation-r1`. No merge, tag, general install or production profile promotion.

## Problem / Incident / Actual Evidence
On Saeid Windows, an exploratory native Browser self-test used separate argv items `--cgwa-profile-dir` then a separate filesystem path argument instead of the required `--cgwa-profile-dir=<absolute-path>` form. The CEF command-line parser accepts switch values in the `--key=value` form. Existing code silently returned when `GetSwitchValue("cgwa-profile-dir")` was empty. That allowed fallback to `%LOCALAPPDATA%\chatgpt-cef-v2`. The original user's profile `Default\History`, `Default\Preferences`, `Local State`, and `tabs.state` changed around 2026-10-09 10:13:21 UTC; QA_PROFILE_R1 stayed empty. The sanitized original `chrome_debug.log` on the default root has process 38188 with `CGWA_SELFTEST PASS` and `CGWA_SHUTDOWN FINALIZE` plus `CGWA_COMPANION_SELFTEST FAIL`. Thus the original QA run was **NOT isolated**, regardless of zero process exit code.

No content of cookies, passwords, saved sessions or the private profile was read/copied. No evidence proves sign-out or credential loss, but the claim that the original QA did not touch the profile is superseded. Do not attempt automatic restoration or re-run that old QA script.

**Failure ID:** `CEF-QA-UNISOLATED-PROFILE-20261009-R1`. Class: implementation+test-harness safety. Phase: isolated Browser runtime QA. Trigger: split-value CLI. Root cause: empty switch value interpreted as absent and silently ignored. Recurrence: first confirmed; prevent future occurrence by binary fail-closed protection and test runner exact syntax.

## Main mutation / one objective
Only `src/main.cc` entry preflight was changed:
- Any self-test/background-test/companion-self-test/shutdown-self-test requires a valid explicit `--cgwa-profile-dir=<absolute-path>`.
- Empty/missing/relative profile arguments fail early with code `30`, before `CefExecuteProcess`, `CefInitialize`, Browser GUI or default-cache writes.
- Canonicalized paths equal to or descendants of the original live profile, including symlink/reparse aliases, are rejected.
- Existing production runs without any QA switches retain original profile selection.
- Environment override failures also reject early instead of falling back.

## Verification evidence
- Six new source QA guard contracts were run *before* modification: 6/6 FAILED as intended. After mutation: 6/6 PASS.
- Original native navigation/STOP source contracts: 12/12 PASS.
- MSVC incremental native Windows CEF compile PASS; exact guarded executable SHA256 `f2d899121349f0b3c4450c3ba17def8a9aa1630e102fd5d81c7c18a84e88b112`.
- Emad Windows negative: four malformed QA invocation forms each rejected with exit code 30 before CEF; original profile metadata unchanged (receipt SHA256 `5bf90e162a89e8a5b80ab24aeb862d128b8c1ab99e3f5f20efbd003c5cc9ec2e`).
- Emad Windows positive: isolated absolute profile, authenticated session not reused, tab lifecycle self-test+shutdown/window destroy PASS, original profile metadata unchanged (receipt SHA256 `595835ee8e3ca8f0795343b10b0f09bc816431b6bce796532baabcce40ea7c73`).
- Exact executable transferred to Saeid Windows with matching SHA. Saeid R2 native runtime: malformed form rejected code 30, genuine isolated profile created, tab lifecycle+shutdown/window destroy PASS, stable Browser v0.8.1 binary and original profile metadata unchanged (receipt SHA256 `3210184e56b1d78b7fda5813202d3ecf971b56a0788386affeb3845273a5f40c`).
- Windows production installs on Emad/Saeid untouched. **Linux native runtime / hosted CI of this new fix NOT YET accepted.** GUI screenshot/STOP on owner desktop and signed release also remain OPEN.

## Security and regression restrictions
Do not run `RC_BROWSER_NAV_SAEID_R1/RUN_QA_R1.ps1` or the old `chatgpt-cef-v2-pr79.exe` self-test. They are **SUPERSEDED—DO NOT RUN**. Treat exit 0 alone as UNPROVEN, and never use `--no-sandbox`, sign out a browser, reset user profiles, merge a Draft PR, or auto-clear GUI uncertainty. Keep all historical evidence and original profile contents intact.

## Next action
Push a Draft child PR from this branch to `feat/browser-native-navigation-r1`, require new-head Windows/Linux hosted CI and cppcheck. Verify at least one successful isolated Linux CEF runtime with sandbox enabled and authenticated owner GUI screenshot STOP, then address staged product release and four-host rollback. If any of these cannot be proven, keep DRAFT/BLOCKED.
