import time

print("=== Python Big Number Comparison ===")

# Test 1: Large addition (should be fast)
start = time.time()
a = 1234567890123456789012345678901234567890
b = 9876543210987654321098765432109876543210
result = a + b
end = time.time()
print(f"Large addition result: {result}")
print(f"Time: {(end - start) * 1000:.2f}ms")

# Test 2: Large multiplication (should be slower)
start = time.time()
c = 123456789012345678901234567890
d = 987654321098765432109876543210
product = c * d
end = time.time()
print(f"Large multiplication result: {product}")
print(f"Time: {(end - start) * 1000:.2f}ms")

# Test 3: Iterative sum with big numbers
start = time.time()
sum_val = 0
for i in range(100):
    sum_val = sum_val + 123456789012345678901234567890
end = time.time()
print(f"Iterative sum result: {sum_val}")
print(f"Time: {(end - start) * 1000:.2f}ms")

print("=== Test Complete ===")
