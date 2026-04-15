# Undo/Redo Coverage Audit

**Date:** 2026-04-16
**Branch:** feature/iss-748-undo-redo-critical-ops
**Purpose:** Identify all user-visible actions that mutate application state but are not yet covered by the undo/redo command system.

---

## Already Covered

The new command-based undo/redo system (`KeyFrameAddCommand`, `KeyFrameRemoveCommand`, `MoveKeyFramesCommand`, `BitmapReplaceCommand`, `VectorReplaceCommand`, `TransformCommand`, `RemoveKeyFramesCommand`, `DeleteLayerCommand`, `PasteFramesCommand`) covers the following:

**Drawing tools**
- [x] Brush, Pencil, Pen, Eraser, Bucket fill, Move, Select, Polyline — all commit via stroke/tool system

**Individual keyframe**
- [x] Add frame
- [x] Remove frame
- [x] Move keyframes (timeline drag)

**Canvas operations**
- [x] Clear frame
- [x] Delete selection
- [x] Paste from previous frame
- [x] Flip selection
- [x] Reposition frame

**Bulk frame / layer operations**
- [x] Remove selected frames
- [x] Cut frames (frame-level)
- [x] Delete layer
- [x] Paste frames
- [x] Apply transformed selection (Move tool commit path)
- [x] Add Transparency to Paper / Trace Scanned Drawings (batched via macro)

---

## Missing Undo/Redo

### Priority 1 — Critical (Destructive, no recovery path)

- [x] **Remove Selected Frames** — `app/src/actioncommands.cpp` L742–763
  Implemented via `RemoveKeyFramesCommand`. "Irreversible" warning removed from dialog.

- [x] **Delete Layer** — `core_lib/src/managers/layermanager.cpp` L321–349
  Implemented via `DeleteLayerCommand` using `Object::takeLayer()` / `Object::insertLayerAt()`. "Cannot be undone" warning removed from dialog.

- [x] **Cut Frames** — `core_lib/src/interface/editor.cpp` L223–224
  Implemented via `RemoveKeyFramesCommand` (reused), pushed through `UndoRedoManager::cutKeyFrames()`.

- [x] **Paste Frames** — `core_lib/src/interface/editor.cpp` L338, L374
  Implemented via `PasteFramesCommand`. Records newly added frame positions and original positions of any displaced frames; displacement map is pre-computed before mutation so clones are captured while frames still exist. (commit 0e75de9)

- [x] **Apply Transformed Selection** — `core_lib/src/interface/scribblearea.cpp` L1333–1375
  Implemented via `createState(KEYFRAME_MODIFY)` + `record()` wrapping the bitmap clear+paste / vector `applySelectionTransformation()` in `ScribbleArea::applyTransformedSelection()`. Redundant dangling `createState` removed from `MoveTool::pointerPressEvent`. (commit 0e75de9)

- [x] **Add Transparency to Paper (Trace Scanned Drawings)** — `app/src/addtransparencytopaperdialog.cpp` L194–265
  Implemented via `beginMacro()`/`endMacro()` on `UndoRedoManager` wrapping per-frame `createState` + `record` pairs. Single-frame path uses one command; all-frames path is batched into a single visible undo step. `endMacro()` is always called even on early cancel. (commit 0e75de9)

---

### Priority 2 — High (Animation-affecting)

- [ ] **Add/Subtract Frame Exposure** — `app/src/actioncommands.cpp` L697–731
  Calls `Layer::setExposureForSelectedFrames(offset)`, emits `framesModified` but no undo command.

- [ ] **Insert Exposure at Position** — `app/src/actioncommands.cpp` L738
  Calls `Layer::insertExposureAt(currentPosition)` with no undo.

- [ ] **Move Frame Forward/Backward** — `app/src/actioncommands.cpp` L844–869
  Calls `Layer::moveKeyFrame(pos, offset)`, only emits `framesModified()`.

- [ ] **Reverse Frame Order** — `app/src/actioncommands.cpp` L765–777
  Calls `Layer::reverseOrderOfSelection()` with no undo command.

- [ ] **Create Layer (Bitmap/Vector/Sound/Camera)** — `app/src/actioncommands.cpp` L871–922
  Each `addNewXxxLayer()` path calls `Object::addNewXxxLayer()` with no undo.

- [ ] **Duplicate Layer** — `app/src/actioncommands.cpp` L784–803
  Creates new layer and copies all keyframes in a loop, no undo.

- [ ] **Duplicate Key (Frame)** — `app/src/actioncommands.cpp` L805–842
  Clones the current keyframe and inserts at next empty position via `layer->addKeyFrame()`, no undo. Sound clips are also duplicated.

