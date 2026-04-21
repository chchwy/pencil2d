# Test Coverage Analysis

**Date:** 2026-03-30
**Estimated Coverage:** ~7-10% by lines of code (15 test files, ~3,400 lines of test code vs ~33,900 lines of core code)

## Current Test Infrastructure

- **Framework:** Catch (single-header C++ testing library)
- **Location:** `tests/src/`
- **Build:** CMake (`tests/tests.cmake`)
- **CI:** GitHub Actions — runs on Linux, macOS, Windows with Qt 5 and Qt 6
- **No coverage metrics** are reported in CI

## What Is Well-Tested

| Component | Coverage | Notes |
|-----------|----------|-------|
| Layer/keyframe system | ~80-90% | Thorough: add, remove, move, navigate, exposure, selection, reversal |
| BitmapImage | ~70-80% | Good edge-case coverage, auto-crop, large image performance |
| FileManager | Good | File I/O, XML validation, project serialization |
| ViewManager | Good | View transforms, translation, rotation, coordinate mapping |
| ColorManager | Good | Color setting, save/load, indexed colors |
| LayerManager | Basic | Layer management operations |
| BitmapBucket | Moderate | Fill behavior, color tolerance, fill modes |
| Tool settings / PropertyInfo | Moderate | Property insertion, defaults, persistence |
| Layer subtypes (Bitmap, Vector, Camera, Sound) | Moderate | Type-specific operations and XML loading |

## Critical Gaps

### 1. UndoRedoManager — Basic Tests Added

- [x] Initialization (new system disabled by default, enabled via preference)
- [x] `load()` clears the stack / returns OK
- [x] `save()` marks stack as clean
- [x] `clearStack()` resets unsaved-changes state
- [x] `state()` returns nullptr when system disabled or INVALID type; valid state on bitmap layer
- [x] `record()` marks stack dirty, nullifies state pointer, no-op on nullptr
- [x] Undo restores bitmap pixel to pre-modification value
- [x] Redo re-applies modification after undo
- [x] Multiple undos restore intermediate states in order
- [x] Undo/redo cycle preserves full history
- [x] Post-save modifications tracked; undo back to save point shows clean
- [ ] Vector layer undo/redo (VectorReplaceCommand)
- [ ] Selection transform undo/redo (TransformCommand)
- [ ] Undo stack limit enforcement (UNDO_REDO_MAX_STEPS)

### 2. SelectionManager — No Tests

- [ ] Rotation, scaling, flipping
- [ ] Coordinate-space conversion and anchor points

### 3. VectorImage — Near-Empty (615 bytes, ~1 test case)

- [ ] Bezier curve operations
- [ ] Area detection
- [ ] Color management

### 4. All Drawing Tools (15+ tools) — No Tests

- [ ] Stroke interpolation
- [ ] Pressure curves
- [ ] Blend modes

### 5. Editor — No Tests

- [x] Initialization and manager wiring (`tests/src/test_editor.cpp`)

### 6. PlaybackManager / SoundManager — No Tests

- [ ] Frame advance logic and FPS timing (PlaybackManager)
- [ ] Sound clip association (SoundManager)

### 7. Rendering Pipeline — No Tests

- [ ] CanvasPainter, OnionSkinPainter, CameraPainter, SelectionPainter

### 8. MovieExporter / MovieImporter — No Tests

- [ ] Export parameter validation, edge cases (empty project, single frame, large canvas)
- [ ] MovieImporter

### Other Untested Managers

- [x] PreferenceManager — defaults and save/load round-trip (`tests/src/test_preferencemanager.cpp`)
- [ ] ClipboardManager — copy/paste of keyframes and layers
- [ ] OverlayManager

## Recommended Priorities

1. **UndoRedoManager** — basic tests added; vector/transform undo still needed
2. **SelectionManager** — math-heavy, regression-prone
3. **VectorImage** — needs parity with BitmapImage tests
4. **Tool computational logic** — extract and test without UI
5. **MovieExporter** — common source of user bug reports
6. **Add coverage metrics to CI** — `gcov`/`lcov` to make coverage visible and trackable

## Quick Wins

- [x] **Editor initialization** — verify all managers are created and wired
- [x] **PreferenceManager** — test defaults and save/load round-trip
- [ ] **ClipboardManager** — test copy/paste of keyframes and layers
