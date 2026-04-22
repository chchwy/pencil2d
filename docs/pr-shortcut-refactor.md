# GitHub Issue

## Title

Shortcut system: consolidate scattered definitions into a single registry

## Labels

`enhancement`, `code quality`, `shortcuts`

## Body

### Problem

The keyboard shortcut system has its metadata scattered across multiple files with no single source of truth:

1. **Command IDs are `#define` macros in `pencildef.h`** (~112 `CMD_*` defines). These are untyped string literals with no compile-time safety — a typo like `CMD_TOLO_BRUSH` compiles silently and fails at runtime.

2. **Default shortcuts live in `kb.ini`**, a flat INI file embedded via `core_lib.qrc`. This is a second, disconnected source of truth that must be kept in sync with the macros by hand.

3. **Human-readable display names are in a hand-maintained `QHash` inside `shortcutspage.cpp`**. Every time a shortcut is added or renamed, three places must be updated: the macro, `kb.ini`, and the QHash — with no automated check that they agree.

4. **`pencilsettings.cpp` parses `kb.ini` at startup** to seed `QSettings`, duplicating default-value logic that could live in one place.

5. **No tests** verify that the set of defined commands matches what the UI wires up, so drift between these files is discovered only by users.

### Proposed solution

Introduce a `CommandRegistry` class in `core_lib` that is the **single source of truth** for every shortcut command's ID string, default key sequence, display name, and category. All consumers (`mainwindow2.cpp`, `shortcutspage.cpp`, `toolboxwidget.cpp`, `pencilsettings.cpp`) read from the registry instead of maintaining their own copies. The `CMD_*` macros and `kb.ini` file become unnecessary and are removed.

This is Phase 1 of a larger refactoring plan (see `docs/shortcut.md`). Future phases can add conflict detection, a shortcut bus (signal/slot), and data-driven UI wiring.

---

# Pull Request

## Title

refactor: introduce CommandRegistry as single source of truth for shortcuts

## Body

### Summary

Replaces the scattered shortcut metadata (`CMD_*` macros, `kb.ini`, display-name QHash) with a single `CommandRegistry` class that owns every command's ID, default shortcut, display name, and category.

### Motivation

See issue #___ (link the issue above). Three disconnected sources of truth made it easy to add a shortcut in one place and forget the others, with no test to catch the drift.

### What changed

| File | Change |
|---|---|
| `core_lib/src/util/commandregistry.h` | New `CommandRegistry` singleton, `CommandDefinition` struct, `CommandCategory` enum, `CmdId` namespace with typed `inline constexpr const char*` constants |
| `core_lib/src/util/commandregistry.cpp` | Static table of 114 command definitions (ID + default shortcut + display name + category) |
| `core_lib/src/util/pencildef.h` | Removed all ~112 `CMD_*` `#define` macros and dead `CMD_INCREASE_SIZE`/`CMD_DECREASE_SIZE` |
| `core_lib/src/util/pencilsettings.cpp` | `checkExistingShortcuts()` and `restoreShortcutsToDefault()` now iterate `CommandRegistry::instance().all()` instead of parsing `kb.ini` |
| `core_lib/data/core_lib.qrc` | Removed `kb.ini` resource entry |
| `app/src/shortcutspage.cpp` | Removed 112-entry `QHash` for display names; uses `CommandRegistry::find()` + `QCoreApplication::translate()` |
| `app/src/mainwindow2.cpp` | `setupKeyboardShortcuts()` uses `CmdId::` constants instead of `CMD_*` macros |
| `app/src/toolboxwidget.cpp` | Tooltip setup uses `CmdId::Tool*` constants instead of `CMD_TOOL_*` macros |
| `tests/src/test_commandregistry.cpp` | **New** — 9 Catch2 test cases (64 total, 1398 assertions all pass) covering: registry count, find, byCategory, every CmdId constant is registered, no duplicate IDs, no duplicate shortcuts, category coverage, and default shortcut spot-checks |
| `core_lib/core_lib.cmake`, `core_lib.pro`, `tests/tests.cmake` | Build system integration for new files |
| `docs/shortcut.md` | Refactoring plan document |

### Commits

1. `ecd1853` — docs: add shortcut system refactoring plan
2. `d016652` — refactor: add CommandRegistry as single source of truth for shortcuts (Phase 1)
3. `3c896ea` — cleanup: remove redundant shortcut display name QHash and kb.ini reference
4. `0f84015` — Remove all CMD_* macros from pencildef.h, replace with CmdId:: constants
5. `1bc0941` — style: apply formatter to toolboxwidget.cpp

### How to verify

```bash
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="C:/Qt/6.5.3/msvc2019_64"
cmake --build build --config Debug --parallel
./build/Debug/pencil2d_tests.exe
# Expected: All tests passed (1398 assertions in 64 test cases)
```

### Breaking changes

None. Shortcut string IDs stored in users' `QSettings` are unchanged (e.g. `"CmdNewFile"`, `"CmdToolBrush"`), so existing custom shortcuts are preserved.

### Future work (not in this PR)

- Phase 2: Shortcut conflict detection in the preferences UI
- Phase 3: Signal/slot shortcut bus to decouple mainwindow2 wiring
- Phase 4: Data-driven action-to-command binding (eliminate the manual `setupKeyboardShortcuts()` block)