- [ ] **Swap Layers** — `core_lib/src/interface/editor.cpp` L1006–1021
  Calls `Object::swapLayers()` with no undo command.

- [ ] **Camera Tool — Transform View** — `core_lib/src/tool/cameratool.cpp` L448–500
  Modifies camera translation, scaling, and rotation via `curCam->translate()/scale()/rotate()` on pointer drag. No `backup()` call; every drag permanently mutates the Camera keyframe.

- [ ] **Camera Tool — Transform Path** — `core_lib/src/tool/cameratool.cpp` L312–318
  Calls `layer->updatePathControlPointAtFrame(pos)` to move camera path waypoints. No undo.

- [ ] **Layer Opacity — Fade In/Out** — `app/src/layeropacitydialog.cpp` L139–205
  Iterates selected keyframes calling `setOpacityForKeyFrame()` in a loop. No `backup()`.

- [ ] **Layer Opacity — Set for Current/Selected Keyframes** — `app/src/layeropacitydialog.cpp` L296–325
  Sets opacity on individual or selected keyframes via slider/spinbox. No undo; changes should be batched into a single undo step per interaction.

- [ ] **Peg Bar Alignment** — `core_lib/src/structure/pegbaraligner.cpp` L40–75
  Calls `img->moveTopLeft()` on every keyframe across selected layers. No `backup()`, permanently moves bitmap positions.

- [ ] **Import Layers (from another .pclx)** — `app/src/importlayersdialog.cpp` L81–130
  Adds entire layers (with all keyframes and colors) from another project via `object->addLayer()`. No undo.

---

### Priority 3 — Medium (Content-affecting)

- [ ] **Change Line Color (current frame)** — `app/src/actioncommands.cpp` L955–965
  Calls `BitmapImage::fillNonAlphaPixels(color)` directly, no undo.

- [ ] **Change Line Color (entire layer)** — `app/src/actioncommands.cpp` L967–980
  Iterates all keyframes calling `fillNonAlphaPixels(color)`, no undo.

- [ ] **Rename Layer** — `core_lib/src/managers/layermanager.cpp` L351–357
  Calls `Layer::setName()` with no undo command.

- [ ] **Color Palette — Add Color** — `app/src/colorpalettewidget.cpp` L129, L561–588
  Calls `mObject->addColorAtIndex()` or `mObject->addColor()` with no undo; both the right-click menu "Add" and the Add button paths.

- [ ] **Color Palette — Replace/Edit Color** — `app/src/colorpalettewidget.cpp` L149–163, L666–700
  Calls `editor()->object()->setColor(index, newColor)` via `updateItemColor()`, no undo.

- [ ] **Color Palette — Remove Color** — `app/src/colorpalettewidget.cpp` L165, L591–640
  Calls `mObject->removeColor(index)`, no undo. Especially destructive for vector layers that reference that color index.

---

### Priority 4 — Lower (Non-destructive / view state)

- [ ] **Toggle Layer Visibility** — `core_lib/src/interface/editor.cpp` L997–1004
  Calls `Layer::switchVisibility()` with no undo.

---

## Implementation Notes

- The new command system lives in `core_lib/src/undocomponents/`. New commands should subclass the appropriate base and be pushed onto the stack via `UndoRedoManager`.
- Layer management commands will be the most complex to implement correctly — they need to capture full layer state (all keyframes + metadata) for the undo payload.
- Exposure commands should be straightforward: record the before/after frame position map, then apply/reverse.
- ~~The `// TODO: undo/redo implementation` comments in `editor.cpp` at lines 338 and 374 mark the paste paths and are a good starting point.~~ (resolved in 0e75de9)
- ~~**Apply Transformed Selection** is Priority 1 — it's the Move tool's commit path and every transform (move/scale/rotate on canvas) is permanently baked in on mouse release with no `backup()`.~~ (resolved in 0e75de9)
- **Camera tool transforms** need careful handling: batch pointer-drag events into a single undo step (record state on `pointerPressEvent`, commit on `pointerReleaseEvent`).
- **Layer opacity** slider/spinbox changes should be batched into a single undo step per interaction, not per value change.
- **Color palette** operations are especially impactful for vector layers where color indices are referenced by geometry — removing or reordering colors can silently corrupt vector frames.
- **Peg bar alignment** and **Add Transparency to Paper** are batch operations that modify many frames at once. The undo payload will need to capture all affected frames' before-state.
- **Import Layers** may be acceptable without undo if the user can simply delete the imported layers, but for consistency it should still be in the system.
