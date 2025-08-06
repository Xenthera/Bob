#!/bin/bash

echo "Attempting to reload Bob language extension..."

# Try to reload the extension using VS Code CLI if available
if command -v code &> /dev/null; then
    echo "Found VS Code CLI, attempting to reload window..."
    code --command "workbench.action.reloadWindow"
    echo "Reload command sent to VS Code/Cursor"
elif command -v cursor &> /dev/null; then
    echo "Found Cursor CLI, attempting to reload window..."
    cursor --command "workbench.action.reloadWindow"
    echo "Reload command sent to Cursor"
else
    echo "VS Code/Cursor CLI not found in PATH"
    echo ""
    echo "Manual reload required:"
    echo "1. Open Cursor"
    echo "2. Press Cmd+Shift+P (or Ctrl+Shift+P on Windows/Linux)"
    echo "3. Type 'Developer: Reload Window' and press Enter"
    echo ""
    echo "Or alternatively:"
    echo "1. Press Cmd+Shift+X to open Extensions"
    echo "2. Find 'Bob Language' extension"
    echo "3. Click the reload button (circular arrow icon)"
fi

echo ""
echo "Extension should now be reloaded with updated syntax highlighting!" 