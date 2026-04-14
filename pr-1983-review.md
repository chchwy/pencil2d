# PR #1983 Review: Fix eraser bug for vector layers

- **Title:** Fix eraser bug for vector layers
- **Author:** RayenChalouati
- **Status:** Open, APPROVED (by MrStevns)
- **Scope:** `core_lib/src/tool/erasertool.cpp`, `core_lib/src/tool/erasertool.h` (+43/-6)

---

## Summary

This PR fixes incorrect vector paint erasure by replacing the old "check only the current point" approach with a segment-interpolation strategy. Instead of only selecting vertices near the cursor's current position, the new code tracks the full stroke path and samples points along each line segment between consecutive stroke positions, selecting all vertices within the eraser radius along that path. This prevents vertices from being missed when the cursor moves quickly between frames.

Additionally, the PR adds a null check for `currKey` in `updateStrokes()` (fixing a pre-existing potential null-pointer dereference) and calls `deselectAll()` at the start of each new stroke to avoid carrying stale selections into a new erase operation.

---

## Critical Issues

### 1. Member variable shadowing: `mStrokePoints` (BLOCKER)

The PR adds a new member `QList<QPointF> mStrokePoints` to `EraserTool` in `erasertool.h`. However, the parent class `StrokeTool` (in `stroketool.h`, line 105) **already declares a protected member with the exact same name and type**: `QList<QPointF> mStrokePoints`.

This means the PR introduces a **shadowed member variable**. The `EraserTool` will have two separate `mStrokePoints` lists:

- `StrokeTool::mStrokePoints` -- managed by `startStroke()`, `drawStroke()`, and `endStroke()` in the base class, used by other tools like `BrushTool`, `PenTool`, `PencilTool` for building BezierCurves.
- `EraserTool::mStrokePoints` -- the new one, managed manually in `pointerPressEvent`, `pointerMoveEvent`, and `pointerReleaseEvent`.

**Consequences:**

- The base class `StrokeTool::startStroke()` (called from `pointerPressEvent`) clears `StrokeTool::mStrokePoints` and adds a starting point to it. But the PR also clears and appends to `EraserTool::mStrokePoints` separately. This is redundant and confusing.
- `StrokeTool::endStroke()` clears `StrokeTool::mStrokePoints`, not the shadowed one. The PR separately clears `EraserTool::mStrokePoints` in `pointerReleaseEvent`. This works by accident but is fragile.
- `StrokeTool::drawStroke()` appends to `StrokeTool::mStrokePoints`, not the eraser's version. This means the two lists diverge in content and could cause subtle bugs if code is later added that references the base-class version from an `EraserTool` context.
- Any compiler with `-Wshadow` enabled will flag this as a warning.

**Fix:** Remove the `mStrokePoints` declaration from `erasertool.h` entirely and reuse the base class's `mStrokePoints`. Alternatively, if separate tracking is truly needed, give the eraser's list a distinct name (e.g., `mEraserStrokePoints`). However, reusing the base class member is strongly preferred since the base class already clears it in `startStroke()`/`endStroke()` and populates it in `drawStroke()`.

---

## Bugs

### 2. `deselectAll()` race with `drawStroke()` selection accumulation

The condition `if (mStrokePoints.size() <= 2)` is used to trigger `deselectAll()` at the "very start of a new stroke." However, because `mStrokePoints` is appended in `pointerMoveEvent` and the shadowed list starts at size 1 (from `pointerPressEvent`), the `deselectAll()` will fire on the first **two** calls to `updateStrokes()` (when size is 2, then still <= 2 at size 2). On the second call, this deselects everything that was selected during the first call's sampling loop, effectively throwing away work. This means the first segment's vertex selections could be lost.

**Fix:** Use a boolean flag (e.g., `mIsFirstStrokeUpdate`) that is set in `pointerPressEvent` and cleared after the first `deselectAll()` call, rather than relying on point count.

### 3. Single-click erase may fail to select any vertices

