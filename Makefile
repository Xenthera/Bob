# Makefile - Bob language interpreter

# Compiler
CC = g++

# Compiler flags  
CFLAGS = -Wall -Wextra -std=c++17 -Wno-unused-variable -Wno-unused-parameter -Wno-switch -O3 -march=native

# Source layout:
# src/
#   ├── headers/     - All header files (public interface)
#   │   ├── runtime/     - Runtime execution headers
#   │   ├── parsing/     - Front-end processing headers  
#   │   ├── stdlib/      - Standard library headers
#   │   ├── cli/         - CLI headers
#   │   └── common/      - Shared utilities (helperFunctions)
#   └── sources/     - All source files (implementation)
#       ├── runtime/     - Interpreter, Evaluator, Executor, Environment, Value, TypeWrapper
#       ├── parsing/     - Lexer, Parser, ErrorReporter, Expression AST
#       ├── stdlib/      - Built-in functions (BobStdLib)
#       └── cli/         - Command-line interface (main, bob)

SRC_ROOT = ./src
BUILD_DIR = ./build

# Find all .cpp files under src/sources (recursively)
CPP_FILES := $(shell find $(SRC_ROOT)/sources -type f -name '*.cpp')

# Map every source file to its corresponding object file inside build/
# Strip src/sources/ prefix and add build/ prefix
OBJ_FILES := $(patsubst $(SRC_ROOT)/sources/%.cpp,$(BUILD_DIR)/%.o,$(CPP_FILES))

# Make sure every object directory exists ahead of time
$(shell mkdir -p $(dir $(OBJ_FILES)))

# Default goal
all: build

# Pattern rule – compile each .cpp from sources/ into a mirrored .o inside build/
$(BUILD_DIR)/%.o: $(SRC_ROOT)/sources/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(SRC_ROOT)/headers/runtime -I$(SRC_ROOT)/headers/parsing -I$(SRC_ROOT)/headers/stdlib -I$(SRC_ROOT)/headers/cli -I$(SRC_ROOT)/headers/common -c $< -o $@

# Link all objects into the final executable
$(BUILD_DIR)/bob: $(OBJ_FILES)
	$(CC) $(CFLAGS) $^ -o $@

# Convenience targets
run:
	./$(BUILD_DIR)/bob

build: clean $(BUILD_DIR)/bob

clean:
	rm -rf $(BUILD_DIR)/*
