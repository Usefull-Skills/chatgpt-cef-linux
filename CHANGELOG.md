# CEF 0.8.0-rc.1 preview — 2026-10-02

Version the Saeid native companion and V03 source snapshots with explicit incomplete integration/platform gates. Stable automatic update is not enabled.

# Changelog

## v0.7.1 — 2026-10-01

### Added
- Modern minimal native shell with Vazirmatn-first Persian typography.
- Automatic Persian/Arabic RTL and English/code/math LTR handling.
- Exact-origin media/clipboard permission policy.
- Native multi-tab shell, keyboard accelerators and atomic tab-session persistence.
- Single-instance relaunch behavior.
- Isolated background self-test and staged-shutdown self-test support.

### Hardened
- Release build validated with `-Wall -Wextra -Wpedantic -Werror`.
- Project source passes configured `cppcheck` release scan.
- Fixed a 4 px native overlay offset mismatch introduced when the modern header height changed from 42 px to 38 px.
- Unified active-tab visual updates through one canonical update path.
- Removed a shadowed local variable and standardized tab lookup.
- Kept Linux Chromium sandbox enabled and validated.
- Kept permission trust checks boundary-safe and exact-origin based.

### Validated
- Login/session persistence
- file/image upload
- clipboard
- dictation/microphone
- Voice start/stop
- RTL/LTR
- windowed/maximized/fullscreen
- tab lifecycle and staged shutdown
- single-instance behavior
- GPU/renderer health
