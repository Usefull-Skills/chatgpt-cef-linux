#!/usr/bin/env bash
set -euo pipefail

ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
VERSION=$(tr -d '[:space:]' < "$ROOT/VERSION")
RUNTIME="$HOME/.local/share/chatgpt-cef-v2/runtime-v$VERSION"
LAUNCHER="$HOME/.local/bin/remote-commander-browser"
LEGACY_LAUNCHER="$HOME/.local/bin/chatgpt-cef-v2"
DESKTOP="$HOME/.local/share/applications/remote-commander-browser.desktop"
LEGACY_DESKTOP="$HOME/.local/share/applications/chatgpt-cef-v2.desktop"

rm -f "$LAUNCHER" "$LEGACY_LAUNCHER" "$DESKTOP" "$LEGACY_DESKTOP"
if [[ -d "$RUNTIME" ]]; then
  if [[ -e "$RUNTIME/chrome-sandbox" && ! -w "$RUNTIME/chrome-sandbox" ]]; then
    sudo rm -f "$RUNTIME/chrome-sandbox"
  fi
  rm -rf "$RUNTIME"
fi

if [[ "${1:-}" == '--purge-profile' ]]; then
  rm -rf "$HOME/.config/chatgpt-cef-v2"
  printf 'PROFILE_PURGED=YES\n'
else
  printf 'PROFILE_PRESERVED=%s\n' "$HOME/.config/chatgpt-cef-v2"
fi
printf 'UNINSTALL=PASS\n'
