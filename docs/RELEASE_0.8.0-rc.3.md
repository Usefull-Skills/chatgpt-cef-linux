# ChatGPT CEF v0.8.0-rc.3 — Windows private-file guard preview

Date: 2026-10-04

## Scope

This prerelease candidate extends the qualified v0.8.0-rc.2 cross-platform native runtime with a native Windows owner-private file guard for Commander companion observation and binding files.

### Added

- Native Windows private-file reader using handle-based canonical identity checks.
- NTFS and persistent-ACL requirement.
- Current-user ownership validation.
- DACL allow-list limited to current user, LocalSystem and Administrators.
- Reparse-point and hardlink rejection.
- Bounded immutable snapshot enumeration and stable before/after identity verification.
- Windows CI fixture that proves valid owner-private reads and rejects a deliberately broadened Everyone ACL.

## Preserved boundaries

- CEF remains read-only toward Commander.
- No command dispatch, model invocation, credential extraction or browser-origin bridge is added.
- A profile that cannot prove the private-file security contract fails closed.
- Linux behavior remains unchanged.

## Promotion boundary

This remains a prerelease candidate. Stable v0.8.0 is still blocked on real Commander↔CEF producer/ownership integration, authenticated current-session validation, sustained non-interference testing, and exact release acceptance on both Windows and Linux.
