#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

// A native, read-only observation contract. No browser JavaScript, network,
// command execution, account cookies, or credentials are accepted by this API.
namespace companion {
constexpr size_t kMaximumSnapshotBytes = 65536;
constexpr size_t kMaximumNativeBindingBytes = 32768;
constexpr uint64_t kMaximumLifetimeMs = 300000;

struct Binding {
  std::string app_id, account_id, profile_id, workflow_id, project_root, chat_url;
};

struct Observation {
  bool valid = false;
  bool blocked = false;
  std::string error = "SNAPSHOT_MISSING";
  uint64_t observed_at = 0, expires_at = 0, route_generation = 0, revision = 0;
  std::string app_id, profile, device_name, version, commit, config_sha256;
  std::string backend_state, tool_catalog_sha256, chat_state, conversation_url;
  std::string chat_app_id, project_state, project_id, project_root, next_action;
  std::string evidence_sha256, reconciliation_state;
  std::vector<std::string> backend_tools, chat_tools, skills, plugins, reasons;
  Binding binding;
};

struct NativeBinding {
  bool valid = false;
  std::string error = "NATIVE_BINDING_MISSING";
  Binding binding;
};

struct DisplayRow { std::string text, detail; };

// Pure deterministic parser/validation. Duplicate keys (including escaped
// aliases), unknown keys, invalid UTF-8, excessive depth/nodes and size fail shut.
Observation ParseObservation(const std::string& json);
NativeBinding ParseNativeBinding(const std::string& json);
std::string CatalogSha256(std::vector<std::string> names);
bool IsCanonicalChatUrl(const std::string& url);
bool IsCanonicalProjectRoot(const std::string& root);
bool SameProjectRoot(const std::string& a, const std::string& b);
bool IsFresh(const Observation& observation, uint64_t now_ms);
bool ImmutableSnapshotName(const std::string& name, uint64_t& observed_at);
std::vector<DisplayRow> Describe(const Observation& observation,
                                 const NativeBinding& expected,
                                 const std::string& active_url,
                                 uint64_t now_ms);

// Linux bounded immutable-candidate reads, newest lexicographic candidate only;
// legacy fixed file only when zero immutable names exist. Same-owner private
// regular files, no links, no symlink path components, identity recheck.
Observation ReadObservation(const std::string& profile_root);
NativeBinding ReadNativeBinding(const std::string& profile_root);
std::string ProfileRoot();
uint64_t NowEpochMs();
}  // namespace companion
