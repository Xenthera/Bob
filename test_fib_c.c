#include <stdio.h>
#include <time.h>

long long fib_iterative(int n) {
    if (n <= 1) return n;
    long long a = 0, b = 1;
    for (int i = 2; i <= n; i++) {
        long long temp = a + b;
        a = b;
        b = temp;
    }
    return b;
}

int main() {
    printf("Testing C fib...\n");
    clock_t start = clock();
    long long result = fib_iterative(50);
    clock_t end = clock();
    
    double time_spent = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    printf("fib_iterative(50) = %lld\n", result);
    printf("Time: %.2fms\n", time_spent);
    printf("Test completed!\n");
    return 0;
}
