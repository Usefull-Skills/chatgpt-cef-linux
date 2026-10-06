# Remote Commander Browser v0.8.0-rc.7

## Scope

rc.7 keeps the rc.6 Browser runtime, ACL, session-safety and installer behavior unchanged. This release corrects only the GitHub Release asset layout so the standalone Windows Setup EXE is published as a top-level release asset and can be pinned by Remote Commander.

## Release-layout correction

The rc.6 tag build and release workflow completed successfully, but the published GitHub Release omitted the standalone Setup EXE. The Windows workflow artifact preserved the Setup under an `installer/` subdirectory, while the publish step intentionally enumerated only top-level files. ZIP/Linux artifacts and their checksums were published; the Setup was therefore not available as an immutable release dependency.

rc.7 copies the qualified Setup to the top level of the Windows release artifact before upload and adds a publish-time assertion requiring `Remote-Commander-Browser-Setup-v0.8.0-rc.7.exe` to exist in the merged release directory. Missing Setup now fails the release instead of silently publishing an incomplete asset set.

## Qualification inherited from rc.6

- official Inno Setup 6.7.3 bootstrap pinned by SHA-256 and valid Authenticode signer;
- Windows native lifecycle and private-file guard PASS;
- standalone Setup build + silent isolated install PASS;
- private companion ACL exact readback PASS;
- Linux regression and Linux release build PASS;
- authenticated production browser profiles are not migrated or used by installer QA.

## Artifact integrity

The tag release workflow rebuilds Windows and Linux artifacts, verifies them, and publishes `SHA256SUMS.txt`. The published checksum file is the authoritative release identity.

## Signing

The Browser Setup remains unsigned because no project code-signing certificate/private key is available. This is an external publisher-trust gap, not a runtime qualification claim.
