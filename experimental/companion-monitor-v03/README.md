# Companion monitor source overlay

This source-only snapshot preserves the V03 monitor development separately
from the previously qualified companion candidate. It is not a standalone CEF
application or an installed Windows/Linux build. It does not modify the main
application, top-level CI or default release build.

The overlay contains controller, contract, scheduler, fixture and source-test
files, plus proposed CMake and CI wiring. Those proposals remain inside this
experimental directory and are not activated by publication. Source checks do
not prove pinned CEF API compatibility, native lifecycle, background ChatGPT
recovery, cross-machine transport or long-duration operation.

Do not overlay or install automatically. A fresh independent review, exact
CEF compilation and same-source native/runtime qualification are required.
Existing failed or consumed historical runners are never replay authority.

See `docs/PROGRESS_20261002_FA.md` and
`docs/PUBLIC_SOURCE_MANIFEST_20261002.json` for scope and exact hashes.
