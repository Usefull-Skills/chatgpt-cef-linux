# Project Brain — ChatGPT CEF Linux

Status: CURRENT — STABLE v0.7.1 / PREVIEW v0.8.0-rc.1 ACCEPTED FOR LINUX ONLY / CROSS-PLATFORM FINAL INCOMPLETE
Updated: 2026-10-03
Authority: immutable stable/preview tags and release assets -> exact-SHA CI/native lifecycle evidence -> this Brain/Evidence record.

## Final objective

Maintain a secure native CEF client and optional read-only Commander companion that can be qualified independently on each supported operating system, without conflating preview source publication with authenticated end-to-end autonomy.

## Accepted current state

### Stable channel

- Stable version: `v0.7.1`.
- Stable main runtime source remains `0.7.1`.
- Stable release commit: `d1a697422e0021985daa901b893947aaaea779c6`.

### Preview channel

- Preview: `v0.8.0-rc.1` — Linux-only prerelease.
- Exact preview source commit: `1581c8012dbbb08b88517e01aa5e4f4ab2eb97f3`.
- Exact candidate was rebuilt and native-qualified on Linux immediately before publication.
- Published assets:
  - Linux x86_64 binary SHA-256: `781e6eaf19317c40b2566f5630a7415fe39ed5d6460117e50fdd6564e15d2f66`
  - source archive SHA-256: `58e1067b29e5c712af677d47382890f722cf66fbf432a35d7549a10c67484865`
  - checksum-file asset digest: `684994107e226d38a077257973fffb7bd27712c374082b615a94d2fc9160b781`
- The annotated tag is unsigned; this is recorded evidence, not a signed-release claim.

## Scope boundary

The preview versions Saeid's native read-only companion and archived V03 monitor development. It does NOT prove:
- Windows native port/build/install;
- V03 native integration;
- production Commander exporter/private-file ownership;
- authenticated current-session binding;
- automatic continuation;
- sustained long-duration/non-interference behavior.

Stable `v0.7.1` remains the default/rollback channel. Preview publication does not change the stable runtime or migrate browser profiles.

## Roadmap ← CURRENT

1. Linux preview publication and native qualification — COMPLETED / ACCEPTED.
2. Windows native port/build/install — OPEN ← CURRENT CRITICAL PATH.
3. Commander↔CEF ownership/transport integration — OPEN.
4. Authenticated current-session validation — OPEN.
5. Sustained/fault/non-interference validation — OPEN.
6. Stable v0.8.0 promotion — BLOCKED until the above evidence exists.

## Superseded history

PR #51 and PR #52 are closed as superseded after the required source was versioned in the preview release. Their branches/history remain available. Closing them does not promote experimental gates.

## Exact next action

Use a reliably connected Windows host to implement/qualify the native Windows build/install path from the exact `v0.8.0-rc.1` preview baseline. Do not promote to stable or wire autonomous Commander transport until Windows and end-to-end gates pass.


## 2026-10-05 — CURRENT: cross-platform rc.3 reconciliation and product rename

**Product name:** Remote Commander Browser.

**Repository migration target:** `Usefull-Skills/remote-commander-browser`. The existing `Usefull-Skills/chatgpt-cef-linux` repository remains the current remote authority until the cross-platform candidate is accepted and repository migration can be performed without breaking open pull requests, clone remotes, CI, or release links.

**Compatibility decision:** user-facing branding and release artifact names move to Remote Commander Browser now, while the legacy internal binary/profile identifiers (`chatgpt-cef-v2`, `ChatGPT-CEF-V2`, and the existing browser-profile directory) remain compatible through the rc.3 line. This preserves authenticated browser state and existing desktop integration during the rename.

**Reconciliation:** PR #56 head `d20d2d64cc2e6ce8cbdfa30e67508a03c91aae58` was 48 commits ahead and 6 commits behind main `8689d4621b6fac77dbd087a4c385a3c34311100b`. Exact-head hosted Linux regression and Windows native jobs had both passed, including static QA, native builds, lifecycle self-tests, Windows private-file ACL guard, and Windows packaging. The divergence was documentation/release-line reconciliation, not a failing native gate. A clean isolated clone is being used; active Commander runtimes/tunnels and Saeid Codex workspaces are not mutated.

**Commander integration evidence:** Saeid's independent Codex line has now passed bounded Chrome direct-DOM and root-WebSocket renderer-constant proofs (`PASS_CHROME_DIRECT_DOM_ONLY`, `PASS_CHROME_ROOT_WS_RENDERER_CONSTANT_ONLY`), but those receipts explicitly remain `whole=NOT_FINAL`. They do not yet prove authenticated isolated test-chat binding, durable ACK/retry, sustained recovery, or CEF end-to-end acceptance.

**Current candidate:** `v0.8.0-rc.3` is the next cross-platform preview candidate. Stable `v0.7.1` remains rollback authority until rc.3 exact-tree build/test/UI/integration gates complete.

**Open gates before stable v0.8.0:** exact-tree Linux and Windows CI after reconciliation/branding; real Windows UI observation and native interaction; Commander companion read-only integration on an owned test profile; authenticated isolated test-chat acceptance without credential extraction; sustained/fault/non-interference validation; installer/rollback acceptance; repository migration verification.

**Exact next action:** finish the isolated rc.3 tree, run static/native/package regressions, exercise the Windows UI with explicit owner-authorized GUI control, then publish only if the exact head passes hosted Linux and Windows gates. Do not overwrite existing browser profiles or Commander routing state.
