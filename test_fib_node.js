function fib_recursive(n) {
    if (n <= 1) return n;
    return fib_recursive(n - 1) + fib_recursive(n - 2);
}

function fib_iterative(n) {
    if (n <= 1) return n;
    let a = 0, b = 1;
    for (let i = 2; i <= n; i++) {
        [a, b] = [b, a + b];
    }
    return b;
}

console.log("Testing Node.js fib...");
const start = Date.now();
const result = fib_iterative(50);
const end = Date.now();
console.log(`fib_iterative(50) = ${result}`);
console.log(`Time: ${end - start}ms`);
console.log("Test completed!");
