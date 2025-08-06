# Bob Language Extension Installation Guide

## Quick Installation

### Option 1: Automatic Installation (Recommended)
```bash
cd bob-language-extension
./install.sh
```

### Option 2: Manual Installation
1. Copy the extension files to your VS Code extensions directory:
   - **macOS**: `~/.vscode/extensions/bob-language-0.1.0/`
   - **Linux**: `~/.vscode/extensions/bob-language-0.1.0/`
   - **Windows**: `%APPDATA%\Code\User\extensions\bob-language-0.1.0\`

2. Restart VS Code/Cursor

### Option 3: VSIX Package
```bash
cd bob-language-extension
./package-vsix.sh
```
Then install the generated `.vsix` file in VS Code/Cursor.

## Features Included

### ✅ Syntax Highlighting
- Keywords: `if`, `else`, `while`, `for`, `break`, `continue`, `return`, `var`, `func`
- Built-in functions: `print`, `assert`, `input`, `type`, `toString`, `toNumber`, `time`
- Data types: numbers, strings, booleans, `none`
- Operators: arithmetic, comparison, logical, bitwise, compound assignment

### ✅ Code Snippets
Type these prefixes and press `Tab`:
- `func` - Function definition
- `if` - If statement
- `while` - While loop
- `for` - For loop
- `var` - Variable declaration
- `print` - Print statement
- `assert` - Assert statement
- `anon` - Anonymous function
- `test` - Test function

### ✅ Language Features
- Auto-closing brackets and quotes
- Smart indentation
- Comment support (`//` and `/* */`)
- Code folding
- Hover information for built-in functions
- IntelliSense completion

### ✅ Color Theme
- "Bob Dark" theme included
- Optimized colors for Bob syntax

## Testing the Extension

1. Open VS Code/Cursor
2. Open the `example.bob` file in the extension directory
3. You should see syntax highlighting, code completion, and snippets working

## File Association

Files with `.bob` extension will automatically be recognized as Bob language files.

## Troubleshooting

### Extension not working?
1. Check that the extension is installed in the correct directory
2. Restart VS Code/Cursor completely
3. Check the Extensions panel (Ctrl+Shift+X) to see if the extension is listed

### Syntax highlighting not working?
1. Make sure your file has a `.bob` extension
2. Check the language mode in the bottom-right corner of VS Code
3. Manually select "Bob" as the language mode if needed

### Snippets not working?
1. Type the snippet prefix (e.g., `func`)
2. Press `Ctrl+Space` to trigger suggestions
3. Select the snippet and press `Tab`

## Development

To modify the extension:
1. Edit the files in the extension directory
2. Run `npm run compile` to rebuild TypeScript
3. Reload VS Code/Cursor to see changes

## Support

For issues or questions:
1. Check the README.md file
2. Look at the example.bob file for syntax examples
3. Review the TextMate grammar in `syntaxes/bob.tmLanguage.json` 