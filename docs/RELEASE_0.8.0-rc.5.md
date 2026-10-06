# Remote Commander Browser v0.8.0-rc.5

## Scope

- standalone Windows Setup EXE in addition to ZIP packaging;
- session-safe, versioned per-user installation;
- private companion-directory ACL repair/readback without administrator requirement;
- Windows built-in `tar.exe` fallback for pinned CEF extraction, so 7-Zip is no longer a mandatory build dependency;
- exact-head Windows CI now builds and silently installs the Setup into an isolated root;
- no public remote-debugging port, cookie export, credential bridge, or authenticated-profile migration.

## Local qualification

- CEF-free companion contract: PASS;
- Windows private-file guard: PASS;
- native lifecycle self-test: PASS;
- Windows ZIP package: PASS;
- standalone Setup compile: PASS;
- isolated Setup install + manifest + ACL readback: PASS;
- production `Local State`, `Cookies`, and `Login Data` fingerprints unchanged.

## Artifact integrity

The release workflow rebuilds the Windows and Linux artifacts from the immutable tag, computes their hashes, verifies them, and publishes `SHA256SUMS.txt` with the GitHub Release. That published checksum file is the authoritative artifact identity; local pre-release build hashes are qualification evidence only and are not release identities.

## Signing

The current Windows Setup is not Authenticode-signed because no valid Code Signing certificate/private key is available on the release host. Do not represent this as signed or SmartScreen-trusted.
