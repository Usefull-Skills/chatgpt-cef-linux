# Remote Commander Browser v0.8.0-rc.9

## Scope and acceptance boundary

Release integrity hardening only. The Browser runtime, profile ownership, cookies,
CEF source and native installer behavior are unchanged from rc.8. This is a
pre-release, not whole-product FINAL or publisher-signature acceptance.

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
