# Remote Commander Browser v0.8.0-rc.9

## Scope and acceptance boundary

Release integrity hardening plus a narrowly tested Linux launcher-template repair.
Browser CEF source, profile ownership, cookies and Windows installer behavior are
unchanged from rc.8. The Linux installer now defers launcher variable expansion
correctly. This is a pre-release, not whole-product FINAL or publisher-signature
acceptance.

## Confirmed defects and changes

- Independent seven-case Windows regression reproduced the old nullable
  LASTEXITCODE false-success and verified the rc.8 fix. An echoed run-script PASS
  string was not executed stage evidence; those historical claims are superseded.
- Downloaded rc.8 PR artifacts matched both GitHub archive digests. Windows producer
  checksums matched. Linux producer digest also matched, but its manifest recorded
  an absolute hosted-runner path. The manifest now records only the portable basename.
- Publication now checks all three original producer digests and exactly six input
  files before computing the aggregate checksum. Missing, extra, malformed,
  path-containing, symlinked and corrupted artifacts are rejected.
- RC publication explicitly uses --prerelease and --latest=false. Existing releases
  require manual reconciliation instead of a blind success exit.
- Post-publication API readback requires immutable=true and the expected preview
  classification. Repository immutability must be enabled before tag publication.
- Cross-platform CEF-free positive/negative tests exercise the new contract; the
  Linux matrix also executes the actual packager in a temporary path with spaces.

## Remaining gates

Require exact-head hosted Release, Build and Windows native qualification before
controlled merge/tag. After tag publication, verify metadata and downloaded
published bytes again; PR artifacts and tag-rebuilt binaries have distinct hashes.
The existing rc.8 release is preserved; do not replace its assets or move its tag.
Code signing is still external. Authenticated UI, native worker, sustained recovery,
combined Commander installer and serial live rollout are separate acceptance gates.


### 2026-10-06 - R4 Linux generated launcher repair

An isolated execution of the actual generated installer on the previous candidate
failed before launching the Browser: RUNTIME: unbound variable. Real privilege
commands were replaced with guarded no-op fixtures; no installed profiles changed.
The nested unquoted heredoc expanded launcher-only variables during installation.
The launcher body now uses a quoted delimiter, with installation target and verified
hash values supplied through Bash printf %q. Existing ownership and digest guards
remain enabled. GNU Bash Redirections/Here Documents and Bash Builtins/printf are
the primary semantics reference; source syntax alone was not acceptance evidence.

A new native POSIX functional case covers generated installation, HOME with spaces,
literal argument forwarding including Unicode, and binary/sandbox tamper rejection.
The suite now has15 distinct cases: Windows can execute12 with3 POSIX skips. Native
Linux before/after proof and hosted exact-head checks must be separately recorded.
This fixes the generated shell contract; real CEF UI, SUID/privilege behavior and
full installer rollback remain separate acceptance gates. No tag or live rollout.
