#!/bin/bash

# Bob Memory Leak Test Runner
# Runs all leak tests in sequence with memory monitoring

echo "🧪 Bob Memory Leak Test Suite"
echo "============================"
echo ""

# Build first
echo "📦 Building Bob..."
ninja -C build-ninja
if [ $? -ne 0 ]; then
    echo "❌ Build failed"
    exit 1
fi
echo "✅ Build successful"
echo ""

# Function to run a test and show memory info
run_test() {
    local test_file=$1
    local test_name=$2
    
    echo "🔬 Running: $test_name"
    echo "File: $test_file"
    echo "Memory monitoring: Use Activity Monitor (macOS) or top/htop (Linux)"
    echo "Press Ctrl+C during test to abort, or follow prompts to continue"
    echo ""
    
    ./build-ninja/bin/bob "$test_file"
    
    echo ""
    echo "✅ Completed: $test_name"
    echo "----------------------------------------"
    echo ""
}

# Run all tests
run_test "leakTests/leaktest_functions.bob" "Function Memory Tests"
run_test "leakTests/leaktest_collections.bob" "Collection Memory Tests" 
run_test "leakTests/leaktest_mixed.bob" "Mixed Type Memory Tests"
run_test "leakTests/leaktest_loops.bob" "Loop Memory Tests"
run_test "leakTests/leaktest_builtin.bob" "Builtin Function Memory Tests"

echo "🎉 All memory leak tests completed!"
echo ""
echo "💡 Memory Monitoring Tips:"
echo "- Memory should spike during object creation"
echo "- Memory should drop after 'cleared' messages"
echo "- Memory should return close to baseline between tests"
echo "- Watch for gradual increases across test cycles (indicates leaks)"