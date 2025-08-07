# Bob Language Extension v0.3.0

## What's New

### ✨ New Features Added

#### **Enhanced Array Support**
- **Auto-truncating float indices**: `array[3.14]` → `array[3]` (like JavaScript/Lua)
- **Increment/decrement on array elements**: `array[0]++`, `++array[1]`
- **Improved array operations**: Better error handling and bounds checking

#### **New Built-in Functions**
- `toInt()` - convert floats to integers (truncates decimals)
- Enhanced error reporting for all built-in functions

#### **Increment/Decrement Operators**
- **Prefix increment**: `++x`
- **Postfix increment**: `x++`
- **Prefix decrement**: `--x`
- **Postfix decrement**: `x--`
- **Works on variables and array elements**

#### **Cross-Type Comparisons**
- **Equality operators** (`==`, `!=`) work with any types
- **Comparison operators** (`>`, `<`, `>=`, `<=`) only work with numbers
- **Clear error messages** for type mismatches

#### **Compound Assignment Operators**
- **Enhanced error reporting** with correct operator names
- **Consistent behavior** across all compound operators
- **Better type checking** before operations

#### **New Code Snippets**
- `toint` - Convert float to integer
- `compound` - Compound assignment operators
- `inc` - Increment variable
- `dec` - Decrement variable
- `arrayinc` - Increment array element
- `arraydec` - Decrement array element
- `crosscomp` - Cross-type comparison
- `floatindex` - Array access with float index

### 🎨 Syntax Highlighting Improvements
- Support for `toInt()` built-in function
- Enhanced operator recognition for increment/decrement
- Better array indexing syntax support

### 📝 Documentation Updates
- Comprehensive array documentation with auto-truncation examples
- New built-in function documentation (`toInt`, enhanced error reporting)
- Cross-type comparison behavior documentation
- Increment/decrement operator documentation
- Compound assignment operator documentation

### 🐛 Bug Fixes
- **Fixed array printing** - arrays no longer show as "unknown"
- **Enhanced error reporting** - all errors now use the error reporter system
- **Improved type checking** - better error messages for type mismatches
- **Memory management** - better cleanup of unused functions and arrays

## Installation

To create the VSIX package:
1. Install Node.js and npm
2. Run `npm install -g vsce`
3. Run `./package-vsix.sh`

The extension will be packaged as `bob-language-0.3.0.vsix`

## Compatibility
- VS Code 1.60.0+
- Cursor (VS Code compatible)
- All platforms (Windows, macOS, Linux) 