When the user clicks without moving (a single point erase), `pointerMoveEvent` never fires, so `mStrokePoints` (the shadowed version) only contains the starting point (size 1). In `updateStrokes()`, the `mStrokePoints.size() >= 2` check will be false, so the code enters neither branch and no vertices are selected. The `segmentLength > 0` else-branch handles "very short strokes" but requires size >= 2.

The old code handled this correctly because it simply checked `getCurrentPoint()` every call regardless of stroke history.

**Fix:** Add handling for the `mStrokePoints.size() == 1` case to select vertices near the single point.

---

## Design Concerns

### 4. Sampling density tied to radius may over-sample or under-sample

The sampling formula `int numSamples = qMax(2, static_cast<int>(segmentLength / (radius * 0.5)))` means:

- A very small eraser radius (e.g., 1px) with a long segment (e.g., 100px) yields 200 samples, each calling `getVerticesCloseTo()` which iterates all curves and vertices in the image. For complex vector images, this could cause noticeable lag per frame.
- A very large radius (e.g., 200px) with a moderate segment length (e.g., 50px) yields only 2 samples, which is fine but may still miss vertices near the edges of the swept area.

Consider capping `numSamples` at a reasonable maximum (e.g., 50) and/or documenting the performance trade-off.

### 5. Inconsistent VectorImage retrieval between `updateStrokes()` and `removeVectorPaint()`

- `removeVectorPaint()` uses: `static_cast<LayerVector*>(layer)->getLastVectorImageAtFrame(...)` -- type-safe via `LayerVector`.
- `updateStrokes()` uses: `static_cast<VectorImage*>(layer->getLastKeyFrameAtPosition(...))` -- casts a `KeyFrame*` directly to `VectorImage*`.

Both should use the same pattern. The `LayerVector::getLastVectorImageAtFrame()` approach is safer and more readable. This pre-dates the PR but would be a good cleanup.

### 6. `getVerticesCloseTo` called inside a tight loop without result deduplication

Each sample point along the segment calls `getVerticesCloseTo()`, and the same `VertexRef` may be returned by multiple sample points. While `setSelected(vertex, true)` is idempotent on the selection state, each call to `setSelected` triggers `modification()` internally, which may propagate unnecessary change notifications. For performance, consider collecting all unique vertices first, then selecting them once.

---

## Minor / Style Issues

### 7. Unnecessary `#include <QLineF>` (nitpick)

`QLineF` was already usable in this file before the PR (line 138 of the original code uses it). The explicit include is not wrong -- in fact, it is good practice to include what you use -- but the commit message/description doesn't mention this, and it was not needed to fix the compilation. This is fine to keep.

### 8. Comment accuracy

The comment `// Clear selections at the very start of a new stroke` is misleading because it fires on the first two updates (size <= 2), not just at the start.

### 9. Range-based for loop uses copy instead of const reference

```cpp
for (auto vertex : nearbyVertices)
```

Should be:

```cpp
for (const auto& vertex : nearbyVertices)
```

`VertexRef` is small (two ints), so this is not a performance concern, but the existing codebase appears inconsistent on this. The old code used `auto nearbyVertice` (also by copy) with a grammatically incorrect variable name. The PR fixes the naming (`vertex` instead of `nearbyVertice`) which is an improvement, but the by-value iteration pattern is copied twice in the new code.

---

## Verdict

**Not ready to merge.** The following must be addressed before merging:

1. **[BLOCKER]** Remove the shadowed `mStrokePoints` declaration from `erasertool.h`. Either reuse the base class's `mStrokePoints` (preferred) or rename the member to avoid shadowing.
2. **[Bug]** Fix the single-click erase case (size == 1) so that a click-without-drag still erases nearby vertices.
3. **[Bug]** Fix the `deselectAll()` logic so it fires exactly once at stroke start, not on the first two update calls.

Nice-to-have improvements (not blocking):
- Cap sampling density to avoid performance issues with small radii.
- Use consistent VectorImage retrieval pattern across both methods.
- Use `const auto&` in range-based for loops.
