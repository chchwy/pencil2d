@echo off
REM CMake build script for Pencil2D on Windows
REM This script provides a convenient way to build Pencil2D using CMake

setlocal enabledelayedexpansion

REM Default values
set BUILD_TYPE=Release
set BUILD_DIR=build-cmake
set BUILD_TESTS=ON
set CLEAN_BUILD=0
set QT_PATH=
set JOBS=%NUMBER_OF_PROCESSORS%
set INSTALL=0
set NIGHTLY=0
set GENERATOR=

:parse_args
if "%~1"=="" goto end_parse
if /i "%~1"=="-h" goto show_help
if /i "%~1"=="--help" goto show_help
if /i "%~1"=="-d" (
    set BUILD_TYPE=Debug
    shift
    goto parse_args
)
if /i "%~1"=="--debug" (
    set BUILD_TYPE=Debug
    shift
    goto parse_args
)
if /i "%~1"=="-r" (
    set BUILD_TYPE=Release
    shift
    goto parse_args
)
if /i "%~1"=="--release" (
    set BUILD_TYPE=Release
    shift
    goto parse_args
)
if /i "%~1"=="-c" (
    set CLEAN_BUILD=1
    shift
    goto parse_args
)
if /i "%~1"=="--clean" (
    set CLEAN_BUILD=1
    shift
    goto parse_args
)
if /i "%~1"=="-t" (
    set BUILD_TESTS=OFF
    shift
    goto parse_args
)
if /i "%~1"=="--no-tests" (
    set BUILD_TESTS=OFF
    shift
    goto parse_args
)
if /i "%~1"=="-j" (
    set JOBS=%~2
    shift
    shift
    goto parse_args
)
if /i "%~1"=="--jobs" (
    set JOBS=%~2
    shift
    shift
    goto parse_args
)
if /i "%~1"=="-q" (
    set QT_PATH=%~2
    shift
    shift
    goto parse_args
)
if /i "%~1"=="--qt-path" (
    set QT_PATH=%~2
    shift
    shift
    goto parse_args
)
if /i "%~1"=="-b" (
    set BUILD_DIR=%~2
    shift
    shift
    goto parse_args
)
if /i "%~1"=="--build-dir" (
    set BUILD_DIR=%~2
    shift
    shift
    goto parse_args
)
if /i "%~1"=="--nightly" (
    set NIGHTLY=1
    shift
    goto parse_args
)
if /i "%~1"=="--install" (
    set INSTALL=1
    shift
    goto parse_args
)
if /i "%~1"=="-g" (
    set GENERATOR=%~2
    shift
    shift
    goto parse_args
)
if /i "%~1"=="--generator" (
    set GENERATOR=%~2
    shift
    shift
    goto parse_args
)
echo Unknown option: %~1
goto show_help

:end_parse

REM Print configuration
echo === Pencil2D CMake Build ===
echo Build Type:    %BUILD_TYPE%
echo Build Dir:     %BUILD_DIR%
echo Build Tests:   %BUILD_TESTS%
echo Jobs:          %JOBS%
if not "%QT_PATH%"=="" echo Qt Path:       %QT_PATH%
if not "%GENERATOR%"=="" echo Generator:     %GENERATOR%
if %NIGHTLY%==1 echo Nightly:       Yes
echo.

REM Clean if requested
if %CLEAN_BUILD%==1 (
    if exist "%BUILD_DIR%" (
        echo Cleaning build directory...
        rmdir /s /q "%BUILD_DIR%"
    )
)

REM Create build directory
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%"

REM Prepare CMake arguments
set CMAKE_ARGS=-DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DBUILD_TESTS=%BUILD_TESTS%

if not "%QT_PATH%"=="" (
    set CMAKE_ARGS=%CMAKE_ARGS% -DCMAKE_PREFIX_PATH=%QT_PATH%
)

if %NIGHTLY%==1 (
    set CMAKE_ARGS=%CMAKE_ARGS% -DPENCIL2D_NIGHTLY=ON
)

if not "%GENERATOR%"=="" (
    set CMAKE_ARGS=%CMAKE_ARGS% -G "%GENERATOR%"
)

REM Configure
echo Configuring...
cmake .. %CMAKE_ARGS%
if errorlevel 1 goto error

REM Build
echo Building...
cmake --build . --config %BUILD_TYPE% --parallel %JOBS%
if errorlevel 1 goto error

REM Run tests
if "%BUILD_TESTS%"=="ON" (
    echo Running tests...
    ctest -C %BUILD_TYPE% --output-on-failure
)

REM Install
if %INSTALL%==1 (
    echo Installing...
    cmake --install . --config %BUILD_TYPE%
)

echo.
echo === Build Complete ===
echo.
echo Executable location:
echo   app\%BUILD_TYPE%\pencil2d.exe
echo.
goto end

:show_help
echo Usage: %~nx0 [OPTIONS]
echo.
echo Build Pencil2D using CMake
echo.
echo OPTIONS:
echo     -h, --help              Show this help message
echo     -d, --debug             Build in Debug mode (default: Release)
echo     -r, --release           Build in Release mode (default)
echo     -c, --clean             Clean build directory before building
echo     -t, --no-tests          Don't build tests
echo     -j, --jobs NUM          Number of parallel jobs (default: %JOBS%)
echo     -q, --qt-path PATH      Path to Qt installation
echo     -b, --build-dir DIR     Build directory (default: %BUILD_DIR%)
echo     -g, --generator GEN     CMake generator (e.g., "Visual Studio 17 2022")
echo     --nightly               Build as nightly version
echo     --install               Install after building
echo.
echo EXAMPLES:
echo     %~nx0                                    # Basic release build
echo     %~nx0 --debug --clean                    # Clean debug build
echo     %~nx0 --qt-path C:\Qt\6.5.0\msvc2019_64  # Specify Qt path
echo     %~nx0 --generator "Ninja"                # Use Ninja generator
echo.
goto end

:error
echo.
echo Build failed!
exit /b 1

:end
cd ..
endlocal
