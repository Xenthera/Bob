#include <iostream>
#include <chrono>

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
    std::cout << "Testing C++ fib..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    long long result = fib_iterative(50);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "fib_iterative(50) = " << result << std::endl;
    std::cout << "Time: " << duration.count() << "μs" << std::endl;
    std::cout << "Test completed!" << std::endl;
    return 0;
}
