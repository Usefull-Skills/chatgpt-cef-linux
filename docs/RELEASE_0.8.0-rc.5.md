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

## Artifact identities

- native exe SHA-256: `1b43d6ef084c24baa42566df8e1910f62dff665b0717fb3f2a0e3f587b706e34`;
- Windows ZIP SHA-256: `2e91ba31a316469b833cfc50ea472b9f8a8bfc0bd6f47fb435686d5888a5abbe`;
- Windows Setup SHA-256: `86162caa43a6ad265b677636072badaf49311b9abe476188d7943c76a592288f`.

## Signing

The current Windows Setup is not Authenticode-signed because no valid Code Signing certificate/private key is available on the release host. Do not represent this as signed or SmartScreen-trusted.
