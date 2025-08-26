import time

print("=== Precise Python Big Number Timing ===")

# Warm up
for _ in range(1000):
    _ = 123456789012345678901234567890 + 987654321098765432109876543210

# Test 1: Large addition with precise timing
a = 1234567890123456789012345678901234567890
b = 9876543210987654321098765432109876543210

# Multiple runs to get accurate timing
times = []
for _ in range(1000):
    start = time.perf_counter()
    result = a + b
    end = time.perf_counter()
    times.append((end - start) * 1000000)  # Convert to microseconds

avg_time = sum(times) / len(times)
print(f"Large addition average time: {avg_time:.2f} microseconds")

# Test 2: Large multiplication with precise timing
c = 123456789012345678901234567890
d = 987654321098765432109876543210

times = []
for _ in range(100):  # Fewer runs for multiplication (slower)
    start = time.perf_counter()
    product = c * d
    end = time.perf_counter()
    times.append((end - start) * 1000000)

avg_time = sum(times) / len(times)
print(f"Large multiplication average time: {avg_time:.2f} microseconds")

print("=== Test Complete ===")
