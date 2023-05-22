# Makefile

# Compiler
CC = g++

# Compiler flags
CFLAGS = -Wall -Wextra -std=c++11

# Source directory
SRC_DIR = ./source

# Output directory
BUILD_DIR = ./build

# Find all C++ files recursively in the source directory
CPP_FILES := $(shell find $(SRC_DIR) -type f -name '*.cpp')

# Generate object file names by replacing the source directory with the build directory
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(CPP_FILES))

# Create directories for object files
$(shell mkdir -p $(dir $(OBJ_FILES)))

# Default target
all: $(BUILD_DIR)/bob

# Rule to create necessary directories
$(DIRS):
	mkdir -p $(patsubst $(SRC_DIR)/%, $(OUTPUT_DIR)/%, $@)

# Rule to compile object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to link object files into the final executable
$(BUILD_DIR)/bob: $(OBJ_FILES)
	$(CC) $(CFLAGS) $^ -o $@

# Clean build directory
clean:
	rm -rf $(BUILD_DIR)/*
