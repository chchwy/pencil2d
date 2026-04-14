# PR #1989 Review: Fixed Issue #1942 [BUG] Some modifying actions can still be performed

**Author:** Jouvens-Corantin  
**Status:** APPROVED (by MrStevns), Open, Mergeable  
**Scope:** +54 additions, -10 deletions, 5 files changed  
**SonarQube:** Passed (0 new issues)

---

## Summary

This PR prevents users from accidentally modifying hidden layers by adding visibility checks to several operations:

- **Clear Frame** (`ScribbleArea::clearImage`) -- blocks clearing on hidden layers
- **Select All** (`Editor::selectAll`) -- blocks selection on hidden layers
- **Paste** (`Editor::paste`) -- blocks pasting on hidden layers
- **Paste from Previous Frame** (`Editor::pasteFromPreviousFrame`) -- blocks pasting on hidden layers
- **Move Key Forward/Backward** (`ActionCommands::moveFrameForward/moveFrameBackward`) -- blocks frame movement on hidden layers
- **Move Key Frame (model)** (`Layer::moveKeyFrame`) -- redundant guard at the data layer
- **Toggle Visibility** (`Editor::switchVisibilityOfLayer`) -- deselects all when any layer's visibility is toggled

Each blocked action shows the existing `showLayerNotVisibleWarning()` message box. The PR also adds a blank line to `selecttool.cpp` (no functional change).

---

## Critical Issues

None.

---

## Bugs

### 1. `selectAll()` missing null check on `layer` (pre-existing, aggravated by PR)

In `Editor::selectAll()`, the PR adds `if (!layer->visible())` but `layer` is never null-checked. If `layers()->currentLayer()` returns `nullptr`, the new `layer->visible()` call will dereference a null pointer and crash.

The original code also had this issue (`layer->type()` was called without a null check), but the PR adds a new dereference before any type check, making it the first point of failure.

**Recommendation:** Add `if (layer == nullptr) { return; }` before the visibility check, consistent with the pattern used in `clearImage()` and the new `moveFrameForward/Backward` code.

```cpp
Layer* layer = layers()->currentLayer();
if (layer == nullptr) { return; }

if (!layer->visible()) {
    mScribbleArea->showLayerNotVisibleWarning();
    return;
}
```

---

## Design Concerns

### 1. `deselectAll()` fires on EVERY visibility toggle, not just hide

In `switchVisibilityOfLayer()`, `deselectAll()` is called unconditionally -- both when hiding AND when showing a layer. If a user has a selection on Layer A and toggles Layer B to *visible*, the selection on Layer A is cleared. This is unexpected and disruptive to the workflow.

**Recommendation:** Only deselect when the toggled layer is the *current* layer and is being *hidden*:

```cpp
void Editor::switchVisibilityOfLayer(int layerNumber)
{
    Layer* layer = mObject->getLayer(layerNumber);
    if (layer != nullptr) layer->switchVisibility();

    // Only deselect if the current layer was just hidden
    if (layer != nullptr && !layer->visible() && layerNumber == layers()->currentLayerIndex()) {
        deselectAll();
    }

    mScribbleArea->onLayerChanged();
    emit updateTimeLine();
}
```

### 2. Duplicate visibility check in `moveKeyFrame` (model layer)

The PR adds a visibility check inside `Layer::moveKeyFrame()` *and* inside `ActionCommands::moveFrameForward/Backward()`. This means the check is performed twice. Maintainer @chchwy explicitly commented that this logic should NOT be in the Layer (model) class, as it's a UI-level concern. The Layer should remain a pure data model that allows operations regardless of visibility state.

The check in `Layer::moveKeyFrame()` also silently returns `false` without any user feedback, unlike the `ActionCommands` versions that show a warning. This creates an inconsistency: if `moveKeyFrame` is called from a code path that doesn't check visibility first, the operation silently fails.

**Recommendation:** Remove the visibility check from `Layer::moveKeyFrame()` and keep it only in the UI/controller layer (`ActionCommands`), consistent with the pattern used for all other operations in this PR (where checks are in `Editor` or `ActionCommands`, not in the model).

### 3. `deselectAll()` placement relative to `onLayerChanged()`

In `switchVisibilityOfLayer()`, `deselectAll()` is called after `switchVisibility()` but before `mScribbleArea->onLayerChanged()`. If `onLayerChanged()` relies on selection state (e.g., for painting), the order is fine. But this should be verified. The call to `deselectAll()` should probably come before `onLayerChanged()` to ensure the repaint sees the cleared selection, which is the current order -- so this is OK.

---

## Minor / Style Issues

### 1. Blank line added to `selecttool.cpp` (no-op change)

The only change in `selecttool.cpp` is adding a blank line at line 218 inside `keepSelection()`. This adds noise to the diff and git blame without any functional benefit. It should be removed.

### 2. Comment style inconsistency

The PR adds comments in varying styles:
- `// Prevent moving keyframe if the layer is not visible, as it can cause confusion...` (verbose, explains "why")
- `// Prevents pasting on an invisible layer, as this is likely a mistake...` (verbose, slightly different phrasing)
- `// Prevents Selection of an invisible layer...` ("Selection" capitalized mid-sentence)
- `// Clearing an invisible layer is likely a mistake...` (different sentence structure)
- `// Deselect all to prevent confusion, as the user might have selected something...` (verbose, redundant "confusion")

The comments are longer than necessary for a simple guard clause. A shorter, consistent pattern would be better:

```cpp
// Don't allow modifications on hidden layers
if (!layer->visible()) {
    ...
}
```

### 3. Inconsistent brace style for null guard

The PR uses `if (layer == nullptr) { return; }` (one-line braces) in `moveFrameForward/Backward`, which is consistent with the existing codebase style (e.g., `clearImage` already uses this). This is fine.

### 4. `notifyAnimationLengthChanged()` and `framesModified()` skipped on early return

In the original `moveFrameForward()`, `notifyAnimationLengthChanged()` and `framesModified()` were always emitted even when `layer` was null. With the new early returns (for null and hidden layer), these signals are no longer emitted in those cases. This is actually *more correct* (no change occurred, so no need to signal), but it's a subtle behavior change worth noting.

---

## Verdict

**Not quite ready to merge.** The PR is a good improvement overall and addresses a real usability issue. However, three things should be addressed before merging:

1. **Must fix:** Add a null check for `layer` in `selectAll()` before the new `!layer->visible()` call. Without this, there is a potential null pointer dereference crash.

2. **Should fix:** Make `deselectAll()` in `switchVisibilityOfLayer()` conditional -- only deselect when the *current* layer is being *hidden*, not on every visibility toggle of any layer.

3. **Should fix:** Remove the visibility check from `Layer::moveKeyFrame()` per maintainer @chchwy's review comment. Keep the check only in the UI layer (`ActionCommands`).

4. **Nice to have:** Remove the spurious blank line in `selecttool.cpp`.

With items 1-3 resolved, this PR would be ready to merge.
