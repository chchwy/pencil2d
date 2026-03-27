# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Pencil2D is a free and open source 2D animation software built with Qt and C++. It enables traditional hand-drawn animation using both bitmap and vector graphics.

## Build System

The project uses **Qt's qmake** build system with `.pro` project files.

### Building the Project

**With Qt Creator (Recommended):**
1. Open `pencil2d.pro` in Qt Creator
2. Configure kits (Desktop option should be selected)
3. Click the green arrow or press `Ctrl+R` (Windows/Linux) / `Cmd+R` (macOS)

**Command Line (Linux/macOS):**
```bash
# Create build directory
mkdir build && cd build

# Configure with qmake
qmake -spec <mkspec> CONFIG+=debug ..
# For Qt 6 on Linux, use: qmake6 -spec <mkspec> CONFIG+=debug ..

# Build
make

# Run application
./bin/pencil2d
```

**Command Line (Windows with MSVC):**
```cmd
mkdir build
cd build
qmake -spec win32-msvc CONFIG+=debug ..
nmake
```

### Build Configuration Options

Pass these to qmake via `CONFIG+=<option>`:

- `CONFIG+=debug` - Development build (default on non-Windows)
- `CONFIG+=release` - Optimized release build
- `CONFIG+=NO_TESTS` - Skip building unit tests
- `CONFIG+=PENCIL2D_NIGHTLY` - Mark as nightly build
- `CONFIG+=PENCIL2D_RELEASE` - Mark as official release
- `CONFIG+=GIT` - Include git repository info in build

### Running Tests

**From Qt Creator:**
Tests build and run automatically as part of the build process.

**Command Line:**
```bash
# Build the project first, then:
./build/tests/tests

# Run headless (useful for CI):
QT_QPA_PLATFORM=minimal ./build/tests/tests
```

Tests use the Catch v2 framework and are located in `tests/src/`.

## Project Architecture

The codebase is organized into 3 sub-projects:

### 1. `core_lib/` - Core Animation Engine

The heart of Pencil2D containing all animation logic, drawing operations, and tool implementations.

**Key Components:**

- **`structure/`** - Data model for animation projects
  - `Object` - Top-level animation project container holding layers and frames
  - `Layer` - Base class for layer types (Bitmap, Vector, Sound, Camera)
  - `KeyFrame` - Base class for keyframe types (BitmapImage, VectorImage, SoundClip, Camera)
  - `FileManager` - Handles loading/saving .pclx project files

- **`managers/`** - Manager classes that coordinate functionality
  - `BaseManager` - Base class; all managers hold references to `Editor` and `Object`
  - `ColorManager` - Color palette and color selection
  - `LayerManager` - Layer creation, deletion, and manipulation
  - `ToolManager` - Tool state and tool switching
  - `ViewManager` - Canvas view transformations (zoom, pan, rotation)
  - `PlaybackManager` - Animation playback controls
  - `SoundManager` - Audio playback
  - `SelectionManager` - Selection state and transformations
  - `UndoRedoManager` - Undo/redo command stack

- **`tool/`** - Drawing and editing tools
  - `BaseTool` - Base class for all tools
  - Tool implementations: `PencilTool`, `BrushTool`, `EraserTool`, `BucketTool`, `SelectTool`, etc.

- **`graphics/`** - Image classes and rendering
  - `bitmap/` - Bitmap image handling
  - `vector/` - Vector graphics handling

### 2. `app/` - GUI Application

Everything related to the user interface and Qt widgets.

**Key Components:**

- `main.cpp` - Application entry point (initializes `Pencil2D` class)
- `MainWindow2` - Main application window
  - Bridge between GUI widgets and core engine
  - Creates `Object`, `Editor`, `ScribbleArea`, and all panels
  - Handles menu actions (see `MainWindow2::createMenus()`)

- `Editor` - Central coordinator between UI and core engine
  - Holds references to all manager classes
  - Acts as bridge between `MainWindow2` and core functionality

- `ScribbleArea` - Main drawing canvas
  - Handles mouse/tablet input events
  - Manages selection transformations
  - Renders canvas cursor and bitmap buffer
  - Creates stroke dabs for drawing operations

- `Timeline` - Timeline widget (top row with buttons)
- `TimelineCells` - Timeline cell grid (where keyframes are displayed)
- `ToolBox` - Tool selection panel
- `ColorPaletteWidget` - Color palette panel
- Various dialogs and preference pages

### 3. `tests/` - Unit Tests

Test suite using Catch v2 framework. Tests cover:
- Bitmap and vector images
- Layer types and operations
- Managers (Color, Layer, View, File)
- Tool properties and settings
- Object serialization

## Key Classes and Data Flow

### Core Architecture Pattern

```
User Action → MainWindow2 → ActionCommands → Editor → Manager → Object/Layer/KeyFrame
                    ↓
              ScribbleArea (for canvas operations)
```

### Important: ActionCommands

Methods in `ActionCommands` should **only** be bound to direct user actions from menus/toolbar. If a behavior is triggered by UI elements within a widget (buttons, etc.), implement it in the widget or appropriate manager - **not** in `ActionCommands`.

### Manager Independence

Manager classes are independent of each other and derive from `BaseManager`. They:
- Hold references to `Editor` and `Object`
- Encapsulate specific domains of functionality
- Should not depend on other managers
- Are designed to be testable in isolation

## Development Workflow

### Starting Point for New Developers

Focus on these core classes first:
1. **`MainWindow2`** (`app/src/mainwindow2.cpp`) - UI entry point
2. **`Editor`** (`core_lib/src/interface/editor.cpp`) - Central coordinator
3. **`Object`** (`core_lib/src/structure/object.cpp`) - Data model
4. **`ScribbleArea`** (`app/src/scribblearea.cpp`) - Drawing canvas

### Understanding Features

To understand how a feature works:
1. Find the menu action in `MainWindow2::createMenus()`
2. Follow the connected slot/method (e.g., `MainWindow2::openDocument()`)
3. Trace through `ActionCommands` and manager calls
4. Examine the relevant manager and data structure classes

### File Formats

- **`.pclx`** - Pencil2D project file (ZIP archive containing XML and assets)
- **`.pcl`** - Legacy project format
- Opening a project converts .pclx to `Object` structure in memory
- Saving converts `Object` back to .pclx format

## Qt Framework Notes

- The project uses Qt's signal/slot mechanism for event handling
- Widgets are organized using Qt's dock widget system
- Resources are managed via `.qrc` files (e.g., `tests/data/tests.qrc`)
- Use Qt Creator's "Go to Definition" (Ctrl+Click) to navigate signal/slot connections

## Dependencies

- **Qt 5.6+** or **Qt 6** (5.6 minimum, latest LTS recommended)
- **Qt Multimedia** module (for sound playback)
- **Qt SVG** module
- C++11 compiler (GCC, Clang, or MSVC)

## Platform-Specific Build Notes

Detailed platform-specific build instructions are in:
- `docs/build_win.md` - Windows (MSVC or MinGW)
- `docs/build_linux.md` - Linux (Ubuntu/Arch)
- `docs/build_mac.md` - macOS

## File Navigation

Use reference format `file_path:line_number` when discussing code locations.

Example: The `MainWindow2` constructor is in `app/src/mainwindow2.cpp:45`
