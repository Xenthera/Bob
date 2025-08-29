#!/bin/bash

# Copy only the headers that BobModuleSDK.h actually includes
MODULE_DIR="${1:-cjson_module}"

if [ -z "$MODULE_DIR" ]; then
    echo "Usage: $0 <module_directory>"
    echo "Example: $0 cjson_module"
    exit 1
fi

# Create include directory
mkdir -p "$MODULE_DIR/include"

# Extract all the #include statements from BobModuleSDK.h
echo "Scanning BobModuleSDK.h for includes..."

# Get all the Bob includes (not system includes)
BOB_INCLUDES=$(grep '^#include "' src/headers/runtime/BobModuleSDK.h | sed 's/#include "//' | sed 's/"//')

echo "Found Bob includes:"
echo "$BOB_INCLUDES"

# Copy each included header
for include in $BOB_INCLUDES; do
    # Handle relative paths
    if [[ $include == *"/"* ]]; then
        # Create subdirectory if needed
        dir=$(dirname "$include")
        mkdir -p "$MODULE_DIR/include/$dir"
        # Try different possible locations
        if [[ -f "src/headers/$include" ]]; then
            cp "src/headers/$include" "$MODULE_DIR/include/$include"
            echo "Copied: $include"
        elif [[ -f "src/headers/common/$include" ]]; then
            cp "src/headers/common/$include" "$MODULE_DIR/include/$include"
            echo "Copied: $include"
        else
            echo "Warning: Could not find $include"
        fi
    else
        # Direct header - try runtime first, then other locations
        if [[ -f "src/headers/runtime/$include" ]]; then
            cp "src/headers/runtime/$include" "$MODULE_DIR/include/"
            echo "Copied: $include"
        elif [[ -f "src/headers/$include" ]]; then
            cp "src/headers/$include" "$MODULE_DIR/include/"
            echo "Copied: $include"
        else
            echo "Warning: Could not find $include"
        fi
    fi
done

# Copy the SDK header itself
cp src/headers/runtime/BobModuleSDK.h "$MODULE_DIR/include/"

echo ""
echo "Minimal SDK copied to $MODULE_DIR/include/"
echo "Only includes the headers actually needed by BobModuleSDK.h"
