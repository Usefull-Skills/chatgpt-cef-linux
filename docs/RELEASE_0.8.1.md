# Remote Commander Browser v0.8.1

## Live Commander monitor

v0.8.1 adds the missing local Browser-to-Commander monitoring path. Remote Commander Core publishes a short-lived owner-private monitor snapshot; the native CEF panel reads it without network calls, shell execution, credentials, cookies, or Browser-origin command authority.

The monitor displays:
- exact local Core version/device/profile/port;
- GUI backend availability and screenshot/mouse/keyboard/focus capabilities;
- Commander-owned background-browser readiness;
- durable workflow count/engine state;
- installed Agent Extension count/ids;
- active operation and lock counts;
- a bounded freshness TTL so stale Core state becomes DISCONNECTED instead of being shown as current.

The existing chat/project binding panel remains separately fail-closed and read-only. A system tool catalog is not treated as proof of current-chat exposure.

Security properties retained:
- no sandbox bypass;
- no saved-password extraction;
- no user-profile reset or hidden sign-in;
- no browser JavaScript bridge to Commander;
- private-file guards reject links, unsafe permissions and invalid schemas;
- monitor schema rejects command/action fields.

The release carries forward the stable v0.8.0 CEF 154 line and its session-preserving installer model.
