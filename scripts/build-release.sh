#!/usr/bin/env bash
set -euo pipefail
umask 022

ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
CEF_ROOT="$ROOT/.deps/cef"
BUILD="$ROOT/build"

[[ -d "$CEF_ROOT" ]] || { printf 'Pinned CEF not found. Run ./scripts/fetch-cef.sh first.\n' >&2; exit 20; }

rm -rf "$BUILD"
cmake -S "$ROOT" -B "$BUILD" -G Ninja \
  -DCEF_ROOT="$CEF_ROOT" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_CXX_FLAGS_RELEASE='-O2 -DNDEBUG -Wall -Wextra -Wpedantic -Werror'
cmake --build "$BUILD" -j"${BUILD_JOBS:-2}"

BIN="$BUILD/bin/chatgpt-cef-v2"
[[ -x "$BIN" ]] || { printf 'Release binary missing after build.\n' >&2; exit 21; }
if ldd "$BIN" | grep -q 'not found'; then
  ldd "$BIN" | grep 'not found' >&2
  exit 22
fi

printf 'BUILD=PASS\n'
printf 'BINARY=%s\n' "$BIN"
printf 'BINARY_SHA256=%s\n' "$(sha256sum "$BIN" | awk '{print $1}')"
printf 'Before runtime testing, configure build/bin/chrome-sandbox as root:root mode 4755.\n'
