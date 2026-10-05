# Architecture

## Overview

Remote Commander Browser is a small cross-platform native host around Chromium Embedded Framework (CEF). The native process owns windowing, tabs, permission policy, navigation routing, session URL persistence, and lifecycle management. ChatGPT itself remains a web application loaded from `https://chatgpt.com/`.

## Main components

### `main.cc`

- Initializes CEF.
- Supports an isolated profile override via `--cgwa-profile-dir=<absolute-path>` for testing.
- Uses `~/.config/chatgpt-cef-v2` as the normal profile root.
- Creates cache directories before CEF initialization.
- Executes CEF subprocess roles using the same binary.

### `app.cc`

- Configures browser-process command-line switches.
- Forces X11 via Chromium's Ozone layer for the validated release baseline.
- Enables GPU rasterization and zero-copy.
- Creates the application controller after CEF context initialization.
- Handles already-running relaunches so the application remains single-instance.

### `controller.cc`

Owns the native shell:

- CEF Views window and header
- native tab strip and up to 8 BrowserViews
- keyboard accelerators
- fullscreen/maximize/minimize/close controls
- tab creation, activation and destruction
- staged shutdown
- atomic `tabs.state` persistence
- window layout and overlay geometry
- X11 WM class and frameless-window behavior

The release-hardening baseline uses one source of truth for header geometry (`kHeaderHeight`) so native content overlays and the visual header cannot drift apart.

### `client.cc`

Owns browser policy:

- browser lifecycle callbacks
- internal vs external URL routing
- permission decisions
- media access
- popup handling
- page-load UI/RTL injection
- exact trusted-origin checks

## Trust boundary

Application policy trusts only the exact HTTPS ChatGPT origin for media/clipboard permission handling. Hostname-prefix matching is intentionally not used.

Authentication credentials are managed by Chromium/ChatGPT in the runtime profile. The repository does not contain or need ChatGPT credentials.

## Navigation model

- `chatgpt.com`: stays in the native client.
- supported OpenAI authentication navigation: stays in the client where required for login.
- external URLs: delegated to `xdg-open` so the user's normal system browser handles them.

## Tab model

Each tab has:

- internal integer ID
- CEF BrowserView
- overlay controller
- native title button
- native close button
- browser ID / loading state

Only the active overlay is visible. `UpdateTabButton()` is the canonical native tab-style update path.

## Persistence

`tabs.state` contains only simple URL/session-layout state; it does not contain authentication tokens. Writes are atomic:

1. write `tabs.state.tmp`
2. flush/close
3. set mode `0600`
4. rename to `tabs.state`

The browser profile itself should be mode `0700`.

## RTL system

A post-load JavaScript/CSS injector:

- detects Persian/Arabic vs Latin strong characters
- applies RTL/right alignment to Persian/Arabic message/composer content
- keeps English LTR
- forces code, keyboard snippets and math LTR
- applies Vazirmatn-first typography when installed
- observes dynamic DOM updates using a MutationObserver

The selectors are deliberately limited; changes to ChatGPT's web DOM may require maintenance in future versions.
