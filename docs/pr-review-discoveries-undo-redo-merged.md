# PR Review Discoveries: undo-redo-merged

Date: 2026-04-19
Branch reviewed: `undo-redo-merged`
Compared against: `master`

## Scope Reviewed

Primary focus was undo/redo behavior and regression risk in these areas:

- `core_lib/src/interface/undoredocommand.cpp`
- `core_lib/src/interface/editor.cpp`
- `app/src/actioncommands.cpp`
- `core_lib/src/managers/undoredomanager.cpp`
- `core_lib/src/managers/layermanager.cpp`
- `tests/src/test_undoredocommand.cpp`

Diff summary observed:

- 26 files changed
- 2602 insertions, 40 deletions

## Confirmed Discovery (Blocking)

### 1) New undo/redo test crashes with SIGSEGV

- Command run:
  - `./tests/debug/tests.exe [undo-redo-new]`
- Result:
  - 1 failing test, fatal segmentation violation
- Failing test:
  - `DeleteLayerCommand restores deleted layer and id on undo`
- Crash location reported by test runner:
  - `tests/src/test_undoredocommand.cpp:161`

Additional focused repro:

- Command run:
  - `./tests/debug/tests.exe "DeleteLayerCommand restores deleted layer and id on undo" -s`
- Result:
  - same SIGSEGV, reported around `tests/src/test_undoredocommand.cpp:161`

Why this matters:

- A crashing test in newly added undo/redo coverage is a release blocker for this branch.

## High-Risk Logic Discovery

### 2) Paste collision undo/redo appears under-specified for cascaded collisions

Observed implementation:

- Collision positions are recorded during paste as positions only:
  - `core_lib/src/interface/editor.cpp` (`collisionPositions.append(newPosition)`)
- Undo removes added keys, then reverses shifts using `position + 1` in reverse order:
  - `core_lib/src/interface/undoredocommand.cpp` in `PasteFramesCommand::undo()`

Risk:

- For multi-step cascaded shifts, position-only bookkeeping may not be enough to fully reconstruct original pre-paste layout deterministically.
- Command does not store a full displacement map (which frames moved from where to where).

Status:

- Marked as high-risk behavior based on code-path analysis.
- Not yet confirmed as a failing runtime test in the current suite.

## Test Coverage Gaps Discovered

### 3) Paste test checks frame existence, not frame identity/content

In `tests/src/test_undoredocommand.cpp`:

- `PasteFramesCommand round-trip preserves displaced contiguous frames` verifies `keyExists()` at positions.
- It does not verify that the original frame objects/content are preserved after undo/redo.

Risk:

- Position-only assertions can miss content/identity corruption.

### 4) Missing stress cases for collision chains

No explicit test discovered for:

- Longer cascaded collision chains (3+ chained displacements)
- Non-contiguous paste sets with mixed collision patterns

## Suggested Follow-up Actions

1. Fix the SIGSEGV in delete-layer undo flow first (blocking).
2. Add a deterministic collision-state model for paste undo/redo (store full frame displacement mapping).
3. Strengthen paste tests to assert frame identity/content, not only position existence.
4. Add targeted regression tests for long collision chains and mixed collision layouts.

## Notes

- `tests/release/tests.exe` could not be used effectively in this environment (exited with code 1 and no output), so verification was done with `tests/debug/tests.exe`.
- Findings are based on local branch state at review time.
