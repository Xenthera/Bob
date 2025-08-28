# Bob Language Extension v0.6.1

## What's New

### ✨ New Features Added

#### **Foreach Loops**
- **foreach loops**: `foreach (item in collection)` for arrays, dictionaries, and strings
- **Range-based iteration**: `foreach (i in range(0, 10))` for Python-style iteration
- **Enhanced loop control**: Works with break and continue statements

#### **Big Number Support (GMP Integration)**
- **BigInt literals**: `123456789n` for arbitrary-precision integers
- **Automatic promotion**: Large numbers automatically promote to BigInt
- **Bitwise operations**: Full support for BigInt bitwise operations
- **Cross-type arithmetic**: Seamless mixing of regular integers and BigInt

#### **Enhanced Error Reporting**
- **Accurate line/column reporting**: AST-based error location extraction
- **Module context preservation**: Errors in imported modules show correct source
- **Source context display**: Shows surrounding code for better debugging
- **File name titles**: Clear indication of which file contains the error

#### **New Array Methods**
- **contains()**: Check if array contains a value
- **slice()**: Extract subarray from start to end
- **reverse()**: Reverse array in place
- **Array concatenation**: `array1 + array2` for combining arrays

#### **Enhanced Built-in Functions**
- **range()**: Python-style range function for iteration
- **dir()**: Get object members for introspection
- **functions()**: Get object functions for introspection
- **values()**: Get dictionary values (alongside existing keys() and has())

#### **Math Module Enhancements**
- **Math constants**: `math.PI`, `math.E`, `math.MAX_INT`
- **Enhanced mathematical operations**: Better precision and range

#### **Module System Improvements**
- **rand module renamed to random**: More intuitive naming
- **Enhanced REPL**: Arrow key navigation, history, and line editing
- **Cross-platform terminal input**: Better input handling across platforms

#### **Try-Catch Error Handling**
- **try-catch blocks**: `try { ... } catch (error) { ... }`
- **throw statements**: `throw "error message"`
- **finally blocks**: Optional cleanup code

#### **Dictionary Support**
- **Dictionary literals**: `{"key": "value", "number": 42}`
- **Dictionary indexing**: `dict{"key"}` (returns `none` for missing keys)
- **Dictionary assignment**: `dict{"key"} = value`
- **Nested dictionaries**: `{"user": {"name": "Bob", "age": 30}}`
- **Mixed type values**: Any type can be stored as dictionary values

#### **Dictionary Built-in Functions**
- `keys(dict)` - Returns array of all keys
- `values(dict)` - Returns array of all values
- `has(dict, key)` - Returns boolean if key exists

#### **Dictionary Code Snippets**
- `dict` - Create dictionary literal
- `dictaccess` - Access dictionary value
- `dictassign` - Assign to dictionary key
- `keys`, `values`, `has` - Built-in function snippets

#### **New Code Snippets**
- `foreach` - Foreach loop for collections
- `range` - Range-based iteration
- `contains` - Check if array contains value
- `slice` - Slice array from start to end
- `reverse` - Reverse array in place
- `concat` - Concatenate arrays
- `try` - Try-catch block
- `throw` - Throw error statement
- `bigint` - BigInt literal
- `import` - Import module
- `importfrom` - Import specific items from module
- `mathconst` - Math constants
- `dir` - Get object members
- `functions` - Get object functions

### 🎨 Syntax Highlighting Improvements
- **Method call syntax highlighting** - `object.method()` patterns are now properly highlighted
- **foreach loop** syntax highlighting
- **try-catch blocks** syntax support
- **BigInt literals** (numbers ending with 'n')
- **New built-in functions**: range, contains, slice, reverse, dir, functions
- **Enhanced keywords**: try, catch, finally, throw, foreach
- Dictionary literal syntax highlighting
- Dictionary indexing syntax support
- Built-in function highlighting for `keys`, `values`, `has`

### 📝 Documentation Updates
- Complete dictionary documentation with examples
- Dictionary built-in functions documentation
- Updated language reference with dictionary section
- Array and dictionary built-in functions documentation

---

## Previous Version (v0.3.0)

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

The extension will be packaged as `bob-language-0.4.0.vsix`

## Compatibility
- VS Code 1.60.0+
- Cursor (VS Code compatible)
- All platforms (Windows, macOS, Linux) 