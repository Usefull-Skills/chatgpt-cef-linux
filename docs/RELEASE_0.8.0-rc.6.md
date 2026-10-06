# Remote Commander Browser v0.8.0-rc.6

## Scope

- standalone Windows Setup EXE in addition to ZIP packaging;
- session-safe, versioned per-user installation;
- private companion-directory ACL repair/readback without administrator requirement;
- Windows built-in `tar.exe` fallback for pinned CEF extraction, so 7-Zip is not a mandatory build dependency;
- exact-head Windows CI builds and silently installs the Setup into an isolated root;
- no public remote-debugging port, cookie export, credential bridge, saved-password extraction, or authenticated-profile migration.

## rc.6 release-infrastructure correction

The rc.5 product tree passed its exact-head Windows and Linux qualification, but the rc.5 tag release workflow stopped before the Windows build because the Chocolatey community feed no longer served the requested `innosetup 6.7.3` package version. The rc.5 tag is retained unchanged for provenance and is not moved or republished.

rc.6 removes that mutable package-feed dependency. The Windows release job downloads the official Inno Setup 6.7.3 asset from the immutable `jrsoftware/issrc` GitHub release `is-6_7_3`, verifies SHA-256 `9c73c3bae7ed48d44112a0f48e66742c00090bdb5bef71d9d3c056c66e97b732`, requires a valid Authenticode signature from Pyrsys B.V., and only then uses the compiler.

## Qualification inherited from the unchanged Browser runtime

- CEF-free companion contract: PASS;
- Windows private-file guard: PASS;
- native lifecycle self-test: PASS;
- Windows ZIP package: PASS;
- standalone Setup compile: PASS;
- isolated Setup install + manifest + exact private ACL readback: PASS;
- Linux regression and Linux release build: PASS;
- production browser profile/login state is not used or migrated by installer qualification.

## Artifact integrity

The release workflow rebuilds Windows and Linux artifacts from the immutable tag, computes and verifies hashes, and publishes `SHA256SUMS.txt` with the GitHub Release. That published checksum file is the authoritative release artifact identity.

## Signing

Remote Commander Browser Setup itself remains unsigned because no project Code Signing certificate/private key is available. The third-party Inno Setup compiler bootstrap is independently SHA-256 pinned and Authenticode verified; that does not sign the resulting Browser Setup.
