#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace companion::winprivate {

struct SnapshotRead {
  std::string bytes;
  std::string error;
  std::string selected_name;
  uint64_t selected_epoch_ms = 0;
};

std::string ReadPrivateFile(const std::string& root,
                            const char* name,
                            size_t cap,
                            std::string& error);
SnapshotRead ReadLatestSnapshot(const std::string& root, size_t cap);

}  // namespace companion::winprivate
