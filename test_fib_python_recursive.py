import time

def fib_recursive(n):
    if n <= 1:
        return n
    return fib_recursive(n - 1) + fib_recursive(n - 2)

print("Testing Python recursive fib...")
start = time.time()
result = fib_recursive(35)  # Much smaller to avoid stack overflow
end = time.time()
print(f"fib_recursive(35) = {result}")
print(f"Time: {(end - start) * 1000:.2f}ms")
print("Test completed!")
