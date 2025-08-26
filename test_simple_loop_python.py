import time

start = time.time()
sum_val = 0
for i in range(1000000):
    sum_val = sum_val + i
end = time.time()

print(f"Sum: {sum_val}")
print(f"Time: {(end - start) * 1000:.2f}ms")
print("Test completed!")
