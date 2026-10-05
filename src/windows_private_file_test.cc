#include "windows_private_file.h"

#if defined(_WIN32)
#include <windows.h>
#endif

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

void Check(bool condition, const char* message) {
  if (!condition) {
    std::cerr << "WINDOWS_PRIVATE_GUARD_FAIL " << message << "\n";
    std::exit(1);
  }
}

#if defined(_WIN32)
std::string Utf8(const std::wstring& input) {
  if (input.empty()) return {};
  const int count = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
      input.data(), static_cast<int>(input.size()), nullptr, 0, nullptr, nullptr);
  if (count <= 0) return {};
  std::string out(static_cast<size_t>(count), '\0');
  if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
      input.data(), static_cast<int>(input.size()), out.data(), count,
      nullptr, nullptr) != count) return {};
  return out;
}
#endif

}  // namespace

#if defined(_WIN32)
int wmain(int argc, wchar_t** argv) {
  Check(argc == 2 || argc == 3, "usage");
  const std::wstring root_w(argv[1]);
  const std::string root = Utf8(root_w);
  Check(!root.empty(), "root_utf8");

  std::string error;
  if (argc == 3) {
    Check(std::wstring(argv[2]) == L"--expect-binding-reject", "reject_mode_arg");
    const std::string rejected = companion::winprivate::ReadPrivateFile(
        root, "commander-binding.json", 65536, error);
    Check(rejected.empty() && error == "FILE_UNSAFE_OR_OVERSIZE",
          "broad_acl_binding_rejected");
    std::cout << "WINDOWS_PRIVATE_GUARD_REJECT_PASS\n";
    return 0;
  }

  const std::string binding = companion::winprivate::ReadPrivateFile(
      root, "commander-binding.json", 65536, error);
  Check(error.empty() && binding == "{\"binding\":true}\n", "private_file_read");

  const std::wstring binding_path = root_w + L"\\commander-binding.json";
  const std::wstring hardlink_path = root_w + L"\\hardlink-alias.json";
  DeleteFileW(hardlink_path.c_str());
  Check(CreateHardLinkW(hardlink_path.c_str(), binding_path.c_str(), nullptr) != FALSE,
        "create_hardlink");
  error.clear();
  Check(companion::winprivate::ReadPrivateFile(
            root, "commander-binding.json", 65536, error).empty() &&
        error == "FILE_UNSAFE_OR_OVERSIZE",
        "hardlink_rejected");
  Check(DeleteFileW(hardlink_path.c_str()) != FALSE, "delete_hardlink");

  const auto snapshot = companion::winprivate::ReadLatestSnapshot(root, 65536);
  Check(snapshot.error.empty(), "snapshot_error");
  Check(snapshot.selected_epoch_ms == 2000, "latest_epoch");
  Check(snapshot.selected_name ==
        "commander-companion-0000000000002000-00000000-0000-4000-8000-000000000002.json",
        "latest_name");
  Check(snapshot.bytes == "{\"snapshot\":2}\n", "latest_bytes");

  std::cout << "WINDOWS_PRIVATE_GUARD_PASS\n";
  return 0;
}
#else
int main() {
  std::cout << "WINDOWS_PRIVATE_GUARD_SKIPPED_NONWINDOWS\n";
  return 0;
}
#endif
