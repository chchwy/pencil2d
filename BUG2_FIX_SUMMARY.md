# Bug #2 Fix Summary - Frame Movement Data Loss

## Quick Overview

**Issue**: Random frame data loss after save/reload when frames were moved/reordered in timeline
**Root Cause**: `LayerBitmap::needSaveFrame()` didn't check if frame path changed
**Fix Status**: ✅ **FIXED** + Comprehensive unit tests added
**Severity**: CRITICAL - Caused permanent user data loss

---

## The Bug

Users reported that after moving frames in the timeline and saving their project, some frames would be **permanently lost** after reloading. This happened silently - no error messages, no warnings.

### Technical Cause

The `needSaveFrame()` function in bitmap layers was missing a critical check that was present in vector layers:

**Before Fix** (Bitmap - BROKEN):
```cpp
bool LayerBitmap::needSaveFrame(KeyFrame* key, const QString& savePath)
{
    if (key->isModified()) return true;
    if (QFile::exists(savePath) == false) return true;
    if (key->fileName().isEmpty()) return true;
    return false;  // ❌ MISSING: No check if path changed!
}
```

**Vector layers** (CORRECT):
```cpp
bool LayerVector::needSaveFrame(KeyFrame* key, const QString& strSavePath)
{
    if (key->isModified()) return true;
    if (QFile::exists(strSavePath) == false) return true;
    if (strSavePath != key->fileName()) return true;  // ✅ Checks if moved!
    return false;
}
```

### Data Loss Scenario

1. User creates frame at position 5 → file: `001.005.png`
2. User moves frame to position 10 (expected new file: `001.010.png`)
3. Frame is not modified (user didn't draw), so `isModified() = false`
4. **During save:**
   - `presave()` renames the file internally
   - But `needSaveFrame()` returns `false` (no path check!)
   - Frame save is **skipped**
   - XML references `001.010.png` but file may not exist there
5. **On reload:**
   - XML says "load 001.010.png"
   - File doesn't exist or is at wrong location
   - Frame is **silently skipped**
   - **Data permanently lost** 🔴

---

## The Fix

### Code Change

**File**: `core_lib/src/structure/layerbitmap.cpp:177-178`

Added the missing path comparison check:

```cpp
bool LayerBitmap::needSaveFrame(KeyFrame* key, const QString& savePath)
{
    if (key->isModified()) return true;
    if (QFile::exists(savePath) == false) return true;
    if (key->fileName().isEmpty()) return true;
    if (savePath != key->fileName()) return true;  // ✅ NEW: Detect moved frames!
    return false;
}
```

This ensures bitmap layers have the same robust move-detection as vector layers.

---

## Unit Tests Added

### Test File
`tests/src/test_layerbitmap.cpp` (lines 120-351)

### Test Coverage

**9 comprehensive test cases** covering:

1. ✅ Unmodified frame with existing file (optimization check)
2. ✅ Modified frame always needs save
3. ✅ New frame without file needs save
4. ✅ Frame with empty filename needs save
5. ✅ **CRITICAL: Moved frame needs save even if unmodified** (the bug fix)
6. ✅ Unmoved frame doesn't trigger unnecessary save
7. ✅ Multiple frame moves scenario (Frame A: 1→2, Frame B: 2→3)
8. ✅ Integration test: Full save workflow with moved frame
9. ✅ Optimization preserved: Repeated saves of unchanged frame return SAFE

### How to Run Tests

#### Windows (MSVC):
```powershell
# Build from root directory
$env:PATH += ";C:\Qt\6.5.3\msvc2019_64\bin"
qmake pencil2d.pro -tp vc -r
msbuild ./pencil2d.sln /t:tests

# Run tests
.\debug\tests.exe "Bug #2"
```

#### Linux/macOS:
```bash
cd tests
qmake tests.pro
make
./tests "Bug #2"
```

Expected output:
```
All tests passed
```

---

## Impact

### Before Fix ❌
- Moving frames → data loss
- Inconsistent behavior between bitmap and vector layers
- Silent failures
- User frustration and lost work

### After Fix ✅
- Frame moves properly saved
- Bitmap and vector layers consistent
- User data protected
- Regression tests prevent future breakage

---

## Files Changed

| File | Type | Description |
|------|------|-------------|
| `core_lib/src/structure/layerbitmap.cpp` | Fix | Added path comparison check (1 line) |
| `tests/src/test_layerbitmap.cpp` | Tests | Added 9 comprehensive test cases (231 lines) |
| `FRAME_DATA_LOSS_BUGS.md` | Docs | Full bug analysis and recommendations |
| `tests/BUG2_TEST_README.md` | Docs | Test documentation and usage guide |
| `BUG2_FIX_SUMMARY.md` | Docs | This summary document |

---

## Verification Checklist

Before considering this fix complete, verify:

- [x] Code fix applied to `layerbitmap.cpp:177-178`
- [x] Unit tests added to `test_layerbitmap.cpp`
- [x] Tests compile without errors
- [x] All tests pass
- [ ] Manual testing: Create project, move frames, save, reload
- [ ] Test on all platforms (Windows/Mac/Linux)
- [ ] Update CHANGELOG.md
- [ ] Create pull request with bug reference
- [ ] Add to release notes

---

## Related Issues

- **Bug #1**: Empty frames written to XML without files
- **Bug #3**: presave() has no error handling (CRITICAL - needs fix)
- **Bug #4**: Silent frame skipping during load
- **Bug #5**: Save continues after frame failures

See `FRAME_DATA_LOSS_BUGS.md` for complete analysis.

---

## Prevention

To prevent similar bugs in the future:

1. ✅ **Always run unit tests** before committing layer save/load changes
2. ✅ **Keep bitmap and vector layer logic synchronized**
3. ✅ **Add tests for any new save/load logic**
4. ✅ **Test edge cases**: moves, reordering, empty frames
5. ✅ **Manual testing**: Actually move frames and save/reload

---

## Credits

- **Bug Report**: User reports of random frame data loss
- **Root Cause Analysis**: Identified inconsistency between bitmap/vector layers
- **Fix Applied**: 2025-12-10
- **Tests Written**: 2025-12-10

---

## Quick Reference

**To test the fix manually:**

1. Create new Pencil2D project
2. Add bitmap layer
3. Create keyframes at positions 1, 2, 3
4. Draw something on each frame
5. **Move frame 2 to position 5** (drag in timeline)
6. Save project as test.pclx
7. Close Pencil2D
8. Reopen test.pclx
9. ✅ **Verify frame at position 5 still has your drawing**

Before fix: Frame 5 would be blank/missing
After fix: Frame 5 has the correct drawing

---

*Bug #2 Fixed: 2025-12-10*
