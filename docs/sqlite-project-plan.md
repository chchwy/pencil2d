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

## Current Slice
This first implementation slice includes:
- Backend interface declaration
- SQLite backend scaffold
- Initial .pcsq extension and filters
- FileManager routing entry points for SQLite format
