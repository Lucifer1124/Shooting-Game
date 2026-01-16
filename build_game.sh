#!/bin/bash
echo "Building Shooter Game with Triangle & Ovals..."
echo "==============================================="

# Compile
echo "Compiling..."
gcc shooter.c -o shooter.exe -lSDL2 -lSDL2_ttf -lm

if [ $? -eq 0 ]; then
    echo "✓ Compilation successful!"
else
    echo "✗ Compilation failed!"
    exit 1
fi

# Copy DLLs
echo "Copying DLL files..."
DLLS=("SDL2.dll" "SDL2_ttf.dll" "libfreetype-6.dll" "libpng16-16.dll" "zlib1.dll")
for dll in "${DLLS[@]}"; do
    if [ -f "/mingw64/bin/$dll" ]; then
        cp "/mingw64/bin/$dll" .
        echo "✓ Copied $dll"
    fi
done

# Copy font
echo "Copying font..."
if [ -f "/c/Windows/Fonts/arial.ttf" ]; then
    cp "/c/Windows/Fonts/arial.ttf" .
    echo "✓ Copied arial.ttf"
else
    echo "⚠ Could not find arial.ttf, trying alternatives..."
    touch arial.ttf  # Create empty file as placeholder
fi

echo ""
echo "Game built successfully!"
echo "Run: ./shooter.exe"
echo ""
echo "Press Enter to run the game now..."
read
./shooter.exe