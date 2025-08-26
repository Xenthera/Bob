import time

print("=== Python Big Number Performance Test ===")

# Test 1: Large integer arithmetic
print("1. Large integer arithmetic...")
start = time.time()
huge1 = 1234567890123456789012345678901234567890
huge2 = 9876543210987654321098765432109876543210
result1 = huge1 + huge2
end = time.time()
print(f"Result: {result1}")
print(f"Time: {(end - start) * 1000:.2f}ms")

# Test 2: Multiplication of large numbers
print("2. Large number multiplication...")
start = time.time()
big1 = 123456789012345678901234567890
big2 = 987654321098765432109876543210
result2 = big1 * big2
end = time.time()
print(f"Result: {result2}")
print(f"Time: {(end - start) * 1000:.2f}ms")

# Test 3: Iterative calculation with big numbers
print("3. Iterative calculation with big numbers...")
start = time.time()
sum_val = 0
for i in range(1000):
    sum_val = sum_val + 123456789012345678901234567890
end = time.time()
print(f"Sum: {sum_val}")
print(f"Time: {(end - start) * 1000:.2f}ms")

# Test 4: Power calculation
print("4. Power calculation...")
start = time.time()
base = 2
power = 1000
result3 = 1
for i in range(power):
    result3 = result3 * base
end = time.time()
print(f"2^1000: {result3}")
print(f"Time: {(end - start) * 1000:.2f}ms")

print("=== Python Big Number Test Complete ===")
