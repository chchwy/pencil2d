#!/usr/bin/env bash

# CMake build script for Pencil2D
# This script provides a convenient way to build Pencil2D using CMake

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Default values
BUILD_TYPE="Release"
BUILD_DIR="build-cmake"
BUILD_TESTS="ON"
CLEAN_BUILD=false
QT_PATH=""
JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Print usage
usage() {
    cat << EOF
Usage: $0 [OPTIONS]

Build Pencil2D using CMake

OPTIONS:
    -h, --help              Show this help message
    -d, --debug             Build in Debug mode (default: Release)
    -r, --release           Build in Release mode (default)
    -c, --clean             Clean build directory before building
    -t, --no-tests          Don't build tests
    -j, --jobs NUM          Number of parallel jobs (default: $JOBS)
    -q, --qt-path PATH      Path to Qt installation
    -b, --build-dir DIR     Build directory (default: $BUILD_DIR)
    --nightly               Build as nightly version
    --install               Install after building

EXAMPLES:
    $0                              # Basic release build
    $0 --debug --clean              # Clean debug build
    $0 --qt-path ~/Qt/6.5.0/macos   # Specify Qt path
    $0 --jobs 8                     # Use 8 parallel jobs

EOF
    exit 0
}

# Parse arguments
INSTALL=false
NIGHTLY=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            usage
            ;;
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -r|--release)
            BUILD_TYPE="Release"
            shift
            ;;
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        -t|--no-tests)
            BUILD_TESTS="OFF"
            shift
            ;;
        -j|--jobs)
            JOBS="$2"
            shift 2
            ;;
        -q|--qt-path)
            QT_PATH="$2"
            shift 2
            ;;
        -b|--build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        --nightly)
            NIGHTLY=true
            shift
            ;;
        --install)
            INSTALL=true
            shift
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            usage
            ;;
    esac
done

# Print configuration
echo -e "${GREEN}=== Pencil2D CMake Build ===${NC}"
echo "Build Type:    $BUILD_TYPE"
echo "Build Dir:     $BUILD_DIR"
echo "Build Tests:   $BUILD_TESTS"
echo "Jobs:          $JOBS"
[[ -n "$QT_PATH" ]] && echo "Qt Path:       $QT_PATH"
[[ "$NIGHTLY" == true ]] && echo "Nightly:       Yes"
echo ""

# Clean if requested
if [[ "$CLEAN_BUILD" == true ]] && [[ -d "$BUILD_DIR" ]]; then
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf "$BUILD_DIR"
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Prepare CMake arguments
CMAKE_ARGS=(
    "-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
    "-DBUILD_TESTS=$BUILD_TESTS"
)

[[ -n "$QT_PATH" ]] && CMAKE_ARGS+=("-DCMAKE_PREFIX_PATH=$QT_PATH")
[[ "$NIGHTLY" == true ]] && CMAKE_ARGS+=("-DPENCIL2D_NIGHTLY=ON")

# Configure
echo -e "${GREEN}Configuring...${NC}"
cmake .. "${CMAKE_ARGS[@]}"

# Build
echo -e "${GREEN}Building...${NC}"
cmake --build . --parallel "$JOBS"

# Run tests
if [[ "$BUILD_TESTS" == "ON" ]]; then
    echo -e "${GREEN}Running tests...${NC}"
    ctest --output-on-failure
fi

# Install
if [[ "$INSTALL" == true ]]; then
    echo -e "${GREEN}Installing...${NC}"
    cmake --install .
fi

echo -e "${GREEN}=== Build Complete ===${NC}"
echo ""
echo "Executable location:"
if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "  app/Pencil2D.app"
    echo ""
    echo "To run: open app/Pencil2D.app"
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
    echo "  app/$BUILD_TYPE/pencil2d.exe"
else
    echo "  app/pencil2d"
fi
