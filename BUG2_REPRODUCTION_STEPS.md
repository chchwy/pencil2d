# Bug #2 - Frame Data Loss Reproduction Steps

## How to Reproduce Bug #2 in Pencil2D (Real Application)

This document provides step-by-step instructions to reproduce the frame data loss bug that occurs when moving frames in the timeline.

---

## Prerequisites

- Pencil2D application (version WITHOUT the Bug #2 fix)
- A new or existing project

---

## Reproduction Steps

### **Method 1: Simple Frame Move (Easiest)**

1. **Launch Pencil2D**
   - Open the application

2. **Create a new project**
   - File → New
   - Or open an existing project

3. **Ensure you have a Bitmap Layer**
   - If not, create one: Layer → New Bitmap Layer

4. **Create keyframes and draw on them**
   - Click on frame 1 in the timeline
   - Draw something (e.g., a red circle)
   - Click on frame 5 in the timeline
   - Click "Add Frame" or press the "+" button
   - Draw something different (e.g., a blue square)
   - Click on frame 10 in the timeline
   - Click "Add Frame"
   - Draw something else (e.g., a green star)

5. **Move frame 5 to a different position WITHOUT drawing on it**
   - In the timeline, **click and drag** the keyframe at position 5
   - Drop it at position 15
   - **Important**: Do NOT draw or modify the frame after moving it
   - The frame should still show the blue square you drew earlier

6. **Save the project**
   - File → Save (or Ctrl+S)
   - Save as `test_bug2.pclx`
   - Wait for "Project saved successfully" message

7. **Close Pencil2D**
   - File → Exit

8. **Reopen the project**
   - Launch Pencil2D again
   - File → Open
   - Select `test_bug2.pclx`

9. **Check the moved frame**
   - Navigate to frame 15 in the timeline
   - **BUG MANIFESTATION**: The frame is blank or missing!
   - The blue square you drew is gone
   - Frame 5 is also empty (original location)

**Expected behavior (with fix):** Frame 15 should contain the blue square
**Actual behavior (without fix):** Frame 15 is blank, data is lost

---

### **Method 2: Multiple Frame Reordering**

This reproduces the exact scenario from the unit test.

1. **Create a new project** with a bitmap layer

2. **Create three keyframes with distinct drawings:**
   - Frame 1: Draw a red dot
   - Frame 2: Draw a blue line
   - Frame 3: Draw a green circle

3. **Reorder the frames without modifying them:**
   - Drag frame 1 to position 5
   - Drag frame 2 to position 6
   - Drag frame 3 to position 7

4. **Save the project** (Ctrl+S)

5. **Close and reopen Pencil2D**

6. **Check frames 5, 6, 7:**
   - **BUG**: Some or all frames will be blank
   - Original drawings are lost

---

### **Method 3: Frame Insertion Causing Shifts**

1. **Create keyframes at positions 1, 5, 10** with different drawings

2. **Insert a new frame at position 3**
   - This shifts frame 5 → 6, and frame 10 → 11

3. **Save without drawing on the shifted frames**

4. **Close and reopen**

5. **Check shifted frames:**
   - **BUG**: Shifted frames may be blank

---

## Why This Bug Occurs

### Technical Explanation

When you move a frame in the timeline:

1. **The frame's position changes** (e.g., from position 5 to position 15)
2. **The expected filename changes** (from `001.005.png` to `001.015.png`)
3. **The `modified` flag is NOT set** (because you didn't draw on the frame)
4. **During save:**
   - `needSaveFrame()` checks: `isModified()`? → NO
   - `needSaveFrame()` checks: File exists at new location? → NO
   - ❌ **WITHOUT FIX**: Returns `false` (doesn't check if path changed)
   - The frame is **not saved** to the new location
5. **The `presave()` function** may have already renamed/deleted the old file
6. **XML is written** referencing `001.015.png` (which doesn't exist)
7. **On reload:**
   - XML says "load `001.015.png`"
   - File doesn't exist
   - Frame is **silently skipped**
   - **User sees a blank frame**

### What the Fix Does

The fix adds one additional check in `needSaveFrame()`:

```cpp
if (savePath != key->fileName()) // keyframe moved to different location
    return true;
```

Now when a frame is moved:
- Even if `isModified()` is `false`
- The path comparison detects the move
- Returns `true` → frame IS saved to new location
- Data is preserved ✅

---

## How to Verify the Bug is Fixed

After applying the Bug #2 fix:

1. **Follow the same reproduction steps above**
2. **After reopening the project:**
   - ✅ Frame 15 should show the blue square
   - ✅ All moved frames retain their drawings
   - ✅ No data loss occurs

---

## Additional Scenarios That Trigger This Bug

1. **Undo/Redo operations** that move frames
2. **Copy/paste frames** to different positions
3. **Removing frames** that causes automatic shifting
4. **Importing frames** that shifts existing frames
5. **Timeline scrubbing** combined with frame operations

---

## User Impact

### Who is Affected?
- **Any user** who moves/reorders keyframes
- **Especially animators** who frequently reorganize their timeline
- **More likely with large projects** (many frames to manage)

### Data Loss Risk
- ⚠️ **CRITICAL**: Lost frames cannot be recovered
- ⚠️ Users may not notice until much later
- ⚠️ No warning or error message shown
- ⚠️ Backup files may also be affected if saved multiple times

---

## Workaround (Before Fix)

If you must use a version without the fix:

1. **After moving a frame:**
   - Make a tiny modification to the frame (draw a single pixel)
   - This sets the `modified` flag
   - The frame will now be saved

2. **Always test save/reload** before doing extensive work

3. **Keep frequent backups** with different filenames

4. **Avoid moving frames** when possible - recreate them instead

---

## Testing Checklist

Use this checklist to verify the fix works:

- [ ] Move single frame to new position → Save → Reload → Frame intact
- [ ] Move multiple frames → Save → Reload → All frames intact
- [ ] Drag frame left (lower position) → Save → Reload → Frame intact
- [ ] Drag frame right (higher position) → Save → Reload → Frame intact
- [ ] Move frame, then undo → Save → Reload → Frame in original position
- [ ] Swap two frames → Save → Reload → Both frames intact in new positions
- [ ] Move frame in bitmap layer → Works
- [ ] Move frame in vector layer → Works (already had fix)

---

## Related Issues

- Bug #1: Empty frames written to XML without files
- Bug #3: presave() has no error handling
- Bug #4: Silent frame skipping during load
- Bug #5: Save continues after frame failures

See `FRAME_DATA_LOSS_BUGS.md` for complete analysis.

---

*Last Updated: 2025-12-12*
*Bug Fixed: Yes (see layerbitmap.cpp:172-173)*
