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

### 1. UndoRedoManager — No Tests

The undo/redo command stack, state save/restore, and command history are completely untested. Bugs here silently corrupt user work. **Recommended tests:** undo stack for each operation type (draw stroke, add/remove keyframe, move layer, transform selection). These are pure state operations that are easy to test without UI.

### 2. SelectionManager — No Tests

Complex transformation math (rotation, scale, translate) and coordinate-space conversions are untested. **Recommended tests:** rotation, scaling, flipping, coordinate-space conversion, anchor points. Floating-point math regressions are hard to spot visually.

### 3. VectorImage — Near-Empty (615 bytes, ~1 test case)

The bitmap side has 13 KB of tests; the vector side has almost none. **Recommended tests:** Bezier curve operations, area detection, color management.

### 4. All Drawing Tools (15+ tools) — No Tests

Brush, Pencil, Eraser, Select, Transform — the primary user interaction — have zero test coverage. **Recommended tests:** stroke interpolation, pressure curves, blend modes. Extract and test computational parts without needing a full canvas.

### 5. Editor — No Tests

The main orchestrator class that creates and wires all 11 managers together is untested. **Recommended tests:** initialization, manager wiring, null-pointer safety.

### 6. PlaybackManager / SoundManager — No Tests

Animation playback and audio have no coverage. **Recommended tests:** frame advance logic, FPS timing, sound clip association.

### 7. Rendering Pipeline — No Tests

CanvasPainter, OnionSkinPainter, CameraPainter, SelectionPainter — all untested. **Recommended tests:** basic rendering output validation, onion skin frame selection.

### 8. MovieExporter / MovieImporter — No Tests

Export is a common source of user-reported bugs. **Recommended tests:** export parameter validation, edge cases (empty project, single frame, large canvas).

### Other Untested Managers

- PreferenceManager (defaults and save/load round-trip)
- ClipboardManager (copy/paste of keyframes and layers)
- OverlayManager

## Recommended Priorities

1. **UndoRedoManager** — highest impact, pure state logic, easy to test
2. **SelectionManager** — math-heavy, regression-prone
3. **VectorImage** — needs parity with BitmapImage tests
4. **Tool computational logic** — extract and test without UI
5. **MovieExporter** — common source of user bug reports
6. **Add coverage metrics to CI** — `gcov`/`lcov` to make coverage visible and trackable

## Quick Wins

- **Editor initialization** — verify all managers are created and wired (2-3 test cases)
- **PreferenceManager** — test defaults and save/load round-trip
- **ClipboardManager** — test copy/paste of keyframes and layers
