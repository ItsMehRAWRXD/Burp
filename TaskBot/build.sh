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

# Check for required libraries
echo "Checking for required libraries..."

MISSING_DEPS=""

# Check for X11 development files
if ! pkg-config --exists x11; then
    MISSING_DEPS="$MISSING_DEPS libx11-dev"
fi

# Check for XTest
if ! pkg-config --exists xtst; then
    MISSING_DEPS="$MISSING_DEPS libxtst-dev"
fi

# Check for CURL
if ! pkg-config --exists libcurl; then
    MISSING_DEPS="$MISSING_DEPS libcurl4-openssl-dev"
fi

# Check for jsoncpp
if ! pkg-config --exists jsoncpp; then
    MISSING_DEPS="$MISSING_DEPS libjsoncpp-dev"
fi

if [ ! -z "$MISSING_DEPS" ]; then
    echo -e "${YELLOW}Missing dependencies: $MISSING_DEPS${NC}"
    echo "Install them with:"
    echo -e "${GREEN}sudo apt-get install $MISSING_DEPS${NC}"
    echo ""
    read -p "Do you want to install them now? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        sudo apt-get update
        sudo apt-get install -y $MISSING_DEPS
    else
        exit 1
    fi
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