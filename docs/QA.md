# QA and acceptance matrix

## Release gate

A release is not considered final unless all mandatory gates pass.

| Area | Gate |
|---|---|
| Build | Clean configure/build from an empty build directory |
| Compiler | `-Wall -Wextra -Wpedantic -Werror` |
| Static analysis | No actionable project-source findings |
| Dependencies | `ldd` reports no `not found` entries |
| Security | No committed profile/cookie/token/log data |
| Sandbox | Linux sandbox present, root-owned, mode `4755` for installed runtime |
| Tab lifecycle | Automated 1 → 2 → 1 self-test passes |
| Shutdown | All browser instances close before final window/message-loop shutdown |
| Login/session | Standard ChatGPT sign-in and persistence works |
| RTL | Persian composer/message path renders RTL |
| LTR | English and code/math remain LTR |
| Clipboard | Copy/paste works without leaving test clipboard owners |
| Upload | File/image selection reaches ChatGPT and completes |
| Audio | Dictation/microphone capture starts/stops cleanly |
| Voice | Voice session starts and releases capture after End Voice |
| Navigation | Internal ChatGPT links stay internal; external links use system browser |
| Window states | Windowed, maximized and fullscreen |
| Instance model | Second launch reuses existing instance |
| Runtime | GPU and renderer processes healthy; no current fatal/segfault |

## v0.7.1 evidence

The local release-engineering run additionally found and fixed a native layout defect before publication: the modern header was 38 px high while browser overlays still used a legacy 42 px offset. The release baseline now derives both from the same `kHeaderHeight` constant.

Static analysis also drove two cleanup changes: a shadowed local variable was renamed and tab lookup was made explicit with `std::find_if`.
