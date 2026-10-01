# v0.7.1 Release Notes

## Release Hardening

v0.7.1 is the first publication-ready baseline of the native CEF client.

### User-facing capabilities

- native frameless Linux shell
- native multi-tab workflow with keyboard shortcuts
- Persian/Arabic RTL and English/code/math LTR behavior
- Vazirmatn-first Persian typography
- login/session persistence through Chromium's normal profile
- image/file upload
- copy/paste
- microphone/dictation and ChatGPT Voice
- windowed, maximized and fullscreen modes
- internal ChatGPT routing and external system-browser routing
- single-instance application behavior

### Release engineering

The final release candidate was rebuilt from an empty build directory using:

```text
-O2 -DNDEBUG -Wall -Wextra -Wpedantic -Werror
```

Project source was also checked with `cppcheck` using warning/style/performance/portability checks.

### Debugging findings resolved before publication

- Fixed a native content-overlay geometry mismatch: the visual header had been reduced to 38 px while overlay layout still retained a legacy 42 px offset.
- Replaced duplicated active-tab styling with one canonical `UpdateTabButton()` path.
- Removed a shadowing local variable identified by static analysis.
- Standardized tab lookup using `std::find_if`.

### Pinned browser engine

- CEF `154.0.32+g682c378+chromium-154.0.8037.58`
- CEF archive SHA-256: `9b6a82e04506d5e1af560e031e718c89af5f96760413fd16380358784545d153`

### Security

The release keeps Chromium's Linux sandbox enabled. Media permission policy is scoped to the exact trusted ChatGPT origin; no browser cookies, credentials or user profile data are included in the repository or release source archive.
