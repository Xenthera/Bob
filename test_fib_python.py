import time

def fib_recursive(n):
    if n <= 1:
        return n
    return fib_recursive(n - 1) + fib_recursive(n - 2)

def fib_iterative(n):
    if n <= 1:
        return n
    a, b = 0, 1
    for i in range(2, n + 1):
        a, b = b, a + b
    return b

print("Testing Python fib...")
start = time.time()
result = fib_iterative(50)
end = time.time()
print(f"fib_iterative(50) = {result}")
print(f"Time: {(end - start) * 1000:.2f}ms")
print("Test completed!")
