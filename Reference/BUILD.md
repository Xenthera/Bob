# Bob Language - Build Guide

This guide describes how to build the Bob language interpreter using CMake + Ninja across different platforms.

## 🚀 Quick Start

### Prerequisites
- **CMake** 3.20 or later
- **Ninja** build system  
- **C++17** compatible compiler

### Platform-Specific Setup

#### 🍎 macOS
```bash
# Install via Homebrew
brew install cmake ninja

# Or via MacPorts
sudo port install cmake ninja
```

#### 🐧 Linux

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install cmake ninja-build build-essential
```

**RHEL/CentOS/Fedora:**
```bash
# RHEL/CentOS
sudo yum install cmake ninja-build gcc-c++

# Fedora
sudo dnf install cmake ninja-build gcc-c++
```

**Arch Linux:**
```bash
sudo pacman -S cmake ninja gcc
```

#### 🪟 Windows

**Option 1: Visual Studio (Recommended)**
- Install Visual Studio 2019+ with C++ workload
- Install CMake via Visual Studio Installer
- Install Ninja: `winget install Ninja-build.Ninja`

**Option 2: MSYS2/MinGW**
```bash
# In MSYS2 terminal
pacman -S mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja mingw-w64-x86_64-gcc
```

**Option 3: Chocolatey**
```powershell
# In Administrator PowerShell
choco install cmake ninja visualstudio2022buildtools
```

## 🔨 Build Commands

### Standard Build

**Release Build (Optimized):**
```bash
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Release
ninja -C build
```

**Debug Build (Development):**
```bash
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Debug
ninja -C build
```

### Platform-Specific Examples

#### macOS/Linux
```bash
# Configure and build
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Release
ninja -C build

# Run interpreter
./build/bin/bob

# Run test suite
./build/bin/bob test_bob_language.bob

# Run with custom script
./build/bin/bob your_script.bob
```

#### Windows (PowerShell/CMD)
```powershell
# Configure and build
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Release
ninja -C build

# Run interpreter
.\build\bin\bob.exe

# Run test suite
.\build\bin\bob.exe test_bob_language.bob

# Run with custom script
.\build\bin\bob.exe your_script.bob
```

#### Windows (MSYS2/Git Bash)
```bash
# Same as macOS/Linux
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Release
ninja -C build
./build/bin/bob.exe test_bob_language.bob
```

## 🧪 Testing

### Automated Testing

**All Platforms:**
```bash
# Build first
ninja -C build

# Run tests via CTest
cd build
ctest --output-on-failure

# Or run tests verbosely
ctest --verbose
```

**Windows PowerShell:**
```powershell
ninja -C build
cd build
ctest --output-on-failure
```

### Manual Testing

**Interactive Mode:**
```bash
# Unix-like systems
./build/bin/bob

# Windows
.\build\bin\bob.exe
```

**Script Execution:**
```bash
# Unix-like systems  
./build/bin/bob examples/hello.bob

# Windows
.\build\bin\bob.exe examples\hello.bob
```

## ⚡ Performance

**CMake + Ninja** provides fast, cross-platform builds with excellent incremental compilation.

## 🔧 Advanced Configuration

### Custom Install Location

**Unix-like:**
```bash
cmake -G Ninja -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/opt/bob
ninja -C build install
```

**Windows:**
```powershell
cmake -G Ninja -B build `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_INSTALL_PREFIX="C:\Program Files\Bob"
ninja -C build install
```

### Compiler Selection

**GCC:**
```bash
cmake -G Ninja -B build \
  -DCMAKE_CXX_COMPILER=g++ \
  -DCMAKE_BUILD_TYPE=Release
```

**Clang:**
```bash
cmake -G Ninja -B build \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_BUILD_TYPE=Release
```

**MSVC (Windows):**
```powershell
cmake -G Ninja -B build `
  -DCMAKE_CXX_COMPILER=cl `
  -DCMAKE_BUILD_TYPE=Release
```

## 🐛 Troubleshooting

### Common Issues

**CMake not found:**
- **macOS**: `brew install cmake`
- **Ubuntu**: `sudo apt install cmake`  
- **Windows**: Install via Visual Studio or winget

**Ninja not found:**
- **macOS**: `brew install ninja`
- **Ubuntu**: `sudo apt install ninja-build`
- **Windows**: `winget install Ninja-build.Ninja`

**Compiler errors:**
- Ensure C++17 compiler is installed
- **Linux**: `sudo apt install build-essential`
- **Windows**: Install Visual Studio Build Tools

**Permission denied (Windows):**
- Run PowerShell as Administrator for system-wide installs

### Build Cache Issues

**Clean build:**
```bash
# Remove build directory
rm -rf build          # Unix-like
rmdir /s build        # Windows CMD
Remove-Item -Recurse build  # PowerShell

# Reconfigure
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Release
ninja -C build
```

