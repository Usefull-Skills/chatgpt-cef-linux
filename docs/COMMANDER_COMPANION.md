# Native Commander companion candidate

Built on the pinned v0.7.1 source. This is an optional native read-only panel,
not a ChatGPT page script, MCP execution bridge, account detector or installed
Commander upgrade. Click **Commander** or press **Ctrl+Shift+C** to toggle it.
The browser view is resized beside the panel, not covered by it. Narrow windows
and fullscreen hide the panel. Rows have full tooltips and accessible names.

## Evidence rather than inferred access

The panel separately displays backend runtime/profile/tool catalog and observed
current-chat tools/plugins/skills. Unknown identity or exposure stays UNKNOWN /
UNPROVEN. A device's display name, installed plugin title or server-side FULL_POWER
flag does not prove that this chat can call a tool. The active main-frame URL is
read from CEF; navigation/loading invalidates a claimed session match.

An independent private `commander-binding.json` must contain exactly
`appId`, `accountId`, `profileId`, `workflowId`, `projectRoot`, `chatUrl`, using
authoritative values or null for unknowns. Missing binding or any conflict cannot
authorize execution. The panel has no action controls, shell, network calls,
cookie access or page-JavaScript bridge.

## Local observation files

The matching Commander candidate exports bounded observations into a dedicated
same-user private profile. See its `docs/COMPANION.md` for the operator setup.
Neither program auto-generates account/App identity or changes your permissions.

Native Linux reads reject symlink components, unsafe writable ancestors,
foreign owner, broad file permissions, hardlinks, nonregular/oversized files,
duplicate JSON keys, invalid UTF-8, excessive nesting and identity drift.

Preferred snapshots have exact names
`commander-companion-<16-digit observedAtEpochMs>-<lowercase UUID>.json`.
Enumeration and candidates are bounded; more than 32 snapshots stops admission.
The newest admitted name is selected, its timestamp is checked against content,
and an invalid/expired newest file is **not** replaced with older evidence.
Legacy `commander-companion.json` is supported only if no immutable file exists.
Unknown files are ignored, never modified or deleted. Refresh only reads local
files; the one-second display tick expires already-read evidence without network
or model calls.

The source contract alone is not secure Windows-to-Linux transport. This
candidate does not copy private observations, run a daemon, resume projects,
correct bindings automatically or guarantee ChatGPT background streaming.

## Acceptance gates

The GitHub workflow first compiles the CEF-free contract with ASan/UBSan and
executes pure and actual Linux private-file regressions. The full build retains
the existing pinned CEF archive/hash. A non-root Xvfb test uses a fresh private
profile, preserves the Chromium sandbox and checks native toggle/geometry,
missing-data UNPROVEN, background self-test and close lifecycle. This test uses
no authenticated chat and does not prove actual ChatGPT tool exposure.

CI artifacts contain source/compiler logs and binary/archive hashes. A passing
source test is not an installed desktop, authenticated-session, long-duration,
Emad host or model-inference acceptance receipt. Do not promote this candidate
to FINAL until those separate operational gates are recorded.
