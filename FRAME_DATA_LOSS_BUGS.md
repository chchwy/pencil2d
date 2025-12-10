# Frame Data Loss Bugs - Critical Issues Report

## Overview

This document details critical bugs that cause random, silent frame data loss in Pencil2D. Users report that after saving and reloading projects, some frames are missing without any error messages or warnings.

---

## Bug #1 (REVISED): Empty Frames Written to XML Without Files

**Severity:** Medium
**Status:** Identified
**Location:** [layerbitmap.cpp:184-199](core_lib/src/structure/layerbitmap.cpp#L184-L199), [bitmapimage.cpp:822-846](core_lib/src/graphics/bitmap/bitmapimage.cpp#L822-L846)

### Problem

New/empty frames get entries in main.xml but no PNG files are created, leading to silent frame loss on reload.

### When It Happens

- User creates a keyframe but doesn't draw on it
- User draws, then erases everything (making bounds empty)
- Frame exists in the timeline but has no pixel data

### Technical Details

1. **Frame creation** creates a BitmapImage with null mImage:
   ```cpp
   KeyFrame* LayerBitmap::createKeyFrame(int position)
   {
       BitmapImage* b = new BitmapImage;  // Default constructor, mImage is null
       b->setPos(position);
       b->enableAutoCrop(true);
       return b;
   }
   ```

2. **During save**, `needSaveFrame()` returns `true` because no file exists yet:
   ```cpp
   if (QFile::exists(savePath) == false) return true;  // TRUE for new frames
   ```

3. **writeFile is called** but no file is created:
   ```cpp
   Status BitmapImage::writeFile(const QString& filename)
   {
       if (!mImage.isNull())  // FALSE (mImage is null)
       {
           // Skipped
       }

       if (bounds().isEmpty())  // TRUE (frame is empty)
       {
           QFile f(filename);
           if(f.exists()) { f.remove(); }
           setFileName("");  // Clears filename
       }
       return Status::SAFE;
   }
   ```

4. **XML is written with reference to non-existent file**:
   ```cpp
   QDomElement imageTag = doc.createElement("image");
   imageTag.setAttribute("src", fileName(pKeyFrame));  // e.g., "001.005.png"
   layerElem.appendChild(imageTag);  // Added even if file doesn't exist!
   ```

5. **On reload**, XML says "load 001.005.png" but file doesn't exist → frame is silently skipped.

### Recommended Fix

- Don't add `<image>` tags to XML if no PNG file exists
- Or create a minimal 1x1 transparent PNG for empty frames
- Or add validation during XML writing to check file existence

---

## Bug #2: No Path Check in LayerBitmap::needSaveFrame() ⚠️ CRITICAL

**Severity:** CRITICAL
**Status:** ✅ PATCHED
**Location:** [layerbitmap.cpp:169-180](core_lib/src/structure/layerbitmap.cpp#L169-L180)

### Problem

Bitmap layers don't check if a frame was moved to a different location, unlike vector layers. This causes moved but unmodified frames to not be saved, resulting in permanent data loss.

### When It Happens

- User moves/reorders frames in the timeline without drawing on them
- Frame position changes but modified flag is not set
- presave() renames the old file, but new location is never saved

### Technical Details

**Before the fix** (Bitmap layers):
```cpp
bool LayerBitmap::needSaveFrame(KeyFrame* key, const QString& savePath)
{
    if (key->isModified()) return true;
    if (QFile::exists(savePath) == false) return true;
    if (key->fileName().isEmpty()) return true;
    return false;  // ❌ Doesn't check if frame moved!
}
```

**Vector layers (correct behavior)**:
```cpp
bool LayerVector::needSaveFrame(KeyFrame* key, const QString& strSavePath)
{
    if (key->isModified()) return true;
    if (QFile::exists(strSavePath) == false) return true;
    if (strSavePath != key->fileName()) return true;  // ✅ Checks if moved!
    return false;
}
```

**Data Loss Scenario:**
1. Frame at position 5 has file `001.005.png`
2. User moves frame to position 10 (new expected path: `001.010.png`)
3. presave() renames `001.005.png` → temp → `001.010.png`
4. BUT needSaveFrame() returns `false` (frame not modified)
5. Frame data is skipped during save
6. XML references `001.010.png` but file might not exist
7. **Frame lost on reload**

### Fix Applied ✅

Added missing path comparison check:
```cpp
bool LayerBitmap::needSaveFrame(KeyFrame* key, const QString& savePath)
{
    if (key->isModified()) return true;
    if (QFile::exists(savePath) == false) return true;
    if (key->fileName().isEmpty()) return true;
    if (savePath != key->fileName()) return true;  // ✅ NOW CHECKS IF MOVED!
    return false;
}
```

This ensures bitmap layers match the robust behavior of vector layers.

---

## Bug #3: presave() Frame Renaming Has No Error Handling ⚠️ CRITICAL

**Severity:** CRITICAL
**Status:** Identified
**Location:** [layerbitmap.cpp:147-154](core_lib/src/structure/layerbitmap.cpp#L147-L154)

### Problem

File operations (remove, rename) in presave() have no error checking. If operations fail, the code assumes they succeeded and updates internal state accordingly, leading to data loss.

### When It Happens

- Disk full
- Insufficient permissions
- File locked by antivirus or backup software
- Network drive disconnection
- File system errors

### Technical Details

```cpp
for (BitmapImage* b : movedOnlyBitmaps)
{
    QString dest = filePath(b, dataFolder);
    QFile::remove(dest);              // ❌ No error check!

    QFile::rename(b->fileName(), dest);  // ❌ No error check! Returns bool
    b->setFileName(dest);             // ❌ Assumes rename succeeded!
}
```

**What happens when rename fails:**
1. `QFile::rename()` returns `false` (ignored)
2. File remains at old location
3. `setFileName(dest)` updates internal state to new location
4. XML references new location
5. **On reload, frame is silently skipped because file isn't where expected**

### Recommended Fix

```cpp
for (BitmapImage* b : movedOnlyBitmaps)
{
    QString dest = filePath(b, dataFolder);

    // Check if destination exists and remove it
    if (QFile::exists(dest))
    {
        if (!QFile::remove(dest))
        {
            DebugDetails dd;
            dd << QString("Error: Failed to remove destination file: %1").arg(dest);
            dd << QString("Reason: %1").arg(QFile(dest).errorString());
            return Status(Status::FAIL, dd);
        }
    }

    // Rename with error checking
    if (!QFile::rename(b->fileName(), dest))
    {
        DebugDetails dd;
        dd << QString("Error: Failed to rename frame file");
        dd << QString("From: %1").arg(b->fileName());
        dd << QString("To: %1").arg(dest);
        dd << QString("Reason: %1").arg(QFile(b->fileName()).errorString());
        return Status(Status::FAIL, dd);
    }

    b->setFileName(dest);
}

return Status::OK;
```

### Additional Improvements Needed

- Add transaction/rollback mechanism
- Keep list of successful operations to reverse on failure
- Don't modify internal state until all file operations succeed

---

## Bug #4: Silent Frame Skipping During Load

**Severity:** HIGH
**Status:** Identified
**Location:** [layerbitmap.cpp:214-222](core_lib/src/structure/layerbitmap.cpp#L214-L222)

### Problem

When loading frames, if a PNG/VEC file is missing or the path validation fails, the frame is silently skipped without any warning to the user. This makes debugging data loss issues nearly impossible.

### When It Happens

- PNG/VEC file is missing from data folder
- File was deleted outside of Pencil2D
- Path validation fails (security check)
- Corrupted archive
- Incomplete save operation

### Technical Details

```cpp
void LayerBitmap::loadDomElement(const QDomElement& element, QString dataDirPath, ...)
{
    // ... parse XML ...

    QString path = validateDataPath(imageElement.attribute("src"), dataDirPath);
    if (!path.isEmpty())
    {
        // Load frame...
        loadImageAtFrame(path, QPoint(x, y), position, opacity);
    }
    // ❌ BUG: If path is empty, frame is SILENTLY SKIPPED - no warning!

    progressStep();
    imageTag = imageTag.nextSibling();
}
```

**Same issue in vector layers** ([layervector.cpp](core_lib/src/structure/layervector.cpp)):
```cpp
QString path = validateDataPath(imageElement.attribute("src"), dataDirPath);
if (!path.isEmpty())
{
    loadImageAtFrame(path, position);
}
// ❌ Also silently skips missing frames
```

### User Impact

- User doesn't know data is missing until they see blank frames during playback
- No indication of which frames are missing
- No way to recover or identify the problem
- Appears as if frames were never created

### Recommended Fix

```cpp
QString path = validateDataPath(imageElement.attribute("src"), dataDirPath);
if (!path.isEmpty())
{
    loadImageAtFrame(path, QPoint(x, y), position, opacity);
}
else
{
    // Log warning
    qWarning() << "Warning: Frame file missing or invalid:"
               << imageElement.attribute("src")
               << "at frame position" << position
               << "in layer" << name();

    // Optionally collect missing frames for user notification
    mMissingFrames.append(position);
}
```

**Additional improvements:**
- Show warning dialog to user after load completes if frames are missing
- List which frames couldn't be loaded
- Offer to create empty replacement frames or abort load
- Add to error log for support debugging

---

## Bug #5: Save Continues After Frame Failures

**Severity:** HIGH
**Status:** Identified
**Location:** [filemanager.cpp:670-684](core_lib/src/structure/filemanager.cpp#L670-L684)

### Problem

If saving individual frames fails, the save process continues anyway and writes XML without the failed frames. The old archive is deleted and replaced with incomplete data, causing permanent data loss.

### When It Happens

- Disk full during save
- File write permissions denied
- Frame data corrupted
- Image encoding fails
- Any frame-level save error

### Technical Details

```cpp
Status FileManager::writeKeyFrameFiles(const Object* object, ...)
{
    bool saveLayersOK = true;
    for (int i = 0; i < numLayers; ++i)
    {
        Layer* layer = object->getLayer(i);
        Status st = layer->save(dataFolder, filesFlushed, [this] { progressForward(); });
        if (!st.ok())
        {
            saveLayersOK = false;  // ❌ Records failure but keeps going!
            dd.collect(st.details());
            dd << QString("\nError: Failed to save Layer[%1] %2").arg(i).arg(layer->name());
        }
    }

    progressForward();

    auto errorCode = (saveLayersOK) ? Status::OK : Status::FAIL;
    return Status(errorCode, dd);  // ❌ Returns error but caller may ignore
}
```

**In the main save function** ([filemanager.cpp:236-378](core_lib/src/structure/filemanager.cpp#L236-L378)):
```cpp
Status stKeyFrames = writeKeyFrameFiles(object, ...);
Status stMainXml = writeMainXml(object, ...);  // ❌ Still writes XML even if frames failed!
Status stPalette = writePalette(object, ...);

// Later...
Status stMiniz = MiniZ::compressFolder(...);  // ❌ Creates incomplete archive
if (!stMiniz.ok())
{
    return Status(Status::ERROR_MINIZ_FAIL, dd, ...);
}

if (saveOk) {
    dd << "Project saved successfully, deleting backup";
    deleteBackupFile(sBackupFile);  // ❌ Deletes backup of complete data!
}
```

### Data Loss Scenario

1. Project has 100 frames across 5 layers
2. During save, frame 50 on layer 3 fails to save (disk full, encoding error, etc.)
3. Error is logged but process continues
4. XML is written without frame 50 (or with invalid reference)
5. Incomplete project is compressed to .pclx
6. **Old backup is deleted**
7. User now has only the incomplete project with missing frame
8. **Original complete data is lost**

### Recommended Fix

```cpp
Status FileManager::writeKeyFrameFiles(const Object* object, ...)
{
    for (int i = 0; i < numLayers; ++i)
    {
        Layer* layer = object->getLayer(i);
        Status st = layer->save(dataFolder, filesFlushed, [this] { progressForward(); });
        if (!st.ok())
        {
            dd.collect(st.details());
            dd << QString("\nCRITICAL: Failed to save Layer[%1] %2").arg(i).arg(layer->name());
            dd << "Aborting save operation to prevent data loss";
            return Status(Status::FAIL, dd);  // ✅ ABORT IMMEDIATELY
        }
    }

    return Status::OK;
}
```

**In main save function:**
```cpp
Status stKeyFrames = writeKeyFrameFiles(object, ...);
if (!stKeyFrames.ok())
{
    // ✅ STOP! Don't write XML or compress
    dd.collect(stKeyFrames.details());
    dd << "Save aborted: Frame files could not be saved";
    dd << "Your project data is safe (previous save not modified)";
    return Status(Status::FAIL, dd);
}

Status stMainXml = writeMainXml(object, ...);
if (!stMainXml.ok())
{
    // ✅ STOP! Don't compress
    return Status(Status::FAIL, dd);
}

// ... continue only if everything succeeded
```

**Additional improvements:**
- Implement atomic save: write to temporary location first
- Only replace old file if new save is 100% complete
- Keep backup until new save is verified
- Add verification step before deleting backup
- Consider writing checksums for validation

---

## Summary and Priority

### Critical (Fix Immediately)
- **Bug #2**: ✅ **FIXED** - Moved frames not saved in bitmap layers
- **Bug #3**: presave() has no error handling - leads to data loss on file operation failures

### High Priority
- **Bug #4**: Silent frame skipping makes debugging impossible
- **Bug #5**: Partial saves overwrite good data

### Medium Priority
- **Bug #1**: Empty frames create broken XML references

### Testing Recommendations

1. **Frame reordering test**: Move frames without drawing, save/reload
2. **Disk full test**: Save to nearly-full drive, verify error handling
3. **File lock test**: Lock frame file with another process during save
4. **Missing file test**: Delete PNG files, reload project, check for warnings
5. **Large project test**: Save project with many frames, verify all saved

### Root Cause Analysis

All these bugs stem from similar architectural issues:
- **Insufficient error handling** on file operations
- **Inconsistent behavior** between bitmap and vector layers
- **Silent failures** without user notification
- **No transaction semantics** for multi-file operations
- **Optimistic assumptions** that operations succeed

### Long-term Recommendations

1. Implement comprehensive error handling for all file I/O
2. Add transaction/rollback mechanisms for save operations
3. Unify bitmap and vector layer save logic
4. Add logging and user notifications for data loss risks
5. Implement save verification and integrity checks
6. Consider atomic saves with temporary staging areas
7. Add unit tests for error conditions
8. Implement telemetry to track save failures in the wild

---

*Report generated: 2025-12-10*
*Pencil2D Repository: https://github.com/pencil2d/pencil2d*
