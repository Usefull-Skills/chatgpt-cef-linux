# Browser v0.8.2-rc.1 — DRAFT CANDIDATE, NOT RELEASED

Source parent: Browser integration PR #76 at `b026b66e22e7f3e4c3d9bbaee610968554dcaf63`.
Accepted rollback authority remains Browser v0.8.1 (`39258bc8e143aa666465e0ad63c4f277b075fcf3`).

## Objective
Prevent the new STOP+adaptive-theme Browser candidate from impersonating installed v0.8.1. This change aligns VERSION, Windows CI artifact references, Inno Setup's source fallback, and release-integrity test fixtures to the **preview** identity `0.8.2-rc.1`. Dynamic Linux and Windows packaging script names derive from VERSION; do not rewrite historical v0.8.1 release docs.

## Security / unresolved gates
- Core combined STOP + mutex GitHub commit and independent CI remains blocked; do not auto-promote.
- Need owner-verified two native STOP clicks, private GUI_STOP and Core denial on isolated Windows and GNOME desktops. Preserve sandbox/ACL; no login/profile changes.
- Verify setup and archive SHA256 from the exact current head, no same-version overwrite, and confirm rollback to immutable v0.8.1.
- Browser v0.8.1 was intentionally removed from Saeid Linux by an independent user-authorized storage cleanup. This candidate must not silently reinstall it or recreate deleted project roots.
- Release tags, distribution, auto-install and merge remain BLOCKED. PR must remain Draft.

## Acceptance
Hosted Windows-native and Linux Build workflows must succeed on the new commit and produce distinct candidate archive identities. Product FINAL is UNPROVEN until the above independent gates close.
