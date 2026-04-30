# SQLite Project File Plan for Pencil2D

## Goal
Add a SQLite-based project format that can coexist with existing .pclx projects while preserving current workflows and maximizing crash safety.

## Decisions
- Keep dual support: .pclx and SQLite format in parallel
- Keep .pclx as default until SQLite backend is proven stable
- Use direct on-disk SQLite with WAL mode
- Prioritize robustness and recovery over early optimization

## Format
- Proposed extension: .pcsq
- Existing extensions remain: .pclx and .pcl

## Phases

### Phase 1: Foundation
1. Add a storage backend abstraction interface
2. Add SQLite backend scaffold with connection, PRAGMA setup, and schema bootstrap
3. Integrate backend files into CMake and qmake builds
4. Add file format constants and file dialog filters for .pcsq
5. Add FileManager routing hook for .pcsq load/save paths

### Phase 2: Read Path
1. Implement SQLite schema for metadata, layers, keyframes, palette, and frame payloads
2. Implement loadProject() in SQLite backend
3. Map database records to Object/Layer/KeyFrame in-memory structures
4. Validate with round-trip tests against existing sample projects

### Phase 3: Write Path
1. Implement saveProject() with transactions
2. Implement incrementalSave() for autosave behavior
3. Add integrity verification and clear error reporting

### Phase 4: Migration and UX
1. Add Save As option for .pcsq
2. Add conversion path from .pclx to .pcsq
3. Keep fallback path for existing .pclx projects

### Phase 5: Hardening
1. Add migration/versioning for schema changes
2. Add corruption and crash-recovery tests
3. Benchmark open/save performance and file size tradeoffs

## Implementation Notes
- SQLite backend should use a dedicated connection name per opened file
- Enable pragmas: foreign_keys, journal_mode=WAL, synchronous=NORMAL
- Keep schema version table for future migrations
- Keep behavior transparent to upper layers through backend interface

## Implementation Status

### Completed Phases

**Phase 1: Foundation** ✓
- Storage backend abstraction interface (`ProjectStorageBackend`)
- SQLite backend implementation (`ProjectStorageBackendSqlite`)
- CMake, qmake, and Visual Studio project integration
- `.pcsq` format support in `FileFormat.h` and dialog filters
- `FileManager` routing for .pcsq load/save

**Phase 2: Read Path** ✓
- SQLite schema for project metadata, layers, keyframes, and assets
- `loadProject()` fully implemented with asset restoration
- XML deserialization for Object/Layer/KeyFrame structures
- Round-trip regression tests for bitmap, vector, and sound layers
- Validated against `.pclx` fixture projects

**Phase 3: Write Path** ✓
- `saveProject()` with transactional integrity
- Asset persistence: bitmap images, vector drawings, sound files stored as BLOBs
- XML serialization with schema version tracking
- Verified save/load cycle for all layer types

**Phase 4: Migration and UX** ✓
- Schema versioning with automatic migration (0 → 1)
- `.pclx` → `.pcsq` conversion coverage with fixture projects
- File extension routing in `FileManager`
- Layer structure preservation during conversion

**Phase 5: Hardening** (in progress)
- ✓ Schema version rejection for future formats
- ✓ Schema version 0→1 migration on save
- ✓ Corruption recovery for missing schema_version rows
- ✓ Asset-store validation: reject projects with missing asset_files entries
- ⏳ Table corruption coverage (missing/malformed tables)
- ⏳ Project document XML corruption rejection

### Recent Commits
- `d31f7752`: PCLX-to-SQLite conversion round-trip coverage
- `2633d226`: SQLite corruption and schema recovery tests
- `0faad09f`: SQLite empty asset-store corruption rejection

### Immediate Next Steps

1. **Add table-structure corruption tests**
   - Missing `asset_files` table during load
   - Malformed `project_document.main_xml` (invalid XML)
   - Truncated asset BLOB content
   - Expected behavior: Load rejection with `ERROR_INVALID_PENCIL_FILE`

2. **Edge case coverage**
   - Large projects (>1000 frames)
   - Projects with many layers (>100)
   - Unicode-heavy layer/file names
   - Concurrent save attempts

3. **Final validation**
   - Run full test suite: `tests.exe "*SQLite*" "*Conversion*"`
   - Manual app testing: open/save/convert workflows
   - Performance baseline: save/load timing for various project sizes

4. **Commit and closure**
   - Final hardening phase commit
   - Mark feature complete on branch
   - Plan merge strategy to main

## Testing Commands
```
# Run SQLite tests
tests.exe "*SQLite*"

# Run conversion tests
tests.exe "*Conversion*"

# Run all FileManager tests
tests.exe "FileManager*"
```

## Build Commands
```
# Full rebuild (from build/ directory)
MSBuild.exe app\pencil2d.vcxproj /p:Configuration=Debug /p:Platform=x64 /m:4
MSBuild.exe core_lib\core_lib.vcxproj /p:Configuration=Debug /p:Platform=x64 /m:4
MSBuild.exe tests\tests.vcxproj /p:Configuration=Debug /p:Platform=x64 /m:4

# Deploy and test
cd app\debug && call C:\Qt\6.5.3\msvc2019_64\bin\windeployqt.exe pencil2d.exe
```
