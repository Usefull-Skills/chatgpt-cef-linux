#pragma once
namespace theme {
// UI preferences are local to the native Browser shell. No website/script
// is granted any command authority by this module.
bool ResolveDark(int mode, bool os_dark);
bool ReadSystemDark();
}
