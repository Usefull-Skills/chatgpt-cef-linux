#include "windows_private_file.h"

#if defined(_WIN32)

#include <windows.h>
#include <aclapi.h>

#include <algorithm>
#include <array>
#include <string_view>
#include <vector>

namespace companion::winprivate {
namespace {

class Handle {
 public:
  Handle() = default;
  explicit Handle(HANDLE value) : value_(value) {}
  ~Handle() { if (valid()) CloseHandle(value_); }
  Handle(const Handle&) = delete;
  Handle& operator=(const Handle&) = delete;
  Handle(Handle&& other) noexcept : value_(other.release()) {}
  Handle& operator=(Handle&& other) noexcept {
    if (this != &other) { if (valid()) CloseHandle(value_); value_ = other.release(); }
    return *this;
  }
  bool valid() const { return value_ != nullptr && value_ != INVALID_HANDLE_VALUE; }
  HANDLE get() const { return value_; }
  HANDLE release() { HANDLE value = value_; value_ = INVALID_HANDLE_VALUE; return value; }
 private:
  HANDLE value_ = INVALID_HANDLE_VALUE;
};

struct Identity {
  DWORD volume = 0;
  DWORD index_high = 0;
  DWORD index_low = 0;
  DWORD attributes = 0;
  DWORD links = 0;
  DWORD size_high = 0;
  DWORD size_low = 0;
  FILETIME write{};
};

bool SameIdentity(const Identity& a, const Identity& b) {
  return a.volume == b.volume && a.index_high == b.index_high &&
         a.index_low == b.index_low && a.attributes == b.attributes &&
         a.links == b.links && a.size_high == b.size_high &&
         a.size_low == b.size_low &&
         CompareFileTime(&a.write, &b.write) == 0;
}

bool IdentityOf(HANDLE handle, Identity& out) {
  BY_HANDLE_FILE_INFORMATION info{};
  if (!GetFileInformationByHandle(handle, &info)) return false;
  out.volume = info.dwVolumeSerialNumber;
  out.index_high = info.nFileIndexHigh;
  out.index_low = info.nFileIndexLow;
  out.attributes = info.dwFileAttributes;
  out.links = info.nNumberOfLinks;
  out.size_high = info.nFileSizeHigh;
  out.size_low = info.nFileSizeLow;
  out.write = info.ftLastWriteTime;
  return true;
}

std::wstring Wide(const std::string& input) {
  if (input.empty()) return {};
  const int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                         input.data(), static_cast<int>(input.size()),
                                         nullptr, 0);
  if (count <= 0) return {};
  std::wstring out(static_cast<size_t>(count), L'\0');
  if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                          input.data(), static_cast<int>(input.size()),
                          out.data(), count) != count) return {};
  return out;
}

std::string Utf8(const std::wstring& input) {
  if (input.empty()) return {};
  const int count = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
                                        input.data(), static_cast<int>(input.size()),
                                        nullptr, 0, nullptr, nullptr);
  if (count <= 0) return {};
  std::string out(static_cast<size_t>(count), '\0');
  if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
                          input.data(), static_cast<int>(input.size()),
                          out.data(), count, nullptr, nullptr) != count) return {};
  return out;
}

bool SamePath(const std::wstring& a, const std::wstring& b) {
  return _wcsicmp(a.c_str(), b.c_str()) == 0;
}

std::wstring StripDevicePrefix(std::wstring value) {
  constexpr wchar_t prefix[] = L"\\\\?\\";
  if (value.rfind(prefix, 0) == 0) value.erase(0, 4);
  return value;
}

bool NormalizeRoot(const std::string& root, std::wstring& out) {
  std::wstring input = Wide(root);
  if (input.size() < 3 || input.size() > 32760 ||
      !((input[0] >= L'A' && input[0] <= L'Z') ||
        (input[0] >= L'a' && input[0] <= L'z')) ||
      input[1] != L':' || (input[2] != L'\\' && input[2] != L'/')) return false;
  std::vector<wchar_t> buffer(32768);
  const DWORD count = GetFullPathNameW(input.c_str(), static_cast<DWORD>(buffer.size()),
                                       buffer.data(), nullptr);
  if (!count || count >= buffer.size()) return false;
  out.assign(buffer.data(), count);
  std::replace(out.begin(), out.end(), L'/', L'\\');
  while (out.size() > 3 && out.back() == L'\\') out.pop_back();
  return out.size() >= 3;
}

