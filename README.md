# Remote Commander Browser

Remote Commander Browser is a hardened cross-platform native desktop shell for ChatGPT built with Chromium Embedded Framework (CEF). It provides native tabs, Persian/Arabic RTL support, voice/media permissions scoped to the trusted ChatGPT origin, session restore, and an optional read-only Remote Commander companion.

> **Unofficial project.** This software is not affiliated with or endorsed by OpenAI. It loads `https://chatgpt.com/` and uses the normal ChatGPT sign-in flow.

## Release status

**v0.8.0-rc.3 — cross-platform preview candidate / NOT STABLE**

Linux and Windows native build/lifecycle paths are qualified independently. Stable `v0.7.1` remains the rollback channel until exact-tree rc.3 CI, real UI observation, Commander↔Browser integration, authenticated isolated test-chat acceptance, sustained recovery, and installer rollback gates are complete.

## Highlights

- Native CEF Views shell for Windows and Linux/X11
- Frameless window with native tabs (up to 8)
- `Ctrl+T`, `Ctrl+W`, `Ctrl+Tab`, `Ctrl+Shift+Tab`, `Ctrl+1..8`, `F11`
- Atomic session URL restore
- Single-instance relaunch handling
- Exact-origin permission policy for `https://chatgpt.com`
- Camera/microphone/clipboard support without broad origin grants
- Internal ChatGPT navigation stays in-app; external links route to the system browser
- Vazirmatn-first Persian typography with automatic RTL/LTR detection
- Read-only local Remote Commander companion
- Native Windows private-file ACL guard
- Sandboxed Chromium runtime and bounded release packaging

## Compatibility and profile safety

The product name and release artifact names are moving to **Remote Commander Browser**. For rc.3, the historical internal executable/profile identifiers remain compatible:

```text
chatgpt-cef-v2
~/.config/chatgpt-cef-v2/
```

On Windows, the existing local profile identifier is also preserved. This is intentional: authenticated browser state is not copied, renamed, or migrated implicitly during the branding transition.

## Security model

The application uses Chromium's normal web isolation model. Linux keeps the Chromium SUID sandbox enabled; Windows uses the native CEF sandbox/runtime boundary available to the build.

Media permissions are granted only to the exact trusted ChatGPT origin. Lookalike domains are not accepted by prefix matching.

The Commander companion reads only bounded private local artifacts through platform-specific guards. It does not grant Commander execution authority to page content.

## Build

Pinned browser engine:

```text
CEF 154.0.32+g682c378+chromium-154.0.8037.58
```

Linux:

```bash
./scripts/fetch-cef.sh
./scripts/build-release.sh
./scripts/static-qa.sh
./scripts/runtime-self-test.sh
```

Windows:

```powershell
./scripts/fetch-cef-windows.ps1
./scripts/build-windows.ps1
./scripts/package-windows.ps1
```

See [docs/BUILDING.md](docs/BUILDING.md) for prerequisites and details.

## Install locally

Linux builds can be installed with:

```bash
./scripts/install-local.sh
```

The installer validates the runtime and Chromium sandbox. The legacy browser-profile path is deliberately preserved during rc.3.

## Architecture and evidence

- [Architecture](docs/ARCHITECTURE.md)
- [Project Brain](docs/PROJECT_BRAIN.md)
- [Knowledge/Evidence](docs/PROJECT_KNOWLEDGE_EVIDENCE.md)
- [rc.3 release candidate notes](docs/RELEASE_0.8.0-rc.3.md)

## Repository migration

The professional repository name is **`remote-commander-browser`**. The current repository remains authoritative until the rc.3 tree is accepted and migration can be completed without breaking open pull requests, CI, remotes, or release references.

## Licensing and trademarks

See [NOTICE.md](NOTICE.md) and [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md). Third-party components retain their original licenses.
