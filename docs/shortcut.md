# Shortcut System Refactoring Plan

## Status: Proposal

## Problem Statement

Adding a single new keyboard shortcut currently requires synchronized edits across **4 files**, with no compile-time or runtime safety net if any one is missed:

| Step | File | What to add |
|------|------|-------------|
| 1 | `core_lib/src/util/pencildef.h` | `#define CMD_FOO "CmdFoo"` |
| 2 | `core_lib/data/resources/kb.ini` | `CmdFoo=Ctrl+X` |
| 3 | `app/src/shortcutspage.cpp` | `{CMD_FOO, tr("Foo", "Shortcut")}` in a 100-entry QHash |
| 4 | `app/src/mainwindow2.cpp` | `ui->actionFoo->setShortcut(cmdKeySeq(CMD_FOO))` in a 100-line function |

Forgetting any one of these produces a silently broken or invisible shortcut. There are no automated tests for the shortcut system.

## Current Architecture

```
pencildef.h          114 CMD_* string macros (command IDs)
        │
        ▼
kb.ini               Default key bindings (Qt resource file)
        │
        ▼
pencilsettings.cpp   checkExistingShortcuts(): syncs kb.ini defaults → QSettings
                     restoreShortcutsToDefault(): overwrites QSettings from kb.ini
        │
        ▼
mainwindow2.cpp      setupKeyboardShortcuts(): reads QSettings, calls setShortcut()
                       on ~100 QActions one by one; installs ShortcutFilter on tools
                     clearKeyboardShortcuts(): zeros all action shortcuts
        │
        ▼
shortcutspage.cpp    Preferences UI: flat tree model, hardcoded QHash for display
                       names, O(n) conflict detection, save/load .pcls presets
        │
        ▼
shortcutfilter.cpp   Event filter: blocks tool shortcuts while mouse is in use
```

### Specific Issues

1. **Triple-redundant command data** — ID, default key, and display name are defined in 3 separate places with no coupling.
2. **No shortcut grouping** — The preferences UI uses a QTreeView but populates it as a flat list. 100+ shortcuts with no categories.
3. **Leak on reload** — `setupKeyboardShortcuts()` creates a new `ShortcutFilter` and installs it on every preference dialog close, without cleaning up the old one.
4. **Duplicate event filter install** — `ui->actionMove->installEventFilter(shortcutFilter)` is called twice (line ~1347 in mainwindow2.cpp).
5. **Inconsistent conflict detection** — `isKeySequenceExist()` compares `QKeySequence` objects but `removeDuplicateKeySequence()` compares portable-text strings.
6. **No tests** — Zero automated tests for shortcut loading, conflict detection, save/restore, or preset import/export.
7. **ShortcutFilter is trivially shallow** — 5 lines of logic, only checks mouse state, only guards tool shortcuts.

## Proposed Architecture

### Phase 1: Unified Command Registry

Introduce a `CommandDefinition` struct and a central registry that is the **single source of truth** for all command metadata.

```cpp
// core_lib/src/util/commandregistry.h

enum class CommandCategory {
    File,
    Edit,
    View,
    Animation,
    Tool,
    Layer,
    Window,
    Help
};

struct CommandDefinition {
    const char* id;              // "CmdNewFile"
    const char* defaultShortcut; // "Ctrl+N"
    const char* displayName;     // QT_TR_NOOP("New File")
    CommandCategory category;
};

class CommandRegistry {
public:
    static const CommandRegistry& instance();

    const CommandDefinition* find(const QString& cmdId) const;
    QVector<const CommandDefinition*> byCategory(CommandCategory cat) const;
    QVector<const CommandDefinition*> all() const;

private:
    CommandRegistry();
    QHash<QString, const CommandDefinition*> mLookup;
    static const CommandDefinition kDefinitions[];
};
```

**Key property**: Adding a new command becomes a single-line addition to `kDefinitions[]`. The ID, default shortcut, display name, and category are all co-located.

**Migration path for kb.ini**: The registry replaces kb.ini as the source of defaults. `checkExistingShortcuts()` reads defaults from the registry instead. kb.ini is kept temporarily for backward compatibility during the transition, then removed.

**Migration path for pencildef.h CMD_* macros**: The macros remain as thin aliases for string literals (they are used broadly). But they are no longer the authoritative source of shortcut metadata.

### Phase 2: ShortcutManager

Extract shortcut lifecycle management from `MainWindow2` into a dedicated class.

```cpp
// core_lib/src/managers/shortcutmanager.h

class ShortcutManager : public QObject {
    Q_OBJECT
public:
    explicit ShortcutManager(QObject* parent = nullptr);

    // Load shortcuts from QSettings, falling back to registry defaults
    void loadShortcuts();

    // Apply loaded shortcuts to a set of named QActions
    void applyToActions(const QHash<QString, QAction*>& actions);

    // Bind a command ID to an action (for dock widget toggles etc.)
    void bindAction(const QString& cmdId, QAction* action);

    // Get/set individual shortcuts
    QKeySequence shortcutFor(const QString& cmdId) const;
    void setShortcut(const QString& cmdId, const QKeySequence& seq);

    // Conflict detection
    QString conflictingCommand(const QString& excludeCmdId,
                               const QKeySequence& seq) const;

    // Persistence
    void saveToSettings();
    void restoreDefaults();
    void exportToFile(const QString& filePath);
    bool importFromFile(const QString& filePath);

signals:
    void shortcutsChanged();

private:
    QHash<QString, QKeySequence> mBindings; // cmdId → current key
};
```

