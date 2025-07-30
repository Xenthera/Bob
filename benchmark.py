import time

# Test the time function
print("Testing time function:")

time1 = time.time()
print(f"Time 1: {time1}")

time2 = time.time()
print(f"Time 2: {time2}")

time3 = time.time()
print(f"Time 3: {time3}")

diff1 = time2 - time1
diff2 = time3 - time2

print(f"Difference 1-2: {diff1} seconds")
print(f"Difference 2-3: {diff2} seconds")

# Test with some work in between
start = time.time()
sum_val = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10
end = time.time()
duration = end - start

print(f"Work duration: {duration} seconds")
print(f"Sum: {sum_val}")

print("Time function analysis complete!") 