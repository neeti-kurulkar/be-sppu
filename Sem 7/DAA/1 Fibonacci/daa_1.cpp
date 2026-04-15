#include <iostream>
#include <chrono>
using namespace std;
using namespace std::chrono;

// Iterative Fibonacci
int fib_iterative(int n) {
    if (n <= 1) return n;
    int a = 0, b = 1;
    for (int i = 2; i <= n; i++) {
        int c = a + b;
        a = b;
        b = c;
    }
    return b;
}

// Recursive Fibonacci
int fib_recursive(int n) {
    if (n <= 1) return n;
    return fib_recursive(n - 1) + fib_recursive(n - 2);
}

int main() {
    int n;
    cout << "Enter a number: ";
    cin >> n;

    // Iterative Fibonacci timing
    auto start_iter = high_resolution_clock::now();
    int result_iter = fib_iterative(n);
    auto end_iter = high_resolution_clock::now();

    auto time_iter_ms = duration_cast<milliseconds>(end_iter - start_iter).count();
    int space_iter = sizeof(int) * 3;  // constant space

    // Recursive Fibonacci timing
    auto start_rec = high_resolution_clock::now();
    int result_rec = fib_recursive(n);
    auto end_rec = high_resolution_clock::now();

    auto time_rec_ms = duration_cast<milliseconds>(end_rec - start_rec).count();
    int space_rec = sizeof(int) * n;   // roughly O(n) due to recursion depth

    // Output results
    cout << "\n------------------------------------------\n";
    cout << "          Fibonacci Comparison             \n";
    cout << "------------------------------------------\n";

    cout << "[ Iterative Approach ]\n";
    cout << "Fibonacci(" << n << ") = " << result_iter << endl;
    cout << "Time taken   : " << time_iter_ms << " milliseconds\n";
    cout << "Space used   : " << space_iter << " bytes\n";

    cout << "\n[ Recursive Approach ]\n";
    cout << "Fibonacci(" << n << ") = " << result_rec << endl;
    cout << "Time taken   : " << time_rec_ms << " milliseconds\n";
    cout << "Space used   : " << space_rec << " bytes\n";

    cout << "------------------------------------------\n";
    return 0;
}