**What MainWindow2 loses**: `setupKeyboardShortcuts()`, `clearKeyboardShortcuts()`, the `cmdKeySeq` lambda, and the ShortcutFilter creation.

**What MainWindow2 keeps**: Registering its QActions with the manager by command ID.

### Phase 3: Grouped Shortcuts UI

Replace the flat list in `ShortcutsPage` with a proper tree using `CommandCategory` as group nodes.

```
├── File
│   ├── New File                    Ctrl+N
│   ├── Open File                   Ctrl+O
│   └── ...
├── Edit
│   ├── Undo                        Ctrl+Z
│   └── ...
├── Tools
│   ├── Brush Tool                  B
│   └── ...
└── ...
```

The `getHumanReadableShortcutName()` QHash is deleted — display names come from `CommandRegistry`.

### Phase 4: Improved Conflict Detection & ShortcutFilter

- `ShortcutManager::conflictingCommand()` replaces both `isKeySequenceExist()` and `removeDuplicateKeySequence()` with consistent QKeySequence-based comparison.
- ShortcutFilter is extended to check additional blocking states (active modal dialog, text input focus) beyond just mouse-in-use.
- The duplicate event filter install is fixed.
- ShortcutFilter is created once and reused across reloads.

### Phase 5: Tests

| Test suite | What it verifies |
|------------|-----------------|
| `test_commandregistry` | All commands have non-empty ID, display name, and category. No duplicate IDs. |
| `test_shortcutmanager` | Load/save round-trip. Conflict detection catches exact duplicates. `restoreDefaults()` resets all bindings. Import/export produces valid .pcls files. |
| `test_shortcutspage` | (Integration) Tree model has correct group structure. Editing a shortcut updates the model. |

## Implementation Order

```
Phase 1 ─── CommandRegistry (core_lib)
  │            Single source of truth for IDs, defaults, names, categories.
  │            kb.ini still consulted for backward compat.
  │
Phase 2 ─── ShortcutManager (core_lib)
  │            Owns bindings, persistence, conflict detection.
  │            MainWindow2 delegates to it.
  │
Phase 3 ─── Grouped ShortcutsPage UI (app)
  │            Tree model reads from CommandRegistry categories.
  │            Edits go through ShortcutManager.
  │
Phase 4 ─── Conflict detection + ShortcutFilter fixes (app)
  │            Consistent comparison, richer context checks,
  │            fix duplicate install & leak.
  │
Phase 5 ─── Tests (tests)
  │            Registry invariants, manager round-trips, UI model.
  │
Cleanup ──── Remove kb.ini, delete getHumanReadableShortcutName(),
              simplify setupKeyboardShortcuts() to a registration loop.
```

Each phase is independently mergeable. Phases 1–2 are pure additions with no UI changes. Phase 3 is a UI improvement with no behavioral change. Phase 4 is bug fixes. Phase 5 is additive.

## Files Changed Per Phase

### Phase 1
- **New**: `core_lib/src/util/commandregistry.h`, `core_lib/src/util/commandregistry.cpp`
- **Modified**: `core_lib/core_lib.cmake` (add new files), `core_lib/src/util/pencilsettings.cpp` (read defaults from registry)

### Phase 2
- **New**: `core_lib/src/managers/shortcutmanager.h`, `core_lib/src/managers/shortcutmanager.cpp`
- **Modified**: `core_lib/core_lib.cmake`, `app/src/mainwindow2.cpp` (delegate to manager), `app/src/mainwindow2.h`

### Phase 3
- **Modified**: `app/src/shortcutspage.cpp` (grouped tree model), `app/src/shortcutspage.h`
- **Deleted content**: `getHumanReadableShortcutName()` QHash

### Phase 4
- **Modified**: `app/src/shortcutfilter.cpp`, `app/src/mainwindow2.cpp` (fix duplicate install, fix leak)

### Phase 5
- **New**: `tests/test_commandregistry.cpp`, `tests/test_shortcutmanager.cpp`
- **Modified**: `tests/tests.cmake` or equivalent

### Cleanup
- **Deleted**: `core_lib/data/resources/kb.ini` (defaults now in registry)
- **Modified**: `core_lib/data/core_lib.qrc` (remove kb.ini reference)

## Risks & Mitigations

| Risk | Mitigation |
|------|-----------|
| User's existing custom shortcuts in QSettings must survive the migration | Phase 1 preserves QSettings as the runtime source; registry is only the fallback for missing keys |
| .pcls preset files use the same key names | Command IDs (e.g. "CmdNewFile") are unchanged — presets remain compatible |
| Translation strings change location | Display names use `QT_TR_NOOP` in the registry; lupdate can extract from the new location. Run lupdate after Phase 1 to verify. |
| Large diff in Phase 3 (UI rewrite) | Phase 3 is cosmetic — group nodes are additive to the existing flat items. Can be reviewed independently. |
