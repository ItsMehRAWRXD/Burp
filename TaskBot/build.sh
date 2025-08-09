#!/bin/bash

# TaskBot Build Script
# This script builds the TaskBot project

set -e  # Exit on error

echo "====================================="
echo "TaskBot Build Script"
echo "====================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check for required tools
echo "Checking requirements..."

if ! command -v cmake &> /dev/null; then
    echo -e "${RED}Error: CMake is not installed.${NC}"
    echo "Please install CMake: sudo apt-get install cmake"
    exit 1
fi

if ! command -v g++ &> /dev/null; then
    echo -e "${RED}Error: g++ is not installed.${NC}"
    echo "Please install g++: sudo apt-get install g++"
    exit 1
fi

# Check g++ version for C++17 support
GCC_VERSION=$(g++ -dumpversion | cut -f1 -d.)
if [ "$GCC_VERSION" -lt 7 ]; then
    echo -e "${YELLOW}Warning: g++ version is less than 7. C++17 support may be limited.${NC}"
fi

# Parse command line arguments
BUILD_TYPE="Release"
CLEAN_BUILD=false
RUN_DEMO=false

for arg in "$@"; do
    case $arg in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --run-demo)
            RUN_DEMO=true
            shift
            ;;
        --help)
            echo "Usage: ./build.sh [options]"
            echo "Options:"
            echo "  --debug      Build in debug mode"
            echo "  --clean      Clean build (remove build directory)"
            echo "  --run-demo   Run the demo after building"
            echo "  --help       Show this help message"
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $arg${NC}"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Create build directory
BUILD_DIR="build"

if [ "$CLEAN_BUILD" = true ]; then
    echo "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with CMake
echo -e "\n${GREEN}Configuring project...${NC}"
cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" ..

# Build the project
echo -e "\n${GREEN}Building project...${NC}"
make -j$(nproc)

echo -e "\n${GREEN}Build complete!${NC}"
echo "Executable: $BUILD_DIR/taskbot"

# Run demo if requested
if [ "$RUN_DEMO" = true ]; then
    echo -e "\n${GREEN}Running demo...${NC}"
    ./taskbot --demo
fi

echo -e "\n${GREEN}Done!${NC}"
echo "To run TaskBot: $BUILD_DIR/taskbot"
echo "To see demo: $BUILD_DIR/taskbot --demo"