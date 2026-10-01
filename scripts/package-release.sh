#!/usr/bin/env bash
set -euo pipefail
umask 022

ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
VERSION=$(tr -d '[:space:]' < "$ROOT/VERSION")
BUILD="$ROOT/build/bin"
CEF="$ROOT/.deps/cef"
DIST="$ROOT/dist"
NAME="chatgpt-cef-v2-v${VERSION}-linux-x86_64"
STAGE="$DIST/$NAME"
ARCHIVE="$DIST/$NAME.tar.gz"

[[ -x "$BUILD/chatgpt-cef-v2" ]] || { printf 'Build first.\n' >&2; exit 20; }
[[ -f "$CEF/LICENSE.txt" && -f "$CEF/CREDITS.html" ]] || { printf 'Pinned CEF license/credits missing.\n' >&2; exit 21; }

rm -rf "$STAGE"
mkdir -p "$STAGE/runtime" "$STAGE/third_party"
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
[[ ! -e "\$TARGET" ]] || { echo "Target exists: \$TARGET" >&2; exit 20; }
mkdir -p "\$(dirname "\$TARGET")" "\$HOME/.local/bin" "\$HOME/.local/share/applications"
cp -a "\$HERE/runtime" "\$TARGET"
sudo chown root:root "\$TARGET/chrome-sandbox"
sudo chmod 4755 "\$TARGET/chrome-sandbox"
BIN_SHA=\$(sha256sum "\$TARGET/chatgpt-cef-v2" | awk '{print \$1}')
SAN_SHA=\$(sha256sum "\$TARGET/chrome-sandbox" | awk '{print \$1}')
cat > "\$LAUNCHER" <<LAUNCH
#!/usr/bin/env bash
set -euo pipefail
umask 077
RUNTIME="\$HOME/.local/share/chatgpt-cef-v2/runtime-v${VERSION}"
BIN="\$RUNTIME/chatgpt-cef-v2"
SANDBOX="\$RUNTIME/chrome-sandbox"
EXPECTED_BIN_SHA="\$BIN_SHA"
EXPECTED_SANDBOX_SHA="\$SAN_SHA"
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
chmod 0755 "\$LAUNCHER"
cat > "\$DESKTOP" <<DESKTOP
[Desktop Entry]
Version=1.0
Type=Application
Name=ChatGPT CEF V2
Comment=Unofficial hardened ChatGPT CEF client for Linux
Exec=\$LAUNCHER
TryExec=\$LAUNCHER
Icon=applications-internet
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
sha256sum "$ARCHIVE" > "$ARCHIVE.sha256"
printf 'PACKAGE=%s\nSHA256=%s\n' "$ARCHIVE" "$(awk '{print $1}' "$ARCHIVE.sha256")"
