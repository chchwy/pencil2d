# PR #1976 Review: Fix undo/redo being ignored when scrubber is on an empty frame

**Author:** MrStevns (Oliver Stevns)
**Status:** Open, no reviews yet
**Labels:** Undo
**Scope:** +6 / -5 across 3 files (`undoredocommand.cpp`, `undoredomanager.cpp`, `layervector.h`)

---

## Summary

This PR fixes a bug where undo/redo does nothing when the user draws on an empty frame (scrubber positioned on a frame with no keyframe). The root cause is that the undo system was using exact-position frame lookups (`keyExists`, `getBitmapImageAtFrame`, `getVectorImageAtFrame`) when it should have been using "at or before" lookups (`getLastKeyFrameAtPosition`, `getLastBitmapImageAtFrame`, `getLastVectorImageAtFrame`).

The `mKeyFrames` map in `Layer` uses `std::greater<int>` ordering (descending), which means `getLastKeyFrameAtPosition(n)` via `lower_bound(n)` correctly finds the keyframe at position `n` or the nearest one before it. This is essential for the "Keep drawing on previous key" behavior, where the scrubber is on frame 2 but the drawing targets the keyframe at frame 1.

The three changes are:

1. **`undoredomanager.cpp` (savedKeyFrameState):** Replaced `keyExists(frameIndex)` with `getLastKeyFrameAtPosition(frameIndex)`, so the undo save state correctly captures the previous keyframe even when the scrubber is on an empty frame. Also eliminates a redundant call by storing the result in a local variable.

2. **`undoredocommand.cpp` (BitmapReplaceCommand / VectorReplaceCommand constructors):** Changed `getBitmapImageAtFrame` / `getVectorImageAtFrame` to `getLastBitmapImageAtFrame` / `getLastVectorImageAtFrame` when capturing the redo state, applying the same "at or before" lookup logic.

3. **`layervector.h`:** Added `= 0` default for the `increment` parameter of `getLastVectorImageAtFrame`, matching the existing default in `LayerBitmap::getLastBitmapImageAtFrame`. This is required for the call site in `undoredocommand.cpp` to compile without explicitly passing `0`.

---

## Critical Issues

None.

---

## Bugs

### Missing null check in undo command constructors (pre-existing, but now more visible)

In `BitmapReplaceCommand` (line 53-54) and `VectorReplaceCommand` (line 93-94), the result of `getLastBitmapImageAtFrame` / `getLastVectorImageAtFrame` is dereferenced without a null check:

```cpp
redoBitmap = *static_cast<LayerBitmap*>(layer)->
        getLastBitmapImageAtFrame(editor->currentFrame());
```

While this is unlikely to be `nullptr` in practice (the constructor is only called after a drawing operation, so a keyframe should exist), `getLastKeyFrameAtPosition` can return `nullptr` if there are no keyframes at all. The old code (`getBitmapImageAtFrame`) had the same problem, so this is pre-existing. However, since this PR touches these lines, it would be good practice to add a null guard or at least a `Q_ASSERT`.

Note: The `savedKeyFrameState()` change in `undoredomanager.cpp` does properly add a null check (`if (frame)`), which is correct.

---

## Design Concerns

### Behavioral change in `savedKeyFrameState` when `getKeyFrameWhichCovers` would have matched

The old code had a two-branch structure:
```cpp
if (layer->keyExists(frameIndex))
    // clone from getLastKeyFrameAtPosition
else if (layer->getKeyFrameWhichCovers(frameIndex) != nullptr)
    // clone from getKeyFrameWhichCovers
```

The new code collapses the first branch:
```cpp
KeyFrame* frame = layer->getLastKeyFrameAtPosition(frameIndex);
if (frame)
    // clone frame
else if (layer->getKeyFrameWhichCovers(frameIndex) != nullptr)
    // clone from getKeyFrameWhichCovers
```

Since `getLastKeyFrameAtPosition(n)` returns the keyframe at `n` or the nearest one before `n`, it will always return a non-null value when `getKeyFrameWhichCovers(n)` would have returned non-null (because `getKeyFrameWhichCovers` itself calls `getLastKeyFrameAtPosition` internally). This means the `else if` branch for `getKeyFrameWhichCovers` becomes effectively dead code. This is fine for correctness -- the first branch now subsumes both cases -- but the dead `else if` branch could be cleaned up in a follow-up for clarity.

This is a minor concern and not a blocker.

---

## Minor / Style Issues

1. **Dead `else if` branch:** As discussed above, the `getKeyFrameWhichCovers` branch in `savedKeyFrameState()` is now unreachable. Consider removing it or adding a comment explaining it is a fallback for layer types with different keyframe semantics (e.g., sound layers with length > 1).

2. **Consistency:** The existing callers throughout the codebase already use `getLastBitmapImageAtFrame` / `getLastVectorImageAtFrame` for canvas painting and tool operations (e.g., `canvaspainter.cpp:301`, `scribblearea.cpp:1048`). This PR makes the undo system consistent with that pattern, which is good.

---

## Verdict

**Ready to merge.** The fix is correct, minimal, and well-targeted. It properly addresses the root cause: exact-position lookups failing when the scrubber is on an empty frame while drawing targets a previous keyframe. The change is consistent with how the rest of the codebase handles frame lookups.

The only actionable suggestion is to add a null guard (or `Q_ASSERT`) on the `getLastBitmapImageAtFrame` / `getLastVectorImageAtFrame` return values in the undo command constructors, since a null pointer dereference there would be a crash. This is a pre-existing issue but worth fixing while the code is being touched.
