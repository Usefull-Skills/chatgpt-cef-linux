# Remote Commander Browser v0.8.0

## Stable release

v0.8.0 promotes the rc.9-qualified Browser line to the first stable 0.8 release.
The CEF runtime/source tree is unchanged from exact qualified rc.9 head
95b8c01b5169201e7a2d1955ab72530e9fe73827 except for stable release identity,
publication metadata and documentation.

Accepted evidence inherited from rc.9:
- hosted Release workflow PASS on exact rc.9 head;
- hosted Build/Linux native CEF build, dependency check, X11 companion/lifecycle test with sandbox intact, and artifact upload PASS;
- hosted Windows native preview PASS;
- 15-case release-integrity suite PASS on native Linux;
- generated Linux launcher variable deferral and tamper guards PASS;
- Windows release staging/integrity contracts PASS.

Stable versions publish without the prerelease flag and are marked latest.
Prerelease versions remain explicitly prerelease and latest=false.
Existing rc.8 and earlier tags/assets are immutable history and are not rewritten.

No saved-password extraction, user-profile reuse, hidden sign-in automation,
security sandbox bypass, or Commander authority expansion is introduced.
Publisher code signing remains external until a real signing identity is supplied.