bool AncestorsNoReparse(const std::wstring& root) {
  if (root.size() < 3) return false;
  std::wstring current = root.substr(0, 3);
  size_t position = 3;
  while (position < root.size()) {
    const size_t end = root.find(L'\\', position);
    const std::wstring part = root.substr(position, end == std::wstring::npos ? end : end - position);
    if (part.empty() || part == L"." || part == L"..") return false;
    if (current.size() > 3) current += L'\\';
    current += part;
    const DWORD attrs = GetFileAttributesW(current.c_str());
    if (attrs == INVALID_FILE_ATTRIBUTES || !(attrs & FILE_ATTRIBUTE_DIRECTORY) ||
        (attrs & FILE_ATTRIBUTE_REPARSE_POINT)) return false;
    if (end == std::wstring::npos) break;
    position = end + 1;
  }
  return true;
}

bool FinalPath(HANDLE handle, std::wstring& out) {
  std::vector<wchar_t> buffer(32768);
  const DWORD count = GetFinalPathNameByHandleW(
      handle, buffer.data(), static_cast<DWORD>(buffer.size()),
      FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
  if (!count || count >= buffer.size()) return false;
  out = StripDevicePrefix(std::wstring(buffer.data(), count));
  std::replace(out.begin(), out.end(), L'/', L'\\');
  while (out.size() > 3 && out.back() == L'\\') out.pop_back();
  return out.size() >= 3 && out[1] == L':';
}

bool CurrentUserSid(std::vector<BYTE>& storage, PSID& sid) {
  Handle token;
  HANDLE raw = nullptr;
  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &raw)) return false;
  token = Handle(raw);
  DWORD bytes = 0;
  GetTokenInformation(token.get(), TokenUser, nullptr, 0, &bytes);
  if (!bytes || GetLastError() != ERROR_INSUFFICIENT_BUFFER) return false;
  storage.resize(bytes);
  if (!GetTokenInformation(token.get(), TokenUser, storage.data(), bytes, &bytes)) return false;
  sid = reinterpret_cast<TOKEN_USER*>(storage.data())->User.Sid;
  return IsValidSid(sid) != FALSE;
}

bool PrivateAcl(HANDLE handle, bool directory) {
  PSID owner = nullptr;
  PACL dacl = nullptr;
  PSECURITY_DESCRIPTOR descriptor = nullptr;
  const DWORD result = GetSecurityInfo(handle, SE_FILE_OBJECT,
      OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
      &owner, nullptr, &dacl, nullptr, &descriptor);
  if (result != ERROR_SUCCESS || !descriptor || !owner || !dacl) {
    if (descriptor) LocalFree(descriptor);
    return false;
  }
  std::vector<BYTE> user_storage;
  PSID user = nullptr;
  if (!CurrentUserSid(user_storage, user) || !EqualSid(owner, user)) {
    LocalFree(descriptor); return false;
  }
  std::array<BYTE, SECURITY_MAX_SID_SIZE> system_storage{};
  std::array<BYTE, SECURITY_MAX_SID_SIZE> admins_storage{};
  DWORD system_size = static_cast<DWORD>(system_storage.size());
  DWORD admins_size = static_cast<DWORD>(admins_storage.size());
  PSID system = system_storage.data(), admins = admins_storage.data();
  if (!CreateWellKnownSid(WinLocalSystemSid, nullptr, system, &system_size) ||
      !CreateWellKnownSid(WinBuiltinAdministratorsSid, nullptr, admins, &admins_size)) {
    LocalFree(descriptor); return false;
  }
  bool user_read = false;
  ACL_SIZE_INFORMATION acl_info{};
  if (!GetAclInformation(dacl, &acl_info, sizeof(acl_info), AclSizeInformation)) {
    LocalFree(descriptor); return false;
  }
  if (acl_info.AceCount == 0 || acl_info.AceCount > 256) {
    LocalFree(descriptor); return false;
  }
  for (DWORD i = 0; i < acl_info.AceCount; ++i) {
    void* raw_ace = nullptr;
    if (!GetAce(dacl, i, &raw_ace) || !raw_ace) { LocalFree(descriptor); return false; }
    auto* header = static_cast<ACE_HEADER*>(raw_ace);
    if (header->AceType == ACCESS_ALLOWED_ACE_TYPE) {
      auto* ace = static_cast<ACCESS_ALLOWED_ACE*>(raw_ace);
      PSID ace_sid = reinterpret_cast<PSID>(&ace->SidStart);
      if (!IsValidSid(ace_sid)) { LocalFree(descriptor); return false; }
      const bool permitted = EqualSid(ace_sid, user) || EqualSid(ace_sid, system) ||
                             EqualSid(ace_sid, admins);
      if (ace->Mask && !permitted) { LocalFree(descriptor); return false; }
      if (EqualSid(ace_sid, user) &&
          (ace->Mask & (directory ? FILE_LIST_DIRECTORY : FILE_READ_DATA)))
        user_read = true;
    } else if (header->AceType == ACCESS_ALLOWED_OBJECT_ACE_TYPE ||
               header->AceType == ACCESS_ALLOWED_CALLBACK_ACE_TYPE ||
               header->AceType == ACCESS_ALLOWED_CALLBACK_OBJECT_ACE_TYPE) {
      LocalFree(descriptor); return false;
    }
  }
  LocalFree(descriptor);
  return user_read;
}

