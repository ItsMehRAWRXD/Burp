@echo off
echo ========================================================================================
echo SIMPLE UNIFIED BENIGN PACKER BUILD SCRIPT
echo ========================================================================================
echo.

REM Check for Visual Studio 2022
where cl.exe >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] Visual Studio 2022 compiler (cl.exe) not found!
    echo Please run this script from "Developer Command Prompt for VS 2022"
    pause
    exit /b 1
)

echo [SUCCESS] Found Visual Studio 2022 compiler: cl.exe
echo.

REM Create output directories
if not exist "output" mkdir output
if not exist "temp" mkdir temp

echo ========================================================================================
echo COMPILING UNIFIED BENIGN PACKER...
echo ========================================================================================

REM Check if UNIFIED_BENIGN_PACKER.cpp exists
if not exist "UNIFIED_BENIGN_PACKER.cpp" (
    echo [ERROR] UNIFIED_BENIGN_PACKER.cpp not found!
    pause
    exit /b 1
)

REM Compile the unified framework
echo [COMPILE] Compiling UNIFIED_BENIGN_PACKER.cpp...
cl.exe /std:c++17 /O2 /MT /DWIN32_LEAN_AND_MEAN /EHsc UNIFIED_BENIGN_PACKER.cpp /link /SUBSYSTEM:CONSOLE /MACHINE:x64 kernel32.lib user32.lib advapi32.lib shell32.lib psapi.lib wincrypt.lib wininet.lib /OUT:UnifiedBenignPacker.exe

if %errorlevel% neq 0 (
    echo [ERROR] Failed to compile UNIFIED_BENIGN_PACKER.cpp
    pause
    exit /b 1
)

echo ========================================================================================
echo BUILD SUCCESSFUL!
echo ========================================================================================
echo.
echo [SUCCESS] Generated: UnifiedBenignPacker.exe
echo.

REM Create a test payload for testing
echo [TEST] Creating test payload...
echo Test payload data for Unified BenignPacker > test_payload.bin
echo.

echo ========================================================================================
echo TESTING THE UNIFIED FRAMEWORK...
echo ========================================================================================

if exist "test_payload.bin" (
    echo [TEST] Testing with sample payload...
    UnifiedBenignPacker.exe test_payload.bin test_unified_output.exe unified
    if %errorlevel% equ 0 (
        echo.
        echo [SUCCESS] Test completed successfully!
        echo Generated: test_unified_output.exe
    ) else (
        echo.
        echo [WARNING] Test failed, but build was successful.
        echo You can still use the application manually.
    )
) else (
    echo [INFO] No test payload found. Build completed successfully.
)

echo.
echo ========================================================================================
echo UNIFIED BENIGN PACKER - READY TO USE!
echo ========================================================================================
echo.
echo [INFO] Your Unified BenignPacker is ready!
echo.
echo [USAGE] Examples:
echo   UnifiedBenignPacker.exe payload.bin
echo   UnifiedBenignPacker.exe payload.bin output.exe unified
echo   UnifiedBenignPacker.exe payload.bin pe_output.exe pe_encrypt
echo   UnifiedBenignPacker.exe payload.bin fileless_output.exe fileless
echo   UnifiedBenignPacker.exe payload.bin exploit_output.exe exploit
echo   UnifiedBenignPacker.exe payload.bin stub_output.exe stub
echo.
echo [METHODS] Available:
echo   unified     - Complete unified framework (default)
echo   pe_encrypt  - PE encryption and packing
echo   fileless    - Fileless execution system
echo   exploit     - Advanced exploit framework
echo   stub        - Unique stub generation (71 variants)
echo.
pause