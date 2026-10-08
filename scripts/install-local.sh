#!/usr/bin/env bash
set -euo pipefail
umask 077

ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
VERSION=$(tr -d '[:space:]' < "$ROOT/VERSION")
BUILD="$ROOT/build/bin"
APPROOT="$HOME/.local/share/chatgpt-cef-v2"
RUNTIME="$APPROOT/runtime-v$VERSION"
LAUNCHER="$HOME/.local/bin/chatgpt-cef-v2"
DESKTOP="$HOME/.local/share/applications/chatgpt-cef-v2.desktop"
ICON_SOURCE="$ROOT/assets/remote-commander-browser-logo.png"
ICON_TARGET="$HOME/.local/share/icons/hicolor/512x512/apps/remote-commander-browser.png"
ICON_SHA256="d724415693a4a4da8c20a065db978467ae979714d9b0559ace7dc33035461195"

[[ -x "$BUILD/chatgpt-cef-v2" ]] || { printf 'Build first: ./scripts/build-release.sh\n' >&2; exit 20; }
[[ -f "$BUILD/chrome-sandbox" ]] || { printf 'Build output is missing chrome-sandbox.\n' >&2; exit 21; }
[[ ! -e "$RUNTIME" ]] || { printf 'Runtime already exists: %s\nRefusing to overwrite.\n' "$RUNTIME" >&2; exit 22; }

# No foreign, replaced or symlinked visual asset may be silently overwritten.
[[ -f "$ICON_SOURCE" && ! -L "$ICON_SOURCE" ]] || { echo 'Official Browser icon missing/unsafe' >&2; exit 25; }
[[ "$(sha256sum "$ICON_SOURCE" | awk '{print $1}')" == "$ICON_SHA256" ]] || { echo 'Official Browser icon hash mismatch' >&2; exit 26; }
if [[ -e "$ICON_TARGET" || -L "$ICON_TARGET" ]]; then
  [[ -f "$ICON_TARGET" && ! -L "$ICON_TARGET" ]] || { echo 'Existing icon is not a regular file' >&2; exit 27; }
  [[ "$(sha256sum "$ICON_TARGET" | awk '{print $1}')" == "$ICON_SHA256" ]] || { echo 'Existing icon differs from signed source; leave unchanged' >&2; exit 28; }
fi

mkdir -p "$APPROOT" "$HOME/.local/bin" "$HOME/.local/share/applications"
cp -a "$BUILD" "$RUNTIME"

printf 'Configuring Chromium SUID sandbox (sudo required)...\n'
sudo chown root:root "$RUNTIME/chrome-sandbox"
sudo chmod 4755 "$RUNTIME/chrome-sandbox"

BIN_SHA=$(sha256sum "$RUNTIME/chatgpt-cef-v2" | awk '{print $1}')
SAN_SHA=$(sha256sum "$RUNTIME/chrome-sandbox" | awk '{print $1}')

cat > "$LAUNCHER" <<LAUNCH
#!/usr/bin/env bash
set -euo pipefail
umask 077
RUNTIME="\$HOME/.local/share/chatgpt-cef-v2/runtime-v$VERSION"
BIN="\$RUNTIME/chatgpt-cef-v2"
SANDBOX="\$RUNTIME/chrome-sandbox"
EXPECTED_BIN_SHA="$BIN_SHA"
EXPECTED_SANDBOX_SHA="$SAN_SHA"
fail(){ printf 'ChatGPT CEF V2: %s\\n' "\$1" >&2; exit "\$2"; }
[[ -x "\$BIN" ]] || fail "executable missing" 20
[[ -f "\$SANDBOX" ]] || fail "sandbox missing" 21
read -r mode owner group < <(stat -Lc '%a %U %G' "\$SANDBOX")
[[ "\$mode" == 4755 && "\$owner" == root && "\$group" == root ]] || fail "unsafe sandbox permissions" 22
[[ "\$(sha256sum "\$BIN" | awk '{print \$1}')" == "\$EXPECTED_BIN_SHA" ]] || fail "binary SHA256 mismatch" 23
[[ "\$(sha256sum "\$SANDBOX" | awk '{print \$1}')" == "\$EXPECTED_SANDBOX_SHA" ]] || fail "sandbox SHA256 mismatch" 24
cd "\$RUNTIME"
exec "\$BIN" "\$@"
LAUNCH
chmod 0755 "$LAUNCHER"

install -d -m 0755 "$HOME/.local/share/icons/hicolor/512x512/apps"
if [[ ! -e "$ICON_TARGET" ]]; then install -m 0644 "$ICON_SOURCE" "$ICON_TARGET"; fi
[[ "$(sha256sum "$ICON_TARGET" | awk '{print $1}')" == "$ICON_SHA256" ]] || exit 29
cat > "$DESKTOP" <<DESKTOP
[Desktop Entry]
Version=1.0
Type=Application
Name=Remote Commander Browser
Comment=Remote Commander Browser native CEF shell
Exec=$LAUNCHER
TryExec=$LAUNCHER
Icon=remote-commander-browser
Terminal=false
Categories=Network;Utility;
StartupWMClass=ChatGPT-CEF-V2
StartupNotify=false
DESKTOP
chmod 0644 "$DESKTOP"

printf 'INSTALL=PASS\nRUNTIME=%s\nLAUNCHER=%s\nBINARY_SHA256=%s\n' "$RUNTIME" "$LAUNCHER" "$BIN_SHA"
printf 'Profile data is stored separately at ~/.config/chatgpt-cef-v2 and is not modified by this installer.\n'
