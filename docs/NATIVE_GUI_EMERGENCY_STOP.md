# Native Browser GUI Emergency STOP — candidate contract

## Scope
This is a narrow, monotonic owner-safety action available only from the native Remote Commander Browser chrome. It is not a web-page API, command shell, workflow runner, start/resume control, or arbitrary IPC service.

The native Commander panel displays "Emergency STOP GUI (2 clicks)" only when the current local monitor is valid, unexpired, reports a healthy GUI backend, and matches the operating system. One click arms; a second distinct native click within 8 seconds requests an owner-private, create-only GUI_STOP file. Expired, uncertain or mismatched status is rejected before any write.

## Local file contract
- Windows: %LOCALAPPDATA%\ChatGPTRemoteCommander\browser-companion\GUI_STOP
- Linux: ${XDG_STATE_HOME:-$HOME/.local/state}/chatgpt-remote-commander/browser-companion/GUI_STOP

The parent directory must be private and owned by the current user. Linux creation uses openat(O_CREAT|O_EXCL|O_NOFOLLOW), mode 0600, followed by fsync and identity checks. Windows uses CreateFileW(CREATE_NEW), explicit private ACL/NTFS and root identity checks. A symlink, pre-existing file, unsafe directory or partial write fails closed. The signal contains a fixed literal with no executable parameters.

Core must support this exact additional GUI_STOP location before this Browser candidate may be installed. The associated Core candidate adds only a stop-file check, not a command execution endpoint.

The GUI safety latch is intentionally not cleared by Browser. Recovery from a deliberate emergency stop requires the owner to investigate and explicitly remove the file using the existing documented host procedure, then obtain a fresh GUI lease and frame. Do not silently resume previous mouse/keyboard actions.

## Tests and release gates
- CEF-free native contract: invalid, stale, uncertain, wrong platform, private root, owner-only permissions, symlink/hardlink and non-overwrite.
- Windows private-file API: create-only, ACL, readback, duplicate refusal and oversized message denial.
- Core Node: platform paths, existing GUI safety and emergency-stop checks.
- Full native CEF build and isolated runtime/self-test.
- Hosted Windows/Ubuntu compilation and security qualification.
- Only after both Core and Browser candidates are released in dependency order may any production host be upgraded.

No privileged Windows recovery tasks, authentication cookies, browser-origin general commands, or profile migration are created by this feature.
