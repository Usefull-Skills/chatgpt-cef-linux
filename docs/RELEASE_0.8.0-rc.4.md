# Remote Commander Browser v0.8.0-rc.4

This candidate reconciles the Windows rc.2 line with current main and establishes the product identity Remote Commander Browser.

Compatibility: executable and profile roots keep the legacy chatgpt-cef-v2 identifiers in rc.4 so existing sessions and login state are preserved. User-facing UI, desktop metadata, documentation and release archives use Remote Commander Browser. Repository migration target: Usefull-Skills/remote-commander-browser.

Stable promotion requires exact-commit Windows and Linux native build/lifecycle gates, Windows private-file ACL guard, package integrity, Commander integration, recovery and rollback acceptance.

Release-line note: v0.8.0-rc.3 was already an immutable historical tag on an older publisher commit; this reconciled candidate advances to rc.4 rather than rewriting that tag.
