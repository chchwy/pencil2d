# Quick Start - Bug #2 Tests on Windows

## TL;DR - Run Tests Right Now

```powershell
cd tests
.\run_bug2_tests.ps1
```

That's it! The script will:
1. ✅ Configure Qt environment
2. ✅ Generate Visual Studio solution
3. ✅ Build the tests
4. ✅ Run Bug #2 regression tests
5. ✅ Show you clear PASS/FAIL results

---

## Expected Output

If the bug fix is working correctly, you'll see:

```
=== Pencil2D Bug #2 Test Runner ===

Setting Qt path: C:\Qt\6.5.3\msvc2019_64\bin
Project root: d:\Github\pencil2d\pencil2d-wrok1

Generating Visual Studio solution...
Done.

Building tests target...
Build complete.

=== Running Bug #2 Regression Tests ===

Test 1: LayerBitmap::needSaveFrame - Bug #2 Regression Tests
All tests passed (X assertions in Y test cases)

Test 2: LayerBitmap::saveKeyFrameFile - Integration Test for Bug #2
All tests passed (X assertions in Y test cases)

=== Test Results ===
✓ All Bug #2 tests PASSED!

Bug #2 fix is working correctly:
  - Moved frames are properly detected
  - Frame data will not be lost during save/reload
  - Bitmap layers now match vector layer behavior
```

---

## If You Have Different Qt Version/Path

```powershell
.\run_bug2_tests.ps1 -QtPath "C:\Qt\6.7.0\msvc2019_64\bin"
```

---

## Build Release Instead of Debug

```powershell
.\run_bug2_tests.ps1 -Configuration release
```

---

## What the Tests Verify

1. ✅ **Moved frames are saved** - The core bug fix
2. ✅ **Unmoved frames aren't re-saved** - Optimization preserved
3. ✅ **Multiple frame moves work** - Complex reordering scenarios
4. ✅ **Modified frames always save** - Basic functionality
5. ✅ **New frames are saved** - First-time save works
6. ✅ **Empty filenames trigger save** - Edge cases covered
7. ✅ **Integration test passes** - Full save workflow works end-to-end

---

## If Tests Fail

**DO NOT PROCEED** - failing tests mean:
- The bug fix wasn't applied correctly
- A regression was introduced
- Users WILL experience data loss

Steps to take:
1. Check that `core_lib/src/structure/layerbitmap.cpp:177-178` has the fix
2. Verify the build is using the latest code
3. Check the detailed test output for what failed
4. Review `FRAME_DATA_LOSS_BUGS.md` Bug #2 section

---

## Manual Testing After Tests Pass

Even with passing unit tests, manually verify:

1. Open Pencil2D
2. Create new project with bitmap layer
3. Add keyframes at positions 1, 2, 3
4. Draw something on each frame
5. **Drag frame 2 to position 5** in timeline
6. Save project as `test.pclx`
7. Close Pencil2D
8. Reopen `test.pclx`
9. ✅ **Verify frame at position 5 has your drawing**

Before fix: Frame 5 would be blank/missing
After fix: Frame 5 has the correct drawing

---

## Full Documentation

- **Test Details**: `BUG2_TEST_README.md`
- **Bug Analysis**: `../FRAME_DATA_LOSS_BUGS.md`
- **Fix Summary**: `../BUG2_FIX_SUMMARY.md`

---

*For questions or issues, check the documentation or file an issue on GitHub.*
