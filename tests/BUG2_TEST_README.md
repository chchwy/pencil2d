# Bug #2 Unit Tests - Frame Movement Data Loss Fix

## Overview

This document describes the unit tests added to verify the fix for Bug #2: "No Path Check in LayerBitmap::needSaveFrame()".

**Bug #2** was a critical data loss issue where bitmap frames that were moved/reordered in the timeline but not modified would not be saved, causing permanent frame loss after save/reload.

## Test Files

- **Location**: `tests/src/test_layerbitmap.cpp`
- **Test Framework**: Catch2
- **Lines Added**: 120-351

## Test Cases

### 1. `LayerBitmap::needSaveFrame - Bug #2 Regression Tests`

This test suite validates the `needSaveFrame()` logic under various conditions:

#### Test Sections:

1. **Unmodified frame with existing file should not need save**
   - Verifies optimization: frames that haven't changed and already have files don't get re-saved
   - Expected: `needSaveFrame()` returns `false`

2. **Modified frame should always need save**
   - Ensures modified frames are always saved regardless of file existence
   - Expected: `needSaveFrame()` returns `true`

3. **Frame without existing file should need save**
   - New frames that haven't been saved yet should trigger save
   - Expected: `needSaveFrame()` returns `true`

4. **Frame with empty fileName should need save**
   - Frames with no associated file should be saved
   - Expected: `needSaveFrame()` returns `true`

5. **BUG #2 FIX: Moved frame should need save even if unmodified** ⭐ **CRITICAL TEST**
   - Tests the core bug fix: frame moved from position 5 to position 10
   - Frame is NOT modified but path has changed
   - Before fix: would return `false` → data loss
   - After fix: returns `true` → frame is saved
   - Expected: `needSaveFrame()` returns `true`

6. **BUG #2 FIX: Frame at same position with same path should not need save**
   - Ensures fix doesn't break optimization for unmoved frames
   - Frame stays at same position with same file
   - Expected: `needSaveFrame()` returns `false`

7. **BUG #2 FIX: Multiple moves scenario**
   - Tests complex reordering: Frame A (1→2), Frame B (2→3)
   - Both frames should need save despite not being modified
   - Prevents the exact scenario reported by users
   - Expected: Both `needSaveFrame()` calls return `true`

### 2. `LayerBitmap::saveKeyFrameFile - Integration Test for Bug #2`

This test suite validates the full save workflow:

#### Test Sections:

1. **Save moved unmodified frame**
   - Creates frame at position 5, saves it
   - Moves frame to position 10 without modifying
   - Calls `saveKeyFrameFile()` again
   - Verifies:
     - New file exists at `001.010.png`
     - Frame's filename is updated correctly
     - Frame is marked as unmodified after save
   - This is the end-to-end test proving Bug #2 is fixed

2. **Unmodified frame at same location should return SAFE**
   - Saves a frame, then saves again without changes
   - Verifies save is skipped (returns `Status::SAFE`)
   - Ensures optimization still works

## How to Build and Run Tests

### Prerequisites

- Qt development environment
- qmake or QtCreator
- Pencil2D dependencies installed

### Building Tests (Command Line)

#### Linux/macOS:
```bash
cd tests
qmake tests.pro
make
```

#### Windows (MSVC):

**Option 1: Using the automated script (recommended)**
```powershell
cd tests
.\run_bug2_tests.ps1

# Or with custom Qt path:
.\run_bug2_tests.ps1 -QtPath "C:\Qt\6.5.3\msvc2019_64\bin"

# Or build release configuration:
.\run_bug2_tests.ps1 -Configuration release
```

**Option 2: Manual build**
```powershell
# Set Qt path (adjust version/path as needed)
$env:PATH += ";C:\Qt\6.5.3\msvc2019_64\bin"

# Generate Visual Studio project from root
qmake pencil2d.pro -tp vc -r

# Build tests target
msbuild ./pencil2d.sln /t:tests
```

#### Windows (MinGW):
```bash
cd tests
qmake tests.pro
mingw32-make
```

### Building Tests (Qt Creator)

1. Open `pencil2d.pro` in Qt Creator
2. In Projects panel, check "tests" subproject
3. Build the tests target

### Running Tests

The test executable will be in the build output directory:
- **Windows (MSVC)**: `debug\tests.exe` or `release\tests.exe`
- **Windows (MinGW)**: `debug\tests.exe` or `release\tests.exe`
- **Linux/macOS**: `tests`

#### Run All Tests:
```bash
# Linux/macOS
./tests

# Windows
.\debug\tests.exe
# or
.\release\tests.exe
```

#### Run Only Bug #2 Tests:
```bash
# Linux/macOS
./tests "LayerBitmap::needSaveFrame - Bug #2 Regression Tests"
./tests "LayerBitmap::saveKeyFrameFile - Integration Test for Bug #2"

# Windows
.\debug\tests.exe "LayerBitmap::needSaveFrame - Bug #2 Regression Tests"
.\debug\tests.exe "LayerBitmap::saveKeyFrameFile - Integration Test for Bug #2"
```

#### Run Specific Test Case:
```bash
# Linux/macOS
./tests "BUG #2 FIX: Moved frame should need save even if unmodified"

# Windows
.\debug\tests.exe "BUG #2 FIX: Moved frame should need save even if unmodified"
```

#### Verbose Output:
```bash
# Linux/macOS
./tests -s  # Show successful assertions too

# Windows
.\debug\tests.exe -s
```

### Expected Output

All tests should pass:

```
All tests passed (XX assertions in 9 test cases)
```

If any test fails, it indicates:
- The bug fix was not applied correctly
- A regression was introduced
- The build environment has issues

## What the Tests Prove

✅ **Before Bug #2 Fix:**
- Test "Moved frame should need save even if unmodified" would FAIL
- Moving frames without drawing would cause data loss

✅ **After Bug #2 Fix:**
- All tests PASS
- Moved frames are correctly detected and saved
- Bitmap layers now match vector layer behavior
- User data is protected from loss during frame reordering

## Test Coverage

The tests cover:
- ✅ Modified flag checking
- ✅ File existence checking
- ✅ Empty filename handling
- ✅ Path comparison (the bug fix)
- ✅ Single frame moves
- ✅ Multiple frame moves
- ✅ Integration with save workflow
- ✅ Optimization preservation (unmoved frames not re-saved)

## Continuous Integration

These tests should be run:
- ✅ Before committing changes to LayerBitmap
- ✅ In CI/CD pipeline for every pull request
- ✅ Before releasing new versions
- ✅ After modifying save/load logic

## Related Files

- **Bug Fix**: `core_lib/src/structure/layerbitmap.cpp:177-178`
- **Test File**: `tests/src/test_layerbitmap.cpp:120-351`
- **Bug Report**: `FRAME_DATA_LOSS_BUGS.md` (Bug #2 section)

## Regression Prevention

If these tests ever fail in the future:
1. **DO NOT** merge the code that broke them
2. Investigate what change caused the regression
3. The failure means users WILL experience data loss
4. Fix the regression before proceeding

## Future Test Improvements

Consider adding:
- [ ] Tests for presave() frame renaming logic (Bug #3)
- [ ] Tests for error handling during file operations
- [ ] Tests for concurrent frame moves
- [ ] Load/save round-trip tests with moved frames
- [ ] Performance tests for large projects with many frames

---

*Tests written: 2025-12-10*
*Bug #2 fixed in commit: [pending]*
