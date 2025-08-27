```
██████╗  ██████╗ ██████╗ 
██╔══██╗██╔═══██╗██╔══██╗
██████╔╝██║   ██║██████╔╝
██╔══██╗██║   ██║██╔══██╗
██████╔╝╚██████╔╝██████╔╝
╚═════╝  ╚═════╝ ╚═════╝ 
```

A modern programming language with all the features/sytax I prefer

## Documentation

- **[Language Reference](Reference/BOB_LANGUAGE_REFERENCE.md)** - Language syntax and features
- **[Build Guide](Reference/BUILD.md)** - How to build Bob  
- **[Roadmap](Reference/ROADMAP.md)** - What's done and what might come next

## Features

- **Core Types**: Numbers (integer, float, BigInt), strings, booleans, arrays, dictionaries, functions
- **Advanced Functions**: First-class functions, closures, anonymous functions, tail call optimization
- **Control Flow**: If/else statements, while/do-while/for loops, break/continue
- **Operators**: Arithmetic, logical, bitwise (full BigInt support), comparison, compound assignment (+=, -=, etc.)
- **Built-in Functions**: print, input, assert, len, push, pop, keys, values, type conversion (toString, toNumber, toInt)
- **Other Stuff**: String interpolation, escape sequences, file I/O, eval, time/sleep, random
- **Memory**: Automatic cleanup, no manual memory management

## Quick Start

```bash
# Build
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Release
ninja -C build

# Run
./build/bin/bob your_file.bob
```