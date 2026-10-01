#!/usr/bin/env bash
set -euo pipefail
umask 022

ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
DEPS="$ROOT/.deps"
DOWNLOADS="$DEPS/downloads"
CEF_ROOT="$DEPS/cef"
NAME='cef_binary_154.0.32+g682c378+chromium-154.0.8037.58_linux64_minimal'
ARCHIVE="$DOWNLOADS/${NAME}.tar.bz2"
URL='https://cef-builds.spotifycdn.com/cef_binary_154.0.32%2Bg682c378%2Bchromium-154.0.8037.58_linux64_minimal.tar.bz2'
EXPECTED_SHA256='9b6a82e04506d5e1af560e031e718c89af5f96760413fd16380358784545d153'

mkdir -p "$DOWNLOADS"

if [[ -f "$ARCHIVE" ]]; then
  actual=$(sha256sum "$ARCHIVE" | awk '{print $1}')
  if [[ "$actual" != "$EXPECTED_SHA256" ]]; then
    printf 'Existing CEF archive has the wrong SHA256; refusing to overwrite silently.\n' >&2
    printf 'expected=%s\nactual=%s\n' "$EXPECTED_SHA256" "$actual" >&2
    exit 21
  fi
else
  curl --fail --location --show-error --progress-bar "$URL" -o "$ARCHIVE.part"
  actual=$(sha256sum "$ARCHIVE.part" | awk '{print $1}')
  if [[ "$actual" != "$EXPECTED_SHA256" ]]; then
    rm -f "$ARCHIVE.part"
    printf 'CEF SHA256 verification failed.\nexpected=%s\nactual=%s\n' "$EXPECTED_SHA256" "$actual" >&2
    exit 22
  fi
  mv "$ARCHIVE.part" "$ARCHIVE"
fi

if [[ -d "$CEF_ROOT" ]]; then
  if grep -q 'CEF Version:      154.0.32+g682c378+chromium-154.0.8037.58' "$CEF_ROOT/README.txt" 2>/dev/null; then
    printf 'CEF already extracted and version-verified: %s\n' "$CEF_ROOT"
    exit 0
  fi
  printf 'Existing .deps/cef does not match the pinned version; refusing to replace it implicitly.\n' >&2
  exit 23
fi

tmp=$(mktemp -d "$DEPS/.cef-extract.XXXXXX")
trap 'rm -rf "$tmp"' EXIT

tar -xjf "$ARCHIVE" -C "$tmp"
extracted="$tmp/$NAME"
[[ -d "$extracted" ]] || { printf 'Unexpected CEF archive layout.\n' >&2; exit 24; }
mv "$extracted" "$CEF_ROOT"

printf 'CEF_ROOT=%s\n' "$CEF_ROOT"
printf 'CEF_ARCHIVE_SHA256=%s\n' "$EXPECTED_SHA256"