bool Ntfs(HANDLE handle) {
  std::array<wchar_t, 64> fs{};
  DWORD serial = 0, max_component = 0, flags = 0;
  return GetVolumeInformationByHandleW(handle, nullptr, 0, &serial, &max_component,
                                        &flags, fs.data(), static_cast<DWORD>(fs.size())) &&
         _wcsicmp(fs.data(), L"NTFS") == 0 &&
         (flags & FILE_PERSISTENT_ACLS) != 0;
}

Handle OpenRoot(const std::string& root, std::wstring& canonical,
                Identity& identity, std::string& error) {
  if (!NormalizeRoot(root, canonical) || !AncestorsNoReparse(canonical)) {
    error = "PROFILE_ROOT_INVALID"; return {};
  }
  Handle handle(CreateFileW(canonical.c_str(),
      FILE_LIST_DIRECTORY | FILE_READ_ATTRIBUTES | READ_CONTROL,
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
      OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
      nullptr));
  if (!handle.valid()) { error = "PROFILE_ROOT_UNAVAILABLE"; return {}; }
  Identity observed{};
  std::wstring final_path;
  if (!IdentityOf(handle.get(), observed) ||
      !(observed.attributes & FILE_ATTRIBUTE_DIRECTORY) ||
      (observed.attributes & FILE_ATTRIBUTE_REPARSE_POINT) ||
      !FinalPath(handle.get(), final_path) || !SamePath(final_path, canonical) ||
      !Ntfs(handle.get()) || !PrivateAcl(handle.get(), true)) {
    error = "PROFILE_ROOT_UNSAFE"; return {};
  }
  identity = observed; error.clear(); return handle;
}

bool RootUnchanged(const std::string& root, const Identity& before) {
  std::wstring canonical; Identity after{}; std::string error;
  Handle handle = OpenRoot(root, canonical, after, error);
  return handle.valid() && SameIdentity(before, after);
}

bool SnapshotName(const std::string& name, uint64_t& observed_at) {
  const std::string prefix = "commander-companion-";
  constexpr size_t stamp = 16, uuid = 36;
  if (name.size() != prefix.size() + stamp + 1 + uuid + 5 ||
      name.compare(0, prefix.size(), prefix) != 0 ||
      name.compare(name.size() - 5, 5, ".json") != 0) return false;
  observed_at = 0;
  for (size_t i = prefix.size(); i < prefix.size() + stamp; ++i) {
    if (name[i] < '0' || name[i] > '9') return false;
    const unsigned digit = static_cast<unsigned>(name[i] - '0');
    if (observed_at > (9007199254740991ULL - digit) / 10) return false;
    observed_at = observed_at * 10 + digit;
  }
  if (!observed_at || name[prefix.size() + stamp] != '-') return false;
  const size_t start = prefix.size() + stamp + 1;
  for (size_t i = 0; i < uuid; ++i) {
    const char value = name[start + i];
    if (i == 8 || i == 13 || i == 18 || i == 23) {
      if (value != '-') return false;
    } else if (!((value >= '0' && value <= '9') ||
                 (value >= 'a' && value <= 'f'))) return false;
  }
  return true;
}

}  // namespace

