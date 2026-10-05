# ChatGPT CEF v0.8.0-rc.2 — Windows native preview

Date: 2026-10-03

## Scope

This prerelease extends the exact v0.8.0-rc.1 Linux preview baseline with a native Windows CEF build path while preserving the Linux/X11 implementation.

### Added

- Windows-native CEF entry point using the official CEF sandbox path when available.
- Windows profile/session-state paths and atomic state replacement.
- Windows external URL dispatch through `ShellExecuteW`.
- Pinned official CEF Windows x64 minimal archive:
  - bytes: `172827308`
  - SHA-256: `aa1f7ab28005307edcc13f95e3ce5cd9d221894b00458fb5e368c00603204a2a`
- Reproducible PowerShell fetch/build/package scripts.
- Hosted Windows native lifecycle qualification.

## Security boundary

The Commander companion private-file guard remains Linux-qualified only. On Windows the UI is deliberately fail-closed with `WINDOWS_PRIVATE_FILE_GUARD_UNAVAILABLE`; it does not read Commander snapshot/binding files through a weaker fallback. This preview therefore proves the Windows CEF application/runtime path, not Windows Commander-companion integration.

## Promotion boundary

This remains a prerelease. Stable v0.7.1 is unchanged. Stable v0.8.0 remains blocked on a native Windows private-file/ACL guard, real Commander↔CEF transport/ownership integration, authenticated current-session validation, and sustained non-interference testing.
