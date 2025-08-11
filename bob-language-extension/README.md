# Bob Language Extension for VS Code

This extension provides syntax highlighting and language support for the Bob programming language in Visual Studio Code and Cursor.

## Features

- **Syntax Highlighting**: Full syntax highlighting for Bob language constructs
- **Code Snippets**: Useful code snippets for common Bob patterns
- **Auto-closing Brackets**: Automatic bracket and quote pairing
- **Indentation**: Smart indentation for Bob code blocks
- **Comments**: Support for line and block comments
- **Folding**: Code folding support with region markers

## Supported Syntax

### Keywords
- Control flow: `if`, `else`, `while`, `for`, `break`, `continue`, `return`
- Variable declaration: `var`
- Function declaration: `func`
- Classes and OOP: `class`, `extends`, `extension`, `this`, `super`
- Logical operators: `and`, `or`, `not`

### Built-in Functions
- `print()`, `assert()`, `input()`, `type()`, `toString()`, `toNumber()`, `toInt()`, `time()`, `sleep()`, `printRaw()`
- Arrays/Dictionaries (preferred method style): `arr.len()`, `arr.push(...)`, `arr.pop()`, `dict.len()`, `dict.keys()`, `dict.values()`, `dict.has()`
- Global forms still available: `len(x)`, `push(arr, ...)`, `pop(arr)`, `keys(dict)`, `values(dict)`, `has(dict, key)`
- Misc: `random()`, `eval()`

### Data Types
- Numbers (integers, floats, binary `0b1010`, hex `0xFF`)
- Strings (single and double quoted)
- Booleans (`true`, `false`)
- None value (`none`)
- Arrays (`[1, 2, 3]`)

### Operators
- Arithmetic: `+`, `-`, `*`, `/`, `%`
- Comparison: `==`, `!=`, `<`, `>`, `<=`, `>=`
- Logical: `&&`, `||`, `!`
- Bitwise: `&`, `|`, `^`, `<<`, `>>`, `~`
- Compound assignment: `+=`, `-=`, `*=`, `/=`, `%=`, `&=`, `|=`, `^=`, `<<=`, `>>=`
- Ternary: `condition ? valueIfTrue : valueIfFalse`
- String multiplication: `"hello" * 3`

## Installation

### From Source
1. Clone this repository
2. Run `npm install` to install dependencies
3. Run `npm run compile` to build the extension
4. Press `F5` in VS Code to launch the extension in a new window

### Manual Installation
1. Copy the extension files to your VS Code extensions directory
2. Restart VS Code
3. Open a `.bob` file to see syntax highlighting

## Usage

### Code Snippets
Type the following prefixes and press `Tab` to insert code snippets:

- `func` - Function definition
- `if` - If statement
- `ifelse` - If-else statement
- `while` - While loop
- `for` - For loop
- `var` - Variable declaration
- `print` - Print statement
- `assert` - Assert statement
- `anon` - Anonymous function
- `return` - Return statement
- `break` - Break statement
- `continue` - Continue statement
- `comment` - Comment block
- `test` - Test function
- `array` - Array declaration
- `arrayaccess` - Array access
- `arrayassign` - Array assignment
- `len` - Array length
- `push` - Array push
- `pop` - Array pop
- `random` - Random number
- `sleep` - Sleep function
- `printraw` - Print raw
- `eval` - Eval function

### File Association
Files with the `.bob` extension will automatically be recognized as Bob language files.

## Example

```go
// This is a comment
var message = "Hello, Bob!";
print(message);

// Array operations (method style)
var numbers = [1, 2, 3, 4, 5];
print("Array length: " + numbers.len());
numbers.push(6);
print("Popped: " + numbers.pop());
print("First element: " + numbers[0]);

// Function with ternary operator
func factorial(n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

var result = factorial(5);
var status = result > 100 ? "large" : "small";
assert(result == 120, "Factorial calculation failed");

// String multiplication
var repeated = "hello" * 3;  // "hellohellohello"
```

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This extension is licensed under the MIT License.

## Links

- [Bob Language Repository](https://github.com/bob-lang/bob)
- [VS Code Extension API](https://code.visualstudio.com/api) 