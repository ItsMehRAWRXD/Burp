#!/bin/bash

echo "========================================================================================"
echo "UNIFIED BENIGN PACKER BUILD SCRIPT (Linux)"
echo "========================================================================================"
echo

# Create output directories
mkdir -p output
mkdir -p temp

echo "========================================================================================"
echo "COMPILING UNIFIED BENIGN PACKER..."
echo "========================================================================================"

# Check if UNIFIED_BENIGN_PACKER.cpp exists
if [ ! -f "UNIFIED_BENIGN_PACKER.cpp" ]; then
    echo "[ERROR] UNIFIED_BENIGN_PACKER.cpp not found!"
    exit 1
fi

# Compile the unified framework
echo "[COMPILE] Compiling UNIFIED_BENIGN_PACKER.cpp..."
g++ -std=c++17 -O2 -DWIN32_LEAN_AND_MEAN \
    UNIFIED_BENIGN_PACKER.cpp \
    -o UnifiedBenignPacker \
    -lpthread -lstdc++fs \
    2>&1 | tee compile_errors.log

if [ $? -ne 0 ]; then
    echo "[ERROR] Failed to compile UNIFIED_BENIGN_PACKER.cpp"
    echo "See compile_errors.log for details"
    exit 1
fi

echo "========================================================================================"
echo "BUILD SUCCESSFUL!"
echo "========================================================================================"
echo
echo "[SUCCESS] Generated: UnifiedBenignPacker"
echo