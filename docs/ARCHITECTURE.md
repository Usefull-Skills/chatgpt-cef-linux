# Architecture

## Overview

Remote Commander Browser is a native Windows/Linux host around Chromium Embedded Framework (CEF). The native process owns windowing, tabs, permission policy, navigation routing, session URL persistence, lifecycle management, and the optional read-only Remote Commander companion. ChatGPT remains the web application loaded from `https://chatgpt.com/`.

CEF is an implementation detail rather than the product name.

## Compatibility identity

The rc.3 branding migration preserves historical internal identifiers:

- binary/target: `chatgpt-cef-v2`
- Linux browser profile: `~/.config/chatgpt-cef-v2`
- Windows local profile identifier: `chatgpt-cef-v2`
- legacy WM class: `ChatGPT-CEF-V2`

These identifiers are intentionally retained during rc.3 so authenticated browser state and existing desktop integration are not silently migrated. New launchers/artifact names use Remote Commander Browser branding and keep legacy aliases.

## Main components

### `main.cc`

- Initializes CEF and subprocess roles.
- Supports `--cgwa-profile-dir=<absolute-path>` for isolated acceptance profiles.
- Creates private cache/profile directories before CEF initialization.
- Uses platform-specific default profile roots while retaining the legacy compatibility identifier.

### `app.cc`

- Configures browser-process command-line switches.
- Keeps platform switches platform-specific.
- Enables the qualified rendering path.
- Creates the application controller after CEF context initialization.
- Handles single-instance relaunch behavior.

### `controller.cc`

Owns the native shell:

- CEF Views window and branded header
- native tab strip and up to 8 BrowserViews
- keyboard accelerators
- fullscreen/maximize/minimize/close controls
- tab creation, activation and destruction
- staged shutdown
- atomic `tabs.state` persistence
- window layout and overlay geometry
- optional read-only Commander companion panel

The release-hardening baseline uses one source of truth for header geometry so native content overlays and the visual header cannot drift apart.

### `client.cc`

Owns browser policy:

- browser lifecycle callbacks
- internal vs external URL routing
- popup handling
- exact-origin permission decisions
- media access
- page-load UI/RTL injection
- exact trusted-origin checks

### Commander companion

The companion is read-only. It consumes bounded local Commander observation/binding artifacts through platform-specific private-file guards. It does not execute Commander operations, accept page-originated authority, or read arbitrary user files.

Windows validates NTFS/private ACL, path identity, hardlink/reparse safety and bounded reads. Linux uses the qualified private local artifact boundary.

## Trust boundary

Only the exact HTTPS ChatGPT origin is trusted for media/clipboard permission handling. Hostname-prefix matching is not used.

Authentication credentials are managed by Chromium/ChatGPT in the private browser profile. The repository and Commander companion do not require credential extraction.

## Navigation model

- `chatgpt.com`: stays in the native client.
- supported OpenAI authentication navigation: stays in-app where required for login.
- external URLs: delegated to the operating-system browser.

## Tab and persistence model

Each tab has an internal ID, BrowserView, overlay, native title/close controls, browser ID and loading state. Only the active overlay is visible.

`tabs.state` stores simple URL/layout state, not authentication tokens. Writes are atomic and the containing private profile remains owner-only.

## RTL system

A bounded post-load JavaScript/CSS injector:

- detects Persian/Arabic vs Latin strong characters
- applies RTL/right alignment to Persian/Arabic content
- keeps English LTR
- forces code and math LTR
- uses Vazirmatn-first typography when installed
- observes dynamic DOM changes

Selectors are intentionally narrow and must be regression-tested when ChatGPT DOM changes.

## Release model

A release is accepted only after exact-tree static checks, Linux and Windows native build/lifecycle tests, package verification, real UI observation, Commander companion acceptance, and rollback-safe installation evidence. Source compilation alone is not whole-product acceptance.
