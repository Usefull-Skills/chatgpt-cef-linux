# QA Results — v0.7.1

Date: 2026-10-01

## Project-source release hardening

- `cppcheck` (warning/style/performance/portability): **PASS / clean**
- clean configure from empty build directory: **PASS**
- clean build with `-Wall -Wextra -Wpedantic -Werror`: **PASS**
- `ldd`: **PASS / no missing libraries**
- isolated tab lifecycle self-test (`1 → 2 → 1`): **PASS**
- staged shutdown / final window destroyed: **PASS**
- fatal / segfault / stack-smash scan: **NONE**

## Repository-layout validation

The publication tree was also configured and built independently using its own root `CMakeLists.txt` and `src/` layout.

- configure: **PASS**
- build: **PASS**
- missing runtime dependencies: **NONE**
- repository-layout test binary SHA-256: `449cfbf855eee4af894563d5f885fa1349dd61f5c3145d6b795eb3f561b1625f`
- isolated repository-layout runtime self-test: **PASS**
- self-test exit code: `0`

The test binary hash is recorded as build evidence, not as a cross-path reproducibility guarantee; current CEF/project logging can embed build-source paths. Reproducible-build normalization is tracked as future feature `FF-013`.

## Functional acceptance inherited by this release baseline

The production acceptance series covered:

- standard ChatGPT login/session persistence
- file/image upload end to end
- copy/paste
- microphone/dictation
- ChatGPT Voice capture and explicit End Voice release
- Persian RTL and English LTR
- code/math LTR isolation
- windowed/maximized/fullscreen
- internal/external navigation routing
- single-instance behavior
- GPU and renderer health

## Defects found and resolved during final tester/debugger pass

1. **4 px overlay mismatch** — native header was 38 px while content overlay still used a legacy 42 px offset. Fixed by introducing one `kHeaderHeight` source of truth.
2. **Duplicated active-tab styling** — activation path could diverge from normal tab styling. Fixed by routing activation through `UpdateTabButton()`.
3. **Static-analysis shadow warning** — renamed local bounds variable.
4. **Raw tab lookup loop** — standardized with `std::find_if`.

A second complete QA pass after these fixes was clean.
