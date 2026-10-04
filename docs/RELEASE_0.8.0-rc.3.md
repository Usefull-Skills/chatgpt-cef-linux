# Remote Commander Browser v0.8.0-rc.3

## Purpose

rc.3 is the cross-platform reconciliation and branding candidate. It carries the qualified Windows native/private-file work forward without rewriting the published rc.1/rc.2 history.

## Product identity

- Product: **Remote Commander Browser**
- Target repository: `Usefull-Skills/remote-commander-browser`
- Legacy internal compatibility identity retained for rc.3: `chatgpt-cef-v2`

The compatibility identity is deliberately retained so browser profiles and authenticated state are not implicitly migrated.

## Required gates

- static QA
- companion contract tests
- Linux native release build/lifecycle
- Windows native build
- Windows private-file ACL guard
- Windows lifecycle self-test
- Windows/Linux package checks
- real native UI observation and basic interaction
- read-only Commander companion observation
- isolated test-profile acceptance
- sustained/fault/non-interference validation before stable promotion

## Stable boundary

`v0.7.1` remains the stable rollback channel until all whole-product gates are evidenced. rc.3 publication, if accepted, is a preview and is not by itself a claim of autonomous ChatGPT continuation.
