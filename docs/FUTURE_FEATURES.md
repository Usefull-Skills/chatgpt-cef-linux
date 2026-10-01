# Future Features

This document is the canonical backlog for ideas that are **not part of the stable v0.7.1 baseline**. Each `FF-xxx` item is intended to become a GitHub issue.

## Next — desktop fundamentals

### FF-001 — Native Dark / Light / System theme
Add native shell and injected web-surface theme modes with system preference detection, persistence, and contrast validation. Do not use DOM overrides that break ChatGPT's own accessibility states.

### FF-002 — Wayland backend
Add a supported Wayland path while retaining validated X11 fallback. Cover native window controls, clipboard, file dialogs, voice, screen scaling and external-browser routing.

### FF-003 — Fractional scaling and per-monitor DPI
Handle 100/125/150/175/200% scaling and monitor transitions without blurry content, incorrect hit targets or geometry drift.

### FF-004 — Tab reordering by drag and drop
Allow native tabs to be reordered, persist the order, and preserve the active tab and BrowserView identity.

### FF-005 — Pinned tabs
Add compact pinned tabs with persistent state and protection from bulk-close operations.

### FF-006 — Reopen closed tab
Maintain a bounded closed-tab history and expose `Ctrl+Shift+T` without storing sensitive page content.

### FF-007 — Native downloads UX
Expose safe download progress, completion/open-folder actions and cancellation while retaining Chromium's download protections.

### FF-008 — File drag and drop
Support dragging files/images directly onto the ChatGPT composer, including clear visual drop-state feedback.

### FF-009 — Clipboard image paste hardening
Test and improve direct image clipboard paste, large-image limits and failure feedback.

### FF-010 — First-class Linux packaging
Provide `.deb`, Flatpak and AppImage release pipelines with uninstall/upgrade semantics and desktop integration.

### FF-011 — Dedicated app icon and brand-safe visual identity
Create original project artwork that does not imply official OpenAI endorsement; provide scalable SVG and Linux icon sizes.

### FF-012 — Installer / uninstaller UX
Provide deterministic local/system installation, rollback, cache/profile preservation choices and complete cleanup reporting.

## Next — quality, security and release engineering

### FF-013 — Reproducible build verification
Measure deterministic build inputs/outputs, record toolchain versions, and compare binary hashes where toolchain behavior permits.

### FF-014 — SBOM generation
Generate SPDX or CycloneDX software bill of materials for source and binary releases, including pinned CEF/Chromium versions.

### FF-015 — Release provenance and signed checksums
Publish machine-readable SHA-256 manifests and signed release provenance. Do not auto-trust unsigned updater metadata.

### FF-016 — Secure auto-update
Design signed-manifest update checking with staged download, exact hash/signature verification, rollback and explicit user control.

### FF-017 — CEF compatibility matrix and updater tooling
Track tested CEF/Chromium versions and automate candidate builds without silently promoting an unvalidated runtime.

### FF-018 — Permission-policy regression tests
Automate exact-origin tests for microphone/camera/clipboard plus negative tests for lookalike hosts and unexpected permission types.

### FF-019 — Navigation-routing regression tests
Automate internal ChatGPT/auth routing and external-link delegation, including malicious/ambiguous URL boundary cases.

### FF-020 — Screenshot-based visual regression tests
Capture privacy-safe native-shell fixtures and compare geometry/theme pixels across releases and window modes.

### FF-021 — Crash/diagnostics export bundle
Create an opt-in, local-only diagnostics exporter that redacts cookies, tokens, conversation content and user paths by default.

### FF-022 — Automated secret/privacy release scanner
Expand release checks for browser databases, tokens, credentials, absolute user paths, build caches and accidental screenshots.

## Later — performance and workspaces

### FF-023 — Background-tab suspension
Suspend inactive BrowserViews after configurable inactivity while preserving recoverable tab state and avoiding message loss.

### FF-024 — Memory-pressure-aware tab unloading
Respond to Linux memory pressure with deterministic, user-visible unloading rules and safe reload behavior.

### FF-025 — Split view
Display two ChatGPT tabs side by side with focus-aware shortcuts and independently resizable panes.

### FF-026 — Multiple workspace/profile sets
Support explicitly separated browser profiles/workspaces without copying cookies between them. Make the active profile obvious to the user.

### FF-027 — Window placement persistence
Persist window size/state/monitor placement safely across single- and multi-monitor setups, including disconnected monitors.

### FF-028 — Optional multi-window mode
Research multiple top-level windows while preserving one process/application instance and predictable session restore.

### FF-029 — Command palette
Add a searchable native command palette for tabs, navigation, theme, zoom and diagnostics actions.

### FF-030 — Customizable keyboard shortcuts
Allow conflict-checked remapping with reset-to-default and import/export of non-sensitive settings.

### FF-031 — Zoom controls and per-site zoom persistence
Expose explicit zoom controls with keyboard shortcuts and safe persistence independent of monitor DPI.

## Later — desktop integration

### FF-032 — System notifications
Integrate desktop notifications only where web/app behavior permits it safely; provide granular opt-in and quiet-mode controls.

### FF-033 — Tray integration
Optional tray icon for show/hide, new chat and quit. Must not keep hidden microphone capture active.

### FF-034 — Unread/activity badge
Expose a privacy-preserving activity indicator without scraping or storing conversation text.

### FF-035 — Global quick-launch shortcut
Optional global shortcut to focus the app or start a new chat, with conflict detection and explicit enablement.

### FF-036 — External-browser routing preferences
Allow users to choose the default browser or per-domain routing rules while keeping a strict safe default.

## Later — voice, accessibility and language

### FF-037 — Native audio device selector
Expose microphone/output-device selection and test hot-plug behavior without broadening site permissions.

### FF-038 — Voice-state UX
Add clearer native indicators for listening/processing/end states and make it impossible to leave capture running invisibly.

### FF-039 — AT-SPI accessibility improvements
Improve native tab/window accessibility names, keyboard focus order and screen-reader interoperability.

### FF-040 — Persian localization
Provide Persian native-shell labels/tooltips/settings while preserving English as a selectable UI language.

### FF-041 — Arabic and Hebrew direction heuristics
Generalize mixed-direction handling and add regression fixtures for Arabic/Hebrew/English/numerals/code/math.

### FF-042 — RTL list/table edge-case suite
Add tests for nested ordered lists, punctuation, mathematical expressions, quoted English and tables inside RTL responses.

## Research — advanced product ideas

### FF-043 — Local-only performance dashboard
Optional diagnostics page for process CPU/memory/GPU/tab state. No remote telemetry by default.

### FF-044 — Sandboxed extension points
Research a constrained local extension model for native UI actions without allowing arbitrary injection into authenticated ChatGPT pages.

### FF-045 — Offline settings/control center
A small native settings view for app-only preferences that does not require or intercept ChatGPT account settings.

### FF-046 — Session export/import of non-secret layout
Export only window/tab URL/layout preferences, explicitly excluding browser cookies, credentials and authenticated storage.

### FF-047 — Crash recovery mode
Detect repeated startup crashes and offer a safe recovery path that disables risky optional app features without deleting the user's profile.

### FF-048 — GPU/backend compatibility modes
Maintain validated GPU/default/software-fallback profiles for problematic drivers without globally disabling the Chromium sandbox.

### FF-049 — Automated long-chat stress suite
Benchmark scrolling, typing, tab switching and memory behavior on very long ChatGPT conversations with reproducible local fixtures.

### FF-050 — Upstream DOM compatibility monitor
Detect when ChatGPT DOM changes invalidate optional style/RTL selectors and fail gracefully instead of applying broad selectors.

## Backlog governance

Every issue created from this list should include:

- user value
- security/privacy impact
- implementation scope
- acceptance criteria
- regression tests
- release target (`next`, `later`, `research`)

No future feature should be merged directly into the stable release without passing the release checklist.
