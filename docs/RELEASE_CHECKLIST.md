# Release checklist

- [ ] Source tree contains no user profile, cookies, tokens, logs or local paths.
- [ ] `scripts/static-qa.sh` passes.
- [ ] Clean `scripts/build-release.sh` passes from an empty build directory.
- [ ] `ldd build/bin/chatgpt-cef-v2` has no missing libraries.
- [ ] Sandbox is configured correctly for runtime testing.
- [ ] `scripts/runtime-self-test.sh` reports lifecycle PASS and clean shutdown.
- [ ] Manual login/session test passes.
- [ ] Manual upload/clipboard/voice test passes.
- [ ] RTL/LTR acceptance passes.
- [ ] Windowed/maximized/fullscreen pass.
- [ ] Single-instance behavior passes.
- [ ] `NOTICE.md` and `THIRD_PARTY_NOTICES.md` are present.
- [ ] Binary distribution includes CEF `LICENSE.txt` and `CREDITS.html`.
- [ ] Version and changelog updated.
- [ ] Future-feature issues reviewed; no experimental feature is silently added to the stable release.
- [ ] Release tag and source archive hashes recorded.
