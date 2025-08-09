@echo off
REM TaskBot Windows Build Script
REM Pure code generation and automation - no BS

echo ====================================
echo TaskBot Windows Build Script
echo ====================================

REM Set MSYS2 paths
set MSYS2_PATH=C:\msys64
set MINGW64_PATH=%MSYS2_PATH%\mingw64
set PATH=%MINGW64_PATH%\bin;%MSYS2_PATH%\usr\bin;%PATH%

REM Check for compiler
where gcc >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] GCC not found. Run from MSYS2 MinGW64 terminal
    exit /b 1
)

REM Build options
set BUILD_TYPE=Release
set CLEAN_BUILD=0

:parse_args
if "%1"=="" goto build
if "%1"=="--debug" set BUILD_TYPE=Debug
if "%1"=="--clean" set CLEAN_BUILD=1
shift
goto parse_args

:build
REM Install deps
echo Installing dependencies...
pacman -S --needed --noconfirm ^
    mingw-w64-x86_64-gcc ^
    mingw-w64-x86_64-cmake ^
    mingw-w64-x86_64-curl ^
    mingw-w64-x86_64-jsoncpp

REM Build directory
set BUILD_DIR=build_win
if %CLEAN_BUILD%==1 rmdir /s /q %BUILD_DIR% 2>nul
if not exist %BUILD_DIR% mkdir %BUILD_DIR%
cd %BUILD_DIR%

REM Configure
echo Configuring...
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ..

REM Build
echo Building TaskBot...
mingw32-make -j%NUMBER_OF_PROCESSORS%

echo.
echo Build complete: %BUILD_DIR%\taskbot.exe
cd ..