# CMake Build System for Pencil2D

This directory contains the CMake build configuration for Pencil2D, which works alongside the existing qmake build system.

**Build Architecture:** This CMake configuration uses `include()` to combine all sources into a **single executable**, rather than building separate static libraries. This approach results in faster build times and a simpler build structure.

## Requirements

- CMake 3.16 or higher
- Qt 5.15+ or Qt 6.2+
- C++17 compatible compiler
- Platform-specific requirements:
  - **macOS**: Xcode Command Line Tools
  - **Windows**: Visual Studio 2019 or newer / MinGW
  - **Linux**: GCC 7+ or Clang 5+

## Quick Start

### Building with CMake

```bash
# Create build directory
mkdir build-cmake
cd build-cmake

# Configure (Qt 6 example)
cmake .. -DCMAKE_BUILD_TYPE=Release

# Or with Qt 5
cmake .. -DCMAKE_BUILD_TYPE=Release -DQt5_DIR=/path/to/qt5

# Build
cmake --build .

# Run tests (optional)
ctest

# Install (optional)
cmake --install .
```

### macOS Specific

```bash
mkdir build-cmake
cd build-cmake

# Configure for macOS
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/macos

# Build
cmake --build .

# The app bundle will be in: app/Pencil2D.app
```

### Windows Specific

```bash
# Using Visual Studio
mkdir build-cmake
cd build-cmake

cmake .. -G "Visual Studio 17 2022" -A x64 ^
         -DCMAKE_PREFIX_PATH=C:\Qt\6.x.x\msvc2019_64

cmake --build . --config Release
```

### Linux Specific

```bash
mkdir build-cmake
cd build-cmake

cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_INSTALL_PREFIX=/usr \
         -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/gcc_64

cmake --build .

# Install to system
sudo cmake --install .
```

## Build Options

CMake provides several options to customize the build:

```bash
# Disable tests
cmake .. -DBUILD_TESTS=OFF

# Nightly build
cmake .. -DPENCIL2D_NIGHTLY=ON

# Release build with optimizations
cmake .. -DPENCIL2D_RELEASE=ON -DCMAKE_BUILD_TYPE=Release

# Custom version
cmake .. -DAPP_VERSION="0.7.0"

# Specify Qt version explicitly
cmake .. -DQT_VERSION_MAJOR=6
```

## Building with Qt Creator

1. Open CMakeLists.txt in Qt Creator
2. Configure the project with your preferred Qt kit
3. Build and run

Qt Creator will automatically detect the CMake project and configure it appropriately.

## Comparing CMake vs qmake

Both build systems are supported. Here's when to use each:

### Use CMake when:
- You prefer modern CMake build practices
- You're integrating with other CMake projects
- You want better IDE support (CLion, VS Code, etc.)
- You need more flexible build configuration

### Use qmake when:
- You're familiar with the existing qmake build
- You need the exact build configuration used in CI/CD
- You're following existing build documentation

## Project Structure

```
pencil2d/
├── CMakeLists.txt              # Root CMake configuration
├── core_lib/
│   └── CMakeLists.txt          # Core library configuration
├── app/
│   └── CMakeLists.txt          # Application configuration
└── tests/
    └── CMakeLists.txt          # Tests configuration
```

## Advanced Usage

### Out-of-Source Builds (Recommended)

Always build in a separate directory:

```bash
# Create multiple build configurations
mkdir build-debug build-release
cd build-debug
cmake .. -DCMAKE_BUILD_TYPE=Debug
cd ../build-release
cmake .. -DCMAKE_BUILD_TYPE=Release
```

### Using ccache for Faster Builds

```bash
cmake .. -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
```

### Ninja Build System

For faster parallel builds:

```bash
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja
```

### IDE Integration

#### Visual Studio Code

Install the CMake Tools extension, then:
1. Open the pencil2d folder
2. Select a kit (Qt configuration)
3. Configure and build from the status bar

#### CLion

Simply open the root CMakeLists.txt file. CLion will automatically configure the project.

## Troubleshooting

### Qt Not Found

If CMake can't find Qt:

```bash
# Specify Qt path explicitly
cmake .. -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/platform
```

### Linking Errors

Ensure you're using matching compiler for Qt and your build:
- **Windows MSVC**: Use the MSVC Qt build
- **Windows MinGW**: Use the MinGW Qt build
- **macOS**: Use the macOS Qt build
- **Linux**: Use the GCC/Clang Qt build matching your compiler

### Precompiled Headers Issues

If you encounter PCH issues, you can disable them:

```bash
cmake .. -DCMAKE_DISABLE_PRECOMPILE_HEADERS=ON
```

## Contributing

When adding new source files:

1. Add them to the appropriate `CMakeLists.txt` file
2. Also add them to the corresponding `.pro` file to maintain qmake compatibility
3. Test both build systems

## Migration Notes

The CMake build system is designed to be compatible with the existing qmake build. Both will be maintained in parallel to ensure:

- CI/CD pipelines continue to work
- Developers can choose their preferred build system
- Gradual migration path for the project

## Getting Help

- Check existing build documentation in `docs/`
- Review GitHub Actions workflows in `.github/workflows/`
- Open an issue on GitHub for build-specific problems

## License

Same as Pencil2D - see LICENSE.TXT
