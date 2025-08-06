#!/bin/bash

# Bob Language Extension Installer for VS Code/Cursor

echo "Installing Bob Language Extension..."

# Get VS Code extensions directory
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    VSCODE_EXTENSIONS_DIR="$HOME/.vscode/extensions"
    CURSOR_EXTENSIONS_DIR="$HOME/.cursor/extensions"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux
    VSCODE_EXTENSIONS_DIR="$HOME/.vscode/extensions"
    CURSOR_EXTENSIONS_DIR="$HOME/.cursor/extensions"
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
    # Windows
    VSCODE_EXTENSIONS_DIR="$APPDATA/Code/User/extensions"
    CURSOR_EXTENSIONS_DIR="$APPDATA/Cursor/User/extensions"
else
    echo "Unsupported operating system: $OSTYPE"
    exit 1
fi

# Create extension directory
EXTENSION_NAME="bob-language-0.1.0"
EXTENSION_DIR="$VSCODE_EXTENSIONS_DIR/$EXTENSION_NAME"

echo "Installing to: $EXTENSION_DIR"

# Create directories
mkdir -p "$EXTENSION_DIR"

# Copy extension files
cp -r package.json "$EXTENSION_DIR/"
cp -r language-configuration.json "$EXTENSION_DIR/"
cp -r syntaxes "$EXTENSION_DIR/"
cp -r snippets "$EXTENSION_DIR/"
cp -r README.md "$EXTENSION_DIR/"

# Compile TypeScript if available
if command -v npm &> /dev/null; then
    echo "Compiling TypeScript..."
    npm install
    npm run compile
    cp -r out "$EXTENSION_DIR/"
else
    echo "npm not found, skipping TypeScript compilation"
fi

echo "Bob Language Extension installed successfully!"
echo ""
echo "To use the extension:"
echo "1. Restart VS Code/Cursor"
echo "2. Open a .bob file"
echo "3. Enjoy syntax highlighting and code snippets!"
echo ""
echo "Code snippets available:"
echo "- func: Function definition"
echo "- if: If statement"
echo "- while: While loop"
echo "- for: For loop"
echo "- var: Variable declaration"
echo "- print: Print statement"
echo "- assert: Assert statement"
echo "- And many more!" 