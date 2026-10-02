#pragma once

#include <cstdint>
#include <limits>

// CEF-free, UI-thread-owned scheduling state. Time is supplied by a monotonic
// clock; this class performs no I/O and does not know whether a panel is visible.
namespace companion {
struct MonitorTicket {
  uint64_t owner = 0;
  uint64_t epoch = 0;
  uint64_t sequence = 0;
};

enum class MonitorCompletion { kUnknown, kStale, kAccepted };

class MonitorSchedule {
 public:
  static constexpr uint64_t kMinimumCadenceMs = 2000;
  static constexpr uint64_t kMaximumBackoffMs = 30000;

  // Context changes invalidate publication, not physical in-flight reads. The
  // outstanding ticket must complete before a second read can be dispatched.
  bool Start(uint64_t owner, uint64_t now_ms) {
    if (!owner || epoch_ == std::numeric_limits<uint64_t>::max()) {
      running_ = false;
      return false;
    }
    owner_ = owner;
    ++epoch_;
    running_ = true;
    failures_ = 0;
    next_due_ = now_ms;
    return true;
  }

  void Stop() {
    running_ = false;
    if (epoch_ != std::numeric_limits<uint64_t>::max()) ++epoch_;
  }

  bool TryBegin(uint64_t now_ms, MonitorTicket& ticket) {
    if (!running_ || in_flight_ || now_ms < next_due_) return false;
    if (sequence_ == std::numeric_limits<uint64_t>::max()) {
      running_ = false;
      return false;
    }
    flight_ = {owner_, epoch_, ++sequence_};
    in_flight_ = true;
    ticket = flight_;
    return true;
  }

  MonitorCompletion Complete(const MonitorTicket& ticket, uint64_t now_ms,
                             bool usable_newest_evidence) {
    if (!in_flight_ || !Same(ticket, flight_))
      return MonitorCompletion::kUnknown;
    in_flight_ = false;
    const bool current = running_ && ticket.owner == owner_ &&
                         ticket.epoch == epoch_;
    if (!current) {
      next_due_ = AddSaturated(now_ms, kMinimumCadenceMs);
      return MonitorCompletion::kStale;
    }
    if (usable_newest_evidence) failures_ = 0;
    else if (failures_ < 4) ++failures_;
    uint64_t cadence = kMinimumCadenceMs << failures_;
    if (cadence > kMaximumBackoffMs) cadence = kMaximumBackoffMs;
    next_due_ = AddSaturated(now_ms, cadence);
    return MonitorCompletion::kAccepted;
  }

  uint64_t DelayUntilDue(uint64_t now_ms) const {
    return now_ms >= next_due_ ? 0 : next_due_ - now_ms;
  }
  bool running() const { return running_; }
  bool in_flight() const { return in_flight_; }
  uint64_t epoch() const { return epoch_; }
  unsigned failure_count() const { return failures_; }

 private:
  static bool Same(const MonitorTicket& a, const MonitorTicket& b) {
    return a.owner == b.owner && a.epoch == b.epoch && a.sequence == b.sequence;
  }
  static uint64_t AddSaturated(uint64_t a, uint64_t b) {
    const auto limit = std::numeric_limits<uint64_t>::max();
    return a > limit - b ? limit : a + b;
  }
  uint64_t owner_ = 0;
  uint64_t epoch_ = 0;
  uint64_t sequence_ = 0;
  uint64_t next_due_ = 0;
  unsigned failures_ = 0;
  bool running_ = false;
  bool in_flight_ = false;
  MonitorTicket flight_;
};
}  // namespace companion
