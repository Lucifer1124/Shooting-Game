#!/bin/bash
echo "=== Simple Shooter Game Setup ==="
echo ""

# Check if in correct directory
if [ ! -f "shooter_fixed.c" ]; then
    echo "Error: shooter_fixed.c not found in current directory!"
    echo "Please run this script from P:\\C game\\"
    exit 1
fi

echo "1. Compiling game..."
gcc shooter_fixed.c -o shooter.exe -lSDL2 -lSDL2_ttf -lm

if [ $? -eq 0 ]; then
    echo "✓ Compilation successful!"
else
    echo "✗ Compilation failed!"
    exit 1
fi

echo ""
echo "2. Copying required files..."

# Copy font if it exists in Windows
if [ -f "/c/Windows/Fonts/arial.ttf" ]; then
    cp /c/Windows/Fonts/arial.ttf . 2>/dev/null && echo "✓ Copied arial.ttf"
else
    echo "⚠ Warning: Could not find arial.ttf"
    echo "   Creating placeholder font file..."
    touch arial.ttf
fi

# Copy DLLs
echo "3. Copying DLL files..."
for dll in SDL2.dll SDL2_ttf.dll libfreetype-6.dll libpng16-16.dll zlib1.dll; do
    if [ -f "/mingw64/bin/$dll" ]; then
        cp "/mingw64/bin/$dll" . 2>/dev/null && echo "✓ Copied $dll"
    fi
done

echo ""
echo "4. Starting game..."
echo "================================="
echo ""
./shooter.exe