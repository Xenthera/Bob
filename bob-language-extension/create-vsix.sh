#!/bin/bash

# Create a simple VSIX package without npm

echo "Creating Bob Language Extension VSIX package..."

# Read version from package.json
VERSION=$(grep '"version"' package.json | sed 's/.*"version": "\([^"]*\)".*/\1/')
echo "Building version: $VERSION"

# Store the original directory
ORIGINAL_DIR="$(pwd)"
# Create a temporary directory for the package
TEMP_DIR=$(mktemp -d)
PACKAGE_DIR="$TEMP_DIR/bob-language-$VERSION"

# Create the extension directory structure
mkdir -p "$TEMP_DIR/extension"

# Copy all extension files to the extension directory
cp package.json "$TEMP_DIR/extension/"
cp language-configuration.json "$TEMP_DIR/extension/"
cp -r syntaxes "$TEMP_DIR/extension/"
cp -r snippets "$TEMP_DIR/extension/"
cp -r themes "$TEMP_DIR/extension/"
cp -r out "$TEMP_DIR/extension/"
cp README.md "$TEMP_DIR/extension/"

# Create the VSIX file (simple zip with .vsix extension)
cd "$TEMP_DIR"
zip -r "bob-language-$VERSION.vsix" extension/

# Move to the extension directory
mv "bob-language-$VERSION.vsix" "$ORIGINAL_DIR/"

# Clean up
rm -rf "$TEMP_DIR"

echo "VSIX package created: bob-language-$VERSION.vsix"
echo ""
echo "To install in Cursor:"
echo "1. Open Cursor"
echo "2. Go to Extensions (Cmd+Shift+X)"
echo "3. Click the '...' menu and select 'Install from VSIX...'"
echo "4. Select the bob-language-$VERSION.vsix file"
echo ""
echo "Or simply restart Cursor - the extension should already be working!" 