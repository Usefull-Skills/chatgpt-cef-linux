#!/usr/bin/env bash
set -euo pipefail
umask 077

ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
VERSION=$(tr -d '[:space:]' < "$ROOT/VERSION")
BUILD="$ROOT/build/bin"

# rc.3 keeps the historical runtime/profile identity so existing authenticated
# browser state is never copied or migrated implicitly during the product rename.
LEGACY_ID="chatgpt-cef-v2"
PRODUCT_ID="remote-commander-browser"
APPROOT="$HOME/.local/share/$LEGACY_ID"
RUNTIME="$APPROOT/runtime-v$VERSION"
LAUNCHER="$HOME/.local/bin/$PRODUCT_ID"
LEGACY_LAUNCHER="$HOME/.local/bin/$LEGACY_ID"
DESKTOP="$HOME/.local/share/applications/$PRODUCT_ID.desktop"
LEGACY_DESKTOP="$HOME/.local/share/applications/$LEGACY_ID.desktop"

[[ -x "$BUILD/chatgpt-cef-v2" ]] || { printf 'Build first: ./scripts/build-release.sh\n' >&2; exit 20; }
[[ -f "$BUILD/chrome-sandbox" ]] || { printf 'Build output is missing chrome-sandbox.\n' >&2; exit 21; }
[[ ! -e "$RUNTIME" ]] || { printf 'Runtime already exists: %s\nRefusing to overwrite.\n' "$RUNTIME" >&2; exit 22; }

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
fail(){ printf 'Remote Commander Browser: %s\n' "\$1" >&2; exit "\$2"; }
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

rm -f "$LEGACY_LAUNCHER"
ln -s "$LAUNCHER" "$LEGACY_LAUNCHER"

cat > "$DESKTOP" <<DESKTOP
[Desktop Entry]
Version=1.0
Type=Application
Name=Remote Commander Browser
Comment=Hardened native browser shell for ChatGPT and Remote Commander
Exec=$LAUNCHER
TryExec=$LAUNCHER
Icon=applications-internet
Terminal=false
Categories=Network;Utility;
StartupWMClass=ChatGPT-CEF-V2
StartupNotify=false
DESKTOP
chmod 0644 "$DESKTOP"

rm -f "$LEGACY_DESKTOP"

printf 'INSTALL=PASS\nRUNTIME=%s\nLAUNCHER=%s\nLEGACY_ALIAS=%s\nBINARY_SHA256=%s\n' "$RUNTIME" "$LAUNCHER" "$LEGACY_LAUNCHER" "$BIN_SHA"
printf 'Legacy profile preserved at ~/.config/chatgpt-cef-v2; no session migration performed.\n'
