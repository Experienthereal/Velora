#!/bin/bash
# Velora Build Script for Linux/macOS

set -e

BUILD_DIR="build"
BUILD_TYPE="Release"

echo ""
echo " =========================================="
echo "  Velora Programming Language - Build"
echo " =========================================="
echo ""

# Check for CMake
if ! command -v cmake &> /dev/null; then
    echo "[ERROR] CMake not found. Install with: sudo apt install cmake"
    exit 1
fi

# Check for C++ compiler
if command -v g++ &> /dev/null; then
    echo "[INFO] Using GCC"
elif command -v clang++ &> /dev/null; then
    echo "[INFO] Using Clang"
else
    echo "[ERROR] No C++ compiler found."
    exit 1
fi

# Create build dir
mkdir -p "$BUILD_DIR"

# Configure
echo "[1/3] Configuring..."
cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" . -Wno-dev > /dev/null

# Build
echo "[2/3] Building..."
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" -j$(nproc 2>/dev/null || echo 4)

# Copy to root
echo "[3/3] Installing to project root..."
if [ -f "$BUILD_DIR/velora" ]; then
    cp "$BUILD_DIR/velora" ./velora
    chmod +x ./velora
    echo ""
    echo " Build successful!"
    echo " Run Velora with: ./velora run examples/hello.vel"
    echo ""
else
    echo "[WARN] Binary not found in $BUILD_DIR. Check build output."
fi
