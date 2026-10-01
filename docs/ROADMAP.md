# Roadmap

The stable baseline is intentionally small and conservative. Future development is split into three tracks.

## Next

Focus on packaging, portability and core desktop UX without weakening the security model:

- first-class `.deb`, Flatpak and AppImage packaging
- dark/light/system theme support
- Wayland support and fractional scaling
- tab reorder/pinning/reopen-closed-tab
- download UX and drag/drop improvements
- localization and accessibility hardening
- CI reproducibility, SBOM and release provenance

## Later

Expand workspace and desktop productivity features:

- suspended/background tab memory management
- split view
- system tray / unread badge / notifications
- configurable shortcuts and command palette
- multiple workspace/profile sets
- native device selection for voice
- automatic but verified CEF update tooling

## Research

Features that require careful compatibility or security design:

- optional local diagnostics/crash-report bundle
- secure auto-update with signed manifests
- broader RTL language heuristics
- multi-window architecture
- controlled local extensions/plugin hooks
- optional performance telemetry that stays local unless explicitly exported

Detailed issue-ready proposals are in [FUTURE_FEATURES.md](FUTURE_FEATURES.md).
