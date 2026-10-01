# Building on Ubuntu

## Validated baseline

- Ubuntu 24.04 LTS family
- CMake 3.28.x
- Ninja 1.11.x
- GCC 14.x
- X11 session
- CEF `154.0.32+g682c378+chromium-154.0.8037.58`

## Packages

A practical Ubuntu build environment is:

```bash
sudo apt update
sudo apt install -y \
  build-essential cmake ninja-build curl bzip2 \
  libgtk-3-dev libnss3-dev libx11-dev libxcomposite-dev \
  libxdamage-dev libxext-dev libxfixes-dev libxrandr-dev \
  libgbm-dev libpango1.0-dev libcups2-dev libdrm-dev \
  libasound2-dev fonts-vazirmatn
```

Package names can differ on other distributions.

## Fetch the pinned CEF distribution

```bash
./scripts/fetch-cef.sh
```

The script verifies the archive against the project-pinned SHA-256 before extraction.

## Build release

```bash
./scripts/build-release.sh
```

The build uses strict project diagnostics:

```text
-Wall -Wextra -Wpedantic -Werror
```

The output runtime is under `build/bin/`.

## Sandbox requirement

Chromium's Linux SUID sandbox must be owned by root and mode `4755` before a normal runtime test/install:

```bash
sudo chown root:root build/bin/chrome-sandbox
sudo chmod 4755 build/bin/chrome-sandbox
```

Do not work around this requirement with `--no-sandbox` for normal use.

## Static QA

```bash
./scripts/static-qa.sh
```

`cppcheck` is optional but recommended:

```bash
sudo apt install cppcheck
```

## Runtime self-test

From an X11 session:

```bash
./scripts/runtime-self-test.sh
```

The test uses an isolated temporary browser profile. It does not read or modify your normal ChatGPT profile.
