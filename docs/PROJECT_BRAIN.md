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
