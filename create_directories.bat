@echo off
echo Creating output directories...

REM Create bin directories for all configurations and platforms
mkdir bin\Debug\Win32 2>nul
mkdir bin\Debug\x64 2>nul
mkdir bin\Release\Win32 2>nul
mkdir bin\Release\x64 2>nul

REM Create obj directories
mkdir obj\Debug\Win32\PluginFramework 2>nul
mkdir obj\Debug\x64\PluginFramework 2>nul
mkdir obj\Release\Win32\PluginFramework 2>nul
mkdir obj\Release\x64\PluginFramework 2>nul

mkdir obj\Debug\Win32\BenignPacker 2>nul
mkdir obj\Debug\x64\BenignPacker 2>nul
mkdir obj\Release\Win32\BenignPacker 2>nul
mkdir obj\Release\x64\BenignPacker 2>nul

mkdir obj\Debug\Win32\UniqueStub71Plugin 2>nul
mkdir obj\Debug\x64\UniqueStub71Plugin 2>nul
mkdir obj\Release\Win32\UniqueStub71Plugin 2>nul
mkdir obj\Release\x64\UniqueStub71Plugin 2>nul

mkdir obj\Debug\Win32\MASMAssemblerPlugin 2>nul
mkdir obj\Debug\x64\MASMAssemblerPlugin 2>nul
mkdir obj\Release\Win32\MASMAssemblerPlugin 2>nul
mkdir obj\Release\x64\MASMAssemblerPlugin 2>nul

echo Directories created successfully!