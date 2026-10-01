# Security policy

## Supported release

Security fixes target the latest stable release branch.

## Sensitive data

Never attach or commit the runtime profile (`~/.config/chatgpt-cef-v2`), Chromium databases, cookies, authentication tokens, conversation exports, or unredacted diagnostic logs to a public issue.

## Reporting a vulnerability

Use the company's private security reporting channel for vulnerabilities involving authentication, sandbox escape, origin validation, permission bypass, credential exposure or arbitrary code execution. Do not publish working exploits or private account data in a public issue.

## Security invariants

Contributions must not:

- disable Chromium's sandbox as a default workaround
- auto-grant media permissions to arbitrary origins
- treat hostname prefixes as trusted origins
- copy cookies/sessions from another browser
- store credentials in source, configuration committed to Git, or issue attachments
- route untrusted external pages into privileged native behavior

## Dependency updates

CEF/Chromium updates are security-sensitive and must be treated as candidate releases until build, permission, navigation and runtime regression tests pass.
