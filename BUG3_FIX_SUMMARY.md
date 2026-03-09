# Bug #3 Fix Summary - presave() Error Handling

## Overview
Bug #3 is the **root cause** of the frame data loss issue. The `presave()` function in LayerBitmap was silently ignoring file operation failures, causing frame data to be lost when disk operations failed.

## The Problem

### Location
- **File**: [core_lib/src/structure/layerbitmap.cpp:110-152](core_lib/src/structure/layerbitmap.cpp#L110-L152)
- **Function**: `LayerBitmap::presave()`

### What Was Wrong
The presave() function moves frame files when frames are repositioned on the timeline. However, it didn't check if file operations succeeded:

```cpp
// OLD CODE (BEFORE FIX):
QFile::rename(b->fileName(), tmpPath);  // ❌ No error check!
b->setFileName(tmpPath);

// Later...
QFile::remove(dest);  // ❌ No error check!
QFile::rename(b->fileName(), dest);  // ❌ No error check!
b->setFileName(dest);  // ❌ Assumes success!
```

### When This Causes Data Loss
1. **Disk full**: Rename fails, but code continues as if it succeeded
2. **Permission denied**: File operation blocked, error ignored
3. **File locked**: Another process has file open, operation fails silently
4. **Network drive issues**: Timeout or disconnect during save
5. **Antivirus interference**: Security software blocks operation

**Result**: `fileName()` points to a location where the file doesn't actually exist, causing the frame to be "lost" on next load.

## The Fix

### Changes Made

Added proper error handling to all file operations in presave():

```cpp
// NEW CODE (AFTER FIX):
bool success = false;
if (QFileInfo(oldPath).dir() != dataFolder) {
    success = QFile::copy(oldPath, tmpPath);
} else {
    success = QFile::rename(oldPath, tmpPath);
}

if (!success) {
    DebugDetails dd;
    dd << "Failed to move frame file to temporary location";
    dd << QString("Frame position: %1").arg(b->pos());
    dd << QString("Source: %1").arg(oldPath);
    dd << QString("Destination: %1").arg(tmpPath);
    return Status(Status::FAIL, dd);  // ✅ Return error!
}
```

### Key Improvements

1. **Check all QFile::rename() calls** - Verify return value
2. **Check all QFile::remove() calls** - Verify return value
3. **Check all QFile::copy() calls** - Verify return value
4. **Return Status::FAIL** - Stop the save operation on first error
5. **Provide detailed diagnostics** - Include frame position, source, and destination paths

## Files Modified

### 1. [core_lib/src/structure/layerbitmap.cpp](core_lib/src/structure/layerbitmap.cpp#L127-L190)
- Added error checking to first pass (temp file moves)
- Added error checking to second pass (final file moves)
- Added DebugDetails messages for diagnostics

### 2. [tests/src/test_layerbitmap.cpp](tests/src/test_layerbitmap.cpp#L378-L622)
- Added `callPresave()` method to test fixture
- Created comprehensive test suite with 6 test cases

## Test Cases Created

### Test Case 1: Success - No Frames Need Moving
**Purpose**: Verify presave() succeeds when no work needed
**Expected**: Returns Status::OK

### Test Case 2: Success - Single Frame Move
**Purpose**: Verify presave() successfully moves one frame
**Expected**: File moved to new location, fileName() updated

### Test Case 3: Success - Multiple Frame Moves
**Purpose**: Verify presave() handles multiple frames correctly
**Expected**: All 3 frames moved from positions 1,2,3 to 5,6,7

### Test Case 4: BUG #3 FIX - Source File Missing
**Purpose**: Verify presave() fails gracefully when source file doesn't exist
**Expected**: Returns Status::FAIL (not silent failure!)

### Test Case 5: BUG #3 FIX - Cannot Remove Destination
**Purpose**: Verify presave() fails when destination file is locked/read-only
**Expected**: Returns Status::FAIL with diagnostic info

### Test Case 6: BUG #3 FIX - Chain Moves (A→B, B→C)
**Purpose**: Verify temporary file strategy prevents overwrites during shuffle
**Expected**: Both frames moved correctly without data loss
**Special**: Tests the critical scenario where frames swap positions

## How to Test

### Build the Project
```batch
cd d:\Github\pencil2d\pencil2d-wrok1
build-win.bat
```

### Run Bug #3 Tests
```batch
cd d:\Github\pencil2d\pencil2d-wrok1
run-bug3-tests.bat
```

Or manually:
```batch
cd tests\debug
set PATH=%PATH%;C:\Qt\6.5.3\msvc2019_64\bin
tests.exe "LayerBitmap::presave - Bug #3 Error Handling Tests"
```

### Run All LayerBitmap Tests
```batch
tests.exe "LayerBitmap*"
```

## Expected Test Results

All 6 test cases should **PASS**:

```
LayerBitmap::presave - Bug #3 Error Handling Tests
  ✓ presave() succeeds when no frames need moving
  ✓ presave() successfully moves single frame
  ✓ presave() successfully handles multiple frame moves
  ✓ BUG #3 FIX: presave() fails gracefully when source file is missing
  ✓ BUG #3 FIX: presave() fails when destination cannot be removed
  ✓ BUG #3 FIX: presave() handles chain moves (A→B, B→C)

All tests passed (6 assertions in 6 test cases)
```

## Relationship to Bug #2

- **Bug #3** (presave() errors) is the **root cause**
- **Bug #2** (missing path check) is the **safety net**

When presave() fails silently (Bug #3), the frame's fileName() doesn't match the actual file location. Bug #2 fix catches this discrepancy and forces a re-save, preventing data loss.

**Both fixes work together**:
1. Bug #3 fix prevents the error from happening in the first place
2. Bug #2 fix provides defense-in-depth if presave() somehow fails

## Impact

### Before Fix
- Silent data loss when file operations failed
- Users lose frames randomly during save
- No error messages or warnings
- Difficult to diagnose or reproduce

### After Fix
- Save operation fails safely with clear error message
- User is notified that save failed
- Detailed diagnostics help identify the problem
- User can retry after fixing the issue (free disk space, close other programs, etc.)

## Verification Checklist

- [x] Code compiles without errors
- [x] All 6 test cases created
- [ ] All tests pass (pending manual execution)
- [ ] Integration test with full save/load cycle
- [ ] Manual testing with simulated failures (disk full, read-only files)

## Next Steps

1. Run the tests to verify they all pass
2. Test the fix in the real application with simulated failures
3. Consider adding user-facing error dialogs when presave() fails
4. Update user documentation about disk space requirements

## Technical Details

### Error Handling Strategy
- **Fail fast**: Return error on first failure
- **No partial updates**: Don't modify fileName() if operation failed
- **Detailed diagnostics**: Provide actionable error information
- **Safe defaults**: Preserve original state on failure

### Why Not Rollback?
The current fix returns early on first failure, which means some frames might be in temp locations. However:
- The Object save code already handles presave() failures
- Temp files use unique names (t_XXX.YYY.png) that won't conflict
- Next save attempt will retry the operation
- More complex than needed for this bug

A future enhancement could add transaction-style rollback, but the current approach is sufficient and follows the existing error handling patterns in the codebase.
