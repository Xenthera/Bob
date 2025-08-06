#!/bin/bash

# Package Bob Language Extension as VSIX

echo "Packaging Bob Language Extension..."

# Check if vsce is installed
if ! command -v vsce &> /dev/null; then
    echo "Installing vsce..."
    npm install -g @vscode/vsce
fi

# Install dependencies
npm install

# Compile TypeScript
npm run compile

# Package the extension
vsce package

echo "Extension packaged successfully!"
echo "You can now install the .vsix file in VS Code/Cursor:"
echo "1. Open VS Code/Cursor"
echo "2. Go to Extensions (Ctrl+Shift+X)"
echo "3. Click the '...' menu and select 'Install from VSIX...'"
echo "4. Select the generated .vsix file" 