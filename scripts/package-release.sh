#!/usr/bin/env bash
set -euo pipefail
umask 022

ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
VERSION=$(tr -d '[:space:]' < "$ROOT/VERSION")
BUILD="$ROOT/build/bin"
CEF="$ROOT/.deps/cef"
DIST="$ROOT/dist"
NAME="remote-commander-browser-v${VERSION}-linux-x86_64"
STAGE="$DIST/$NAME"
ARCHIVE="$DIST/$NAME.tar.gz"
ICON_SOURCE="$ROOT/assets/remote-commander-browser-logo.png"
ICON_SHA256="d724415693a4a4da8c20a065db978467ae979714d9b0559ace7dc33035461195"

[[ -x "$BUILD/chatgpt-cef-v2" ]] || { printf 'Build first.\n' >&2; exit 20; }
[[ -f "$CEF/LICENSE.txt" && -f "$CEF/CREDITS.html" ]] || { printf 'Pinned CEF license/credits missing.\n' >&2; exit 21; }
[[ -f "$ICON_SOURCE" && ! -L "$ICON_SOURCE" ]] || { echo 'Official Browser icon unavailable' >&2; exit 22; }
[[ "$(sha256sum "$ICON_SOURCE" | awk '{print $1}')" == "$ICON_SHA256" ]] || { echo 'Official Browser icon hash mismatch' >&2; exit 23; }

rm -rf "$STAGE"
mkdir -p "$STAGE/runtime" "$STAGE/third_party" "$STAGE/assets"
cp "$ICON_SOURCE" "$STAGE/assets/remote-commander-browser-logo.png"
cp -a "$BUILD/." "$STAGE/runtime/"
cp "$CEF/LICENSE.txt" "$STAGE/third_party/CEF_LICENSE.txt"
cp "$CEF/CREDITS.html" "$STAGE/third_party/CEF_CREDITS.html"
cp "$ROOT/NOTICE.md" "$ROOT/THIRD_PARTY_NOTICES.md" "$ROOT/RELEASE_NOTES.md" "$STAGE/"

cat > "$STAGE/install.sh" <<INSTALL
#!/usr/bin/env bash
set -euo pipefail
umask 077
HERE=\$(cd "\$(dirname "\${BASH_SOURCE[0]}")" && pwd)
TARGET="\$HOME/.local/share/chatgpt-cef-v2/runtime-v${VERSION}"
LAUNCHER="\$HOME/.local/bin/chatgpt-cef-v2"
DESKTOP="\$HOME/.local/share/applications/chatgpt-cef-v2.desktop"
ICON_SOURCE="\$HERE/assets/remote-commander-browser-logo.png"
ICON_TARGET="\$HOME/.local/share/icons/hicolor/512x512/apps/remote-commander-browser.png"
EXPECTED_ICON_SHA="d724415693a4a4da8c20a065db978467ae979714d9b0559ace7dc33035461195"
[[ ! -e "\$TARGET" ]] || { echo "Target exists: \$TARGET" >&2; exit 20; }
[[ -f "\$ICON_SOURCE" && ! -L "\$ICON_SOURCE" ]] || { echo "Embedded icon missing" >&2; exit 25; }
[[ "\$(sha256sum "\$ICON_SOURCE" | awk '{print \$1}')" == "\$EXPECTED_ICON_SHA" ]] || { echo "Embedded icon digest mismatch" >&2; exit 26; }
if [[ -e "\$ICON_TARGET" || -L "\$ICON_TARGET" ]]; then
  [[ -f "\$ICON_TARGET" && ! -L "\$ICON_TARGET" ]] || { echo "Unsafe installed icon" >&2; exit 27; }
  [[ "\$(sha256sum "\$ICON_TARGET" | awk '{print \$1}')" == "\$EXPECTED_ICON_SHA" ]] || { echo "Unknown installed icon, refusing overwrite" >&2; exit 28; }
fi
mkdir -p "\$(dirname "\$TARGET")" "\$HOME/.local/bin" "\$HOME/.local/share/applications"
cp -a "\$HERE/runtime" "\$TARGET"
sudo chown root:root "\$TARGET/chrome-sandbox"
sudo chmod 4755 "\$TARGET/chrome-sandbox"
BIN_SHA=\$(sha256sum "\$TARGET/chatgpt-cef-v2" | awk '{print \$1}')
SAN_SHA=\$(sha256sum "\$TARGET/chrome-sandbox" | awk '{print \$1}')
{
  printf '%s\n' '#!/usr/bin/env bash' 'set -euo pipefail' 'umask 077'
  printf 'RUNTIME=%q\nEXPECTED_BIN_SHA=%q\nEXPECTED_SANDBOX_SHA=%q\n' "\$TARGET" "\$BIN_SHA" "\$SAN_SHA"
  cat <<'LAUNCH'
BIN="\$RUNTIME/chatgpt-cef-v2"
SANDBOX="\$RUNTIME/chrome-sandbox"
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
} > "\$LAUNCHER"
chmod 0755 "\$LAUNCHER"
install -d -m 0755 "\$HOME/.local/share/icons/hicolor/512x512/apps"
if [[ ! -e "\$ICON_TARGET" ]]; then install -m 0644 "\$ICON_SOURCE" "\$ICON_TARGET"; fi
[[ "\$(sha256sum "\$ICON_TARGET" | awk '{print \$1}')" == "\$EXPECTED_ICON_SHA" ]] || exit 29
cat > "\$DESKTOP" <<DESKTOP
[Desktop Entry]
Version=1.0
Type=Application
Name=Remote Commander Browser
Comment=Remote Commander Browser native CEF shell
Exec=\$LAUNCHER
TryExec=\$LAUNCHER
Icon=remote-commander-browser
Terminal=false
Categories=Network;Utility;
StartupWMClass=ChatGPT-CEF-V2
StartupNotify=false
DESKTOP
chmod 0644 "\$DESKTOP"
echo "INSTALL=PASS"
echo "RUNTIME=\$TARGET"
echo "BINARY_SHA256=\$BIN_SHA"
INSTALL

cat > "$STAGE/uninstall.sh" <<UNINSTALL
#!/usr/bin/env bash
set -euo pipefail
TARGET="\$HOME/.local/share/chatgpt-cef-v2/runtime-v${VERSION}"
rm -f "\$HOME/.local/bin/chatgpt-cef-v2" "\$HOME/.local/share/applications/chatgpt-cef-v2.desktop"
if [[ -d "\$TARGET" ]]; then
  [[ ! -e "\$TARGET/chrome-sandbox" ]] || sudo rm -f "\$TARGET/chrome-sandbox"
  rm -rf "\$TARGET"
fi
echo "UNINSTALL=PASS"
echo "Profile preserved at: \$HOME/.config/chatgpt-cef-v2"
UNINSTALL
chmod 0755 "$STAGE/install.sh" "$STAGE/uninstall.sh"

cat > "$STAGE/README.txt" <<'TXT'
Unofficial third-party ChatGPT CEF client for Linux.
Run ./install.sh to install the runtime and configure Chromium's SUID sandbox.
The installer requires sudo only to set chrome-sandbox to root:root mode 4755.
The ChatGPT browser profile is stored separately under ~/.config/chatgpt-cef-v2.
See NOTICE.md and THIRD_PARTY_NOTICES.md.
TXT

(
  cd "$STAGE"
  find . -type f ! -name SHA256SUMS.txt -print0 | sort -z | xargs -0 sha256sum
) > "$STAGE/SHA256SUMS.txt"
(cd "$STAGE" && sha256sum -c SHA256SUMS.txt >/dev/null)

rm -f "$ARCHIVE" "$ARCHIVE.sha256"
tar -C "$DIST" -czf "$ARCHIVE" "$NAME"
(cd "$DIST" && sha256sum "$NAME.tar.gz" > "$NAME.tar.gz.sha256")
printf 'PACKAGE=%s\nSHA256=%s\n' "$ARCHIVE" "$(awk '{print $1}' "$ARCHIVE.sha256")"
