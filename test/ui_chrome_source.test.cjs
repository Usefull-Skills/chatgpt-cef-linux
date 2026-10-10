'use strict';
const test=require('node:test');
const assert=require('node:assert/strict');
const fs=require('node:fs');
const path=require('node:path');
const root=path.resolve(__dirname,'..');
const read=n=>fs.readFileSync(path.join(root,n),'utf8');
const controller=read('src/controller.cc');
const header=read('src/controller.h');
const layout=read('src/ui_chrome_layout.h');
test('native chrome height is one shared constant for header and BrowserView overlays',()=>{
 assert.match(controller,/constexpr int kHeaderHeight = 52;/);
 assert.match(controller,/new FixedPanelDelegate\(1240, kHeaderHeight\)/);
 assert.match(controller,/header_visible \? kHeaderHeight : 0/);
 assert.match(controller,/fullscreen_ \? 0 : kHeaderHeight/);
});
test('all eight restored tab states remain in the model, only controls are bounded',()=>{
 assert.match(layout,/SelectTabs\(int logical_width, int count, int active\)/);
 assert.match(controller,/tabs_\.size\(\) >= 8/);
 assert.match(controller,/!in_strip[\s\S]*tab\.ui_panel = nullptr/);
 assert.match(controller,/tab_switcher_button_->SetText\(label\)/);
 assert.match(controller,/kAccelTab1 \+ i/);
 assert.match(controller,/if \(id == kTabSwitcherButton\) \{ CycleTab\(1\); return; \}/);
});
test('tab interactions and labels are accessible and remain distinct from draggable regions',()=>{
 assert.match(controller,/SetAccessibleName\("Switch tabs"\)/);
 assert.match(controller,/SetAccessibleName\("Activate tab "/);
 assert.match(controller,/SetAccessibleName\("Close tab "/);
 assert.match(controller,/UpdateDraggableRegions\(\)/);
 assert.match(controller,/regions\.emplace_back/);
});
test('companion panel is viewport bounded and never silently hides data as healthy',()=>{
 assert.match(layout,/CompanionRowSlots/);
 assert.match(controller,/const bool overflow = static_cast<int>\(rows\.size\(\)\) > slots;/);
 assert.match(controller,/more • enlarge window/);
 assert.match(controller,/SetTooltipText\("Additional local monitor fields/);
 assert.match(controller,/caution \? kDangerText : kText/);
});
test('UI-only patch does not disable sandbox or touch ChatGPT authentication',()=>{
 assert.doesNotMatch(controller,/--no-sandbox|--disable-web-security|DeleteAllCookies|clearBrowsingData/);
 assert.match(controller,/IsChatGPTURL\(url\)/);
 assert.match(controller,/SaveSessionState\(\)/);
 assert.match(header,/visible_tab_capacity_/);
});
