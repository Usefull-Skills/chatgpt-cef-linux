#include "../src/companion_monitor.h"

#include <cstdlib>
#include <iostream>
#include <limits>

namespace {
int checks = 0;
void Require(bool condition, const char* label) {
  ++checks;
  if (!condition) {
    std::cerr << "FAIL: " << label << '\n';
    std::exit(1);
  }
}
}

// CEF-free deterministic contract tests. No file reads, browser, model, network,
// credentials or sleep. Checks remain enabled in release builds.
int main() {
  using companion::MonitorSchedule;
  using companion::MonitorTicket;
  using companion::MonitorCompletion;
  MonitorSchedule monitor;
  MonitorTicket first;
  Require(!monitor.TryBegin(0, first), "not started");
  Require(!monitor.Start(0, 0), "zero owner rejected");
  Require(monitor.Start(101, 100), "start valid owner");
  Require(monitor.TryBegin(100, first), "first poll immediately due");
  MonitorTicket other;
  Require(!monitor.TryBegin(5000, other), "single flight even when overdue");
  Require(monitor.Complete({102, first.epoch, first.sequence}, 5000, true) ==
          MonitorCompletion::kUnknown, "different owner cannot release flight");
  Require(monitor.Complete(first, 1000, true) == MonitorCompletion::kAccepted,
          "exact successful completion");
  Require(!monitor.TryBegin(2999, other), "minimum cadence enforced");
  Require(monitor.TryBegin(3000, other), "poll at minimum cadence");
  Require(monitor.Complete(first, 3001, true) == MonitorCompletion::kUnknown,
          "replayed completion rejected");
  Require(monitor.Complete(other, 3000, false) == MonitorCompletion::kAccepted,
          "invalid newest result completes without fallback");
  Require(monitor.failure_count() == 1 && monitor.DelayUntilDue(3000) == 4000,
          "first failure bounded backoff");
  MonitorTicket missing;
  Require(!monitor.TryBegin(6999, missing), "manual request cannot defeat backoff");
  Require(monitor.TryBegin(7000, missing), "failure poll becomes due");
  Require(monitor.Start(101, 7500), "changed context gets new epoch");
  Require(!monitor.TryBegin(100000, other), "epoch change retains physical flight");
  Require(monitor.Complete(missing, 8000, true) == MonitorCompletion::kStale,
          "old epoch releases flight but cannot publish");
  Require(!monitor.TryBegin(9999, other), "stale completion retains minimum cadence");
  Require(monitor.TryBegin(10000, other), "new epoch can poll after old completion");
  monitor.Stop();
  Require(!monitor.running(), "stop monitor");
  Require(!monitor.TryBegin(100000, first), "stop cannot spawn a read");
  Require(monitor.Complete(other, 11000, true) == MonitorCompletion::kStale,
          "shutdown completion not accepted");
  Require(monitor.Start(303, 12000), "new owner context");
  for (int i = 0; i < 10; ++i) {
    const auto due = 12000 + static_cast<uint64_t>(i) * 40000;
    Require(monitor.TryBegin(due, first), "backoff series starts");
    Require(monitor.Complete(first, due, false) == MonitorCompletion::kAccepted,
            "backoff series completion");
  }
  Require(monitor.DelayUntilDue(372000) == 30000 && monitor.failure_count() == 4,
          "failure cadence and counter saturated");
  Require(monitor.TryBegin(402000, first), "poll after maximum backoff");
  Require(monitor.Complete(first, 402000, true) == MonitorCompletion::kAccepted &&
          monitor.failure_count() == 0 && monitor.DelayUntilDue(402000) == 2000,
          "recovery returns to minimum cadence");
  Require(monitor.Complete(first, 402000, true) == MonitorCompletion::kUnknown,
          "already-consumed ticket cannot replay");
  MonitorSchedule near_limit;
  const uint64_t near = std::numeric_limits<uint64_t>::max() - 1;
  Require(near_limit.Start(404, near) && near_limit.TryBegin(near, first),
          "near maximum monotonic time accepted without wrap");
  Require(near_limit.Complete(first, near, false) == MonitorCompletion::kAccepted &&
          near_limit.DelayUntilDue(near) == 1,
          "deadline saturates rather than overflow");
  Require(near_limit.TryBegin(std::numeric_limits<uint64_t>::max(), first),
          "saturated deadline remains representable");
  std::cout << "PASS companion_monitor_pure checks=" << checks << '\n';
}