std::string ReadPrivateFile(const std::string& root, const char* name,
                            size_t cap, std::string& error) {
  if (!name || !*name || std::string_view(name).find_first_of("\\/") != std::string_view::npos) {
    error = "FILE_NAME_INVALID"; return {};
  }
  std::wstring canonical; Identity root_before{};
  Handle directory = OpenRoot(root, canonical, root_before, error);
  if (!directory.valid()) return {};

  const std::wstring wname = Wide(name);
  if (wname.empty()) { error = "FILE_NAME_INVALID"; return {}; }
  const std::wstring path = canonical + L"\\" + wname;
  Handle file(CreateFileW(path.c_str(), GENERIC_READ | READ_CONTROL,
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT,
      nullptr));
  if (!file.valid()) {
    error = GetLastError() == ERROR_FILE_NOT_FOUND ? "FILE_MISSING" : "FILE_UNAVAILABLE";
    return {};
  }

  Identity before{};
  std::wstring final_path;
  if (!IdentityOf(file.get(), before) ||
      (before.attributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) ||
      before.links != 1 || before.size_high != 0 || before.size_low == 0 ||
      static_cast<uint64_t>(before.size_low) > cap ||
      !FinalPath(file.get(), final_path) || !SamePath(final_path, path) ||
      !Ntfs(file.get()) || !PrivateAcl(file.get(), false)) {
    error = "FILE_UNSAFE_OR_OVERSIZE"; return {};
  }

  std::string bytes(static_cast<size_t>(before.size_low), '\0');
  DWORD total = 0;
  while (total < before.size_low) {
    DWORD count = 0;
    if (!ReadFile(file.get(), bytes.data() + total, before.size_low - total,
                  &count, nullptr) || count == 0) {
      error = "FILE_READ_FAILED"; return {};
    }
    total += count;
  }

  Identity after{};
  std::wstring final_after;
  if (!IdentityOf(file.get(), after) || !SameIdentity(before, after) ||
      !FinalPath(file.get(), final_after) || !SamePath(final_after, path) ||
      !RootUnchanged(root, root_before)) {
    error = "FILE_CHANGED_DURING_READ"; return {};
  }
  error.clear(); return bytes;
}

SnapshotRead ReadLatestSnapshot(const std::string& root, size_t cap) {
  SnapshotRead result;
  std::wstring canonical; Identity root_before{}; std::string error;
  Handle directory = OpenRoot(root, canonical, root_before, error);
  if (!directory.valid()) { result.error = error; return result; }

  WIN32_FIND_DATAW data{};
  Handle find;
  HANDLE raw_find = FindFirstFileW((canonical + L"\\*").c_str(), &data);
  if (raw_find == INVALID_HANDLE_VALUE) {
    result.error = "SNAPSHOT_DIRECTORY_UNAVAILABLE"; return result;
  }
  size_t scanned = 0, recognized = 0;
  std::string latest;
  uint64_t latest_time = 0;
  bool more = true;
  while (more) {
    const std::wstring wide_name(data.cFileName);
    if (wide_name != L"." && wide_name != L"..") {
      if (++scanned > 4096) { error = "SNAPSHOT_DIRECTORY_ENTRY_LIMIT"; break; }
      const std::string name = Utf8(wide_name);
      if (name.empty()) { error = "SNAPSHOT_FILENAME_INVALID"; break; }
      uint64_t observed_at = 0;
      if (SnapshotName(name, observed_at)) {
        if (++recognized > 32) { error = "SNAPSHOT_CANDIDATE_LIMIT"; break; }
        if (name > latest) { latest = name; latest_time = observed_at; }
      }
    }
    if (!FindNextFileW(raw_find, &data)) {
      const DWORD last = GetLastError();
      if (last != ERROR_NO_MORE_FILES) error = "SNAPSHOT_DIRECTORY_READ_FAILED";
      more = false;
    }
  }
  FindClose(raw_find);
  if (!error.empty()) { result.error = error; return result; }
  if (!RootUnchanged(root, root_before)) {
    result.error = "PROFILE_DIRECTORY_CHANGED_DURING_READ"; return result;
  }

  result.bytes = ReadPrivateFile(root,
      latest.empty() ? "commander-companion.json" : latest.c_str(), cap, error);
  if (!error.empty()) { result.error = error; return result; }
  if (!RootUnchanged(root, root_before)) {
    result.bytes.clear(); result.error = "PROFILE_DIRECTORY_CHANGED_DURING_READ";
    return result;
  }
  result.selected_name = latest;
  result.selected_epoch_ms = latest_time;
  return result;
}

}  // namespace companion::winprivate

#else

namespace companion::winprivate {
std::string ReadPrivateFile(const std::string&, const char*, size_t, std::string& error) {
  error = "WINDOWS_FILE_GUARD_UNAVAILABLE"; return {};
}
SnapshotRead ReadLatestSnapshot(const std::string&, size_t) {
  SnapshotRead out; out.error = "WINDOWS_FILE_GUARD_UNAVAILABLE"; return out;
}
}  // namespace companion::winprivate

#endif
