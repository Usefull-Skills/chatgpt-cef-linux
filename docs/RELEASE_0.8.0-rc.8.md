# Remote Commander Browser v0.8.0-rc.8

## Scope

rc.8 keeps the qualified rc.7 Browser runtime, ACL, profile/session behavior and installer semantics unchanged. It replaces ambiguous multi-path release artifact collection with an explicit four-file Windows staging contract and exercises the tag-release build path on release-related pull requests.

## Release-infrastructure root cause

Three consecutive release-infrastructure failures exposed distinct assumptions that pre-tag product qualification did not exercise: a mutable compiler feed (rc.5), an unquoted compiler install path (rc.6), and release artifact layout (rc.7). Official `actions/upload-artifact` documentation states that wildcard hierarchy and multi-path common-root behavior affect stored artifact structure. Release filenames are a product contract, so rc.8 no longer relies on multi-path inference.

The Windows release job now:

- builds the same ZIP and standalone Setup;
- creates the Setup checksum;
- copies exactly the ZIP, ZIP checksum, Setup and Setup checksum into `dist/release-windows`;
- requires the staging directory to contain exactly those four filenames;
- uploads that single staging directory using one wildcard pattern;
- uses compression level 0 for already-compressed large release payloads.

The Release workflow also runs its Windows/Linux build jobs on release-related pull requests while the publish job is explicitly tag-only. This moves release-layout validation before promotion without adding a second release implementation.

## Qualification inherited

Browser runtime, private ACL handling, persistent/isolated profile ownership, silent isolated Setup acceptance and Linux regression remain inherited from the rc.7 exact tree and existing exact-head gates.

## Artifact integrity

The tag publish job still creates and verifies aggregate `SHA256SUMS.txt` before creating the GitHub Release.

## Signing

The Browser Setup remains unsigned because no project code-signing certificate/private key is available. Publisher signing remains an external gate.
