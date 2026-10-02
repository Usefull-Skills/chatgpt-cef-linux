'use strict';

// Executable SOURCE-ONLY guards, not a C++ compiler, native CEF test, scheduler
// runtime substitute or live ChatGPT/background-continuation acceptance test.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const assert = require('node:assert/strict');
const root = path.resolve(__dirname, '..');
let checks = 0;
function requireGuard(value, label) {
  ++checks;
  assert.ok(value, label);
}
function read(relative) {
  const target = path.resolve(root, relative);
  requireGuard(target.startsWith(root + path.sep), 'file stays inside copied CEF root');
  const before = fs.lstatSync(target);
  requireGuard(before.isFile() && !before.isSymbolicLink() && before.size <= 65536,
               'bounded regular source before opening');
  const fd = fs.openSync(target, 'r');
  try {
    const opened = fs.fstatSync(fd);
    requireGuard(opened.isFile() && opened.dev === before.dev && opened.ino === before.ino &&
                 opened.size === before.size, 'source identity at open');
    const bytes = Buffer.alloc(before.size);
    let count = 0;
    while (count < bytes.length) {
      const amount = fs.readSync(fd, bytes, count, bytes.length - count, count);
      requireGuard(amount > 0, 'no unexpected source EOF');
      count += amount;
    }
    const after = fs.fstatSync(fd);
    const named = fs.lstatSync(target);
    requireGuard(after.size === before.size && after.mtimeMs === before.mtimeMs &&
                 after.ctimeMs === before.ctimeMs && named.dev === before.dev &&
                 named.ino === before.ino && !named.isSymbolicLink(),
                 'source identity/content metadata stable after finite read');
    return bytes.toString('utf8');
  } finally { fs.closeSync(fd); }
}
const header = read('src/companion_monitor.h');
const test = read('test/companion_monitor_test.cc');
const source = read('src/controller.cc');
const controllerHeader = read('src/controller.h');
const cmake = read('CMakeLists.txt');
const workflow = read('.github/workflows/build.yml');

