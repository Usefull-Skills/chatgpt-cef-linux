# Contributing

## Development principles

1. Preserve the security boundary before adding convenience features.
2. Keep authenticated browser data outside the repository and test artifacts.
3. Prefer small, reviewable revisions with explicit rollback.
4. Add or update acceptance tests when changing lifecycle, permissions, navigation, RTL or window geometry.
5. Do not silently broaden DOM selectors or trusted origins.

## Before opening a pull request

```bash
./scripts/static-qa.sh
./scripts/build-release.sh
```

When an X11 runtime environment is available and the sandbox is configured:

```bash
./scripts/runtime-self-test.sh
```

Describe security/privacy implications and user-visible behavior in the PR.

## Commit hygiene

Do not commit generated CEF binaries, browser profiles, logs, screenshots with personal content, secrets, local absolute paths or build directories.