// Lexical brace/string/comment screening only. It intentionally refuses raw
// literals instead of claiming a complete C++ parser or AST.
function screen(text) {
  requireGuard(!/R"/.test(text), 'no unhandled C++ raw string literal');
  const stack = [];
  let out = '', state = 'code';
  for (let i = 0; i < text.length; ++i) {
    const ch = text[i], next = text[i + 1];
    if (state === 'line') {
      if (ch === '\n') state = 'code';
      out += ch === '\n' ? '\n' : ' ';
    } else if (state === 'block') {
      if (ch === '*' && next === '/') { out += '  '; ++i; state = 'code'; }
      else out += ch === '\n' ? '\n' : ' ';
    } else if (state === 'string' || state === 'char') {
      if (ch === '\\') { out += '  '; ++i; }
      else {
        if (ch === (state === 'string' ? '"' : "'")) state = 'code';
        out += ch === '\n' ? '\n' : ' ';
      }
    } else if (ch === '/' && next === '/') { out += '  '; ++i; state = 'line'; }
    else if (ch === '/' && next === '*') { out += '  '; ++i; state = 'block'; }
    else if (ch === '"' || ch === "'") { out += ' '; state = ch === '"' ? 'string' : 'char'; }
    else {
      if ('{(['.includes(ch)) stack.push(ch);
      if ('})]'.includes(ch)) {
        requireGuard(stack.pop() === ({'}':'{', ')':'(', ']':'['})[ch], 'balanced source delimiters');
      }
      out += ch;
    }
  }
  requireGuard((state === 'code' || state === 'line') && stack.length === 0,
               'lexical EOF is complete');
  return out;
}
const maskedSource = screen(source);
screen(header); screen(test); screen(controllerHeader);
function method(name) {
  const token = 'void AppController::' + name + '(';
  const start = maskedSource.indexOf(token);
  requireGuard(start >= 0, 'exact method exists: ' + name);
  const body = maskedSource.indexOf('{', start);
  let level = 1, end = body + 1;
  while (level && end < maskedSource.length) {
    if (maskedSource[end] === '{') ++level;
    if (maskedSource[end] === '}') --level;
    ++end;
  }
  requireGuard(level === 0, 'complete method body: ' + name);
  return source.slice(body, end);
}
const tick = method('CompanionTick');
const refresh = method('RefreshCompanion');
const schedule = method('ScheduleCompanionTick');
const start = method('StartCompanionMonitor');
requireGuard(/kMinimumCadenceMs = 2000/.test(header), 'minimum cadence 2 seconds');
requireGuard(/kMaximumBackoffMs = 30000/.test(header), 'maximum backoff 30 seconds');
requireGuard(/failures_ < 4/.test(header), 'backoff counter saturation');
requireGuard(/!running_ \|\| in_flight_ \|\| now_ms < next_due_/.test(header), 'single-flight and due guard');
requireGuard(/!Same\(ticket, flight_\)/.test(header), 'exact flight ticket before release');
requireGuard(/ticket.owner == owner_/.test(header) && /ticket.epoch == epoch_/.test(header), 'owner and epoch publication guard');
requireGuard(/AddSaturated/.test(header), 'deadline overflow guarded');
const starts = header.slice(header.indexOf('bool Start('), header.indexOf('void Stop('));
requireGuard(!/in_flight_\s*=\s*false/.test(starts), 'context change retains physical flight');
requireGuard(!/(fstream|FILE|Cef|chrono|filesystem|system\(|getenv)/.test(header.replace(/\/\/[^\n]*/g, '')), 'scheduling contract is CEF-free and I/O-free');
requireGuard((source.match(/companion::ReadObservation\(/g) || []).length === 1 &&
             (source.match(/companion::ReadNativeBinding\(/g) || []).length === 1,
             'only one observation and binding read site');
const workerStart = tick.indexOf('CefPostTask(TID_FILE_BACKGROUND');
const workerEnd = tick.indexOf('if (!posted)', workerStart);
requireGuard(workerStart >= 0 && workerEnd > workerStart, 'file worker posting scope');
const worker = tick.slice(workerStart, workerEnd);
requireGuard(worker.includes('[read, root]') && !/\bthis\b|AppController::Get|window_|GetBrowser|GetMainFrame/.test(worker), 'worker captures mailbox/root only');
requireGuard(worker.includes('companion::ReadObservation(root)') && worker.includes('companion::ReadNativeBinding(root)'), 'bounded existing readers in file worker');
requireGuard(tick.includes('std::memory_order_acquire') && worker.includes('std::memory_order_release'), 'mailbox publication ordering');
requireGuard(tick.includes('MONITOR_FILE_TASK_POST_FAILED') && schedule.includes('MONITOR_UI_TASK_POST_FAILED'), 'both task posting failures surfaced');
requireGuard(schedule.includes('[owner, epoch]') && schedule.includes('companion_owner_id_ != owner') && schedule.includes('epoch() != epoch'), 'queued timer cannot target a new owner/window epoch');
requireGuard(schedule.includes('companion_tick_scheduled_') && schedule.includes('}), 1000)'), 'one UI timer with finite minimum delay');
requireGuard(!/companion_visible_/.test(tick + schedule + start), 'hidden panel does not suspend monitor');
requireGuard(!/ReadObservation|ReadNativeBinding/.test(refresh), 'refresh never reads files on UI thread');
requireGuard(tick.includes('SNAPSHOT_OBSERVATION_REGRESSED') && tick.includes('std::move(read->observation)'), 'invalid/regressed newest replaces prior display, no fallback');
requireGuard(method('BeginShutdown').includes('companion_monitor_.Stop()') && method('OnWindowDestroyed').includes('companion_monitor_.Stop()'), 'shutdown and destroyed window invalidate publication');
requireGuard(source.includes('g_companion_owner_sequence.compare_exchange_weak') && source.includes('return 0;'), 'owner identity does not wrap');
requireGuard(!/ExecuteJavaScript|LoadURL|Reload|SendKeyEvent|SendMouse|SetFocus|Activate\(|BringToTop|CreateBrowser|GetURL|CefURLRequest/.test(tick + schedule + start + refresh), 'monitor adds no page/input/focus/network operation');
requireGuard(test.includes('MonitorCompletion::kStale') && test.includes('MonitorCompletion::kUnknown') && test.includes('std::numeric_limits<uint64_t>::max()'), 'deterministic C++ tests authored for stale/replay/overflow');
const selftest = method('RunCompanionSelfTest');
requireGuard(selftest.includes('WAIT_READ_PENDING') && selftest.includes('monitor_runtime=UNPROVEN') &&
             !selftest.includes('ToggleCompanionPanel()') && !selftest.includes('RefreshCompanion()'),
             'layout seam cannot equate async pending data with runtime-monitor PASS');
requireGuard(cmake.includes('add_executable(companion-monitor-test test/companion_monitor_test.cc)') &&
             cmake.includes('add_test(NAME companion-monitor-schedule COMMAND companion-monitor-test)') &&
             cmake.includes('set_tests_properties(companion-monitor-schedule PROPERTIES TIMEOUT 10)') &&
             cmake.includes('src/companion_monitor.h'), 'CEF-free test and app header registered in candidate build');
requireGuard(workflow.includes('node test/companion_monitor_source_test.cjs') &&
             workflow.includes('ctest --test-dir build-companion') &&
             workflow.includes('CGWA_COMPANION_LAYOUT_SELFTEST PASS') &&
             workflow.includes('COMPANION_MONITOR_RUNTIME=UNPROVEN'), 'future CI preserves qualification boundaries');

const immutable = {
  'src/commander_companion.h': 'b90349588850d94fdc5a6d3ef8382a2e86531f81122268a853830e2e53f662db',
  'src/commander_companion.cc': 'e93274db37ad17d2087cf9d865105fa31f81d2dd07b1cdb0771a4e3e7f265f0b',
  'src/commander_companion_test.cc': 'bc1cd729c1b780e091691b990aaed595064dc9a2acb345dfc7cd84f95dce9bf7',
  'src/companion-emitted-fixture.json': 'a9e05da0274e582e7b4e4b5ff626213264c893bc531cd75e8e0324ed80a10f38'
};
for (const [name, expected] of Object.entries(immutable)) {
  const bytes = Buffer.from(read(name), 'utf8');
  requireGuard(crypto.createHash('sha256').update(bytes).digest('hex') === expected,
               'unchanged reader/parser/regression: ' + name);
}
console.log(JSON.stringify({status:'PASS_SOURCE_ONLY', checks,
  cpp_scheduler_execution:'NOT_RUN', native_cef:'NOT_RUN',
  browser_or_chat_interaction:false, effects:'bounded source reads only'}));
