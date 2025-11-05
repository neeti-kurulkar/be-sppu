#include <iostream>
#include <vector>
#include <chrono>
using namespace std;
using namespace std::chrono;

// Check if it's safe to place a queen at (row, col)
bool isSafe(const vector<int>& rows, const vector<int>& hills, const vector<int>& dales,
            int row, int col, int n) {
    int hill = row - col + n - 1, dale = row + col;
    return !rows[row] && !hills[hill] && !dales[dale];
}

// Backtracking function
bool solve(int col, int n, vector<int>& queens,
           vector<int>& rows, vector<int>& hills, vector<int>& dales, int fixedCol) {
    if (col >= n) return true;
    if (col == fixedCol) return solve(col + 1, n, queens, rows, hills, dales, fixedCol);

    for (int row = 0; row < n; row++) {
        if (isSafe(rows, hills, dales, row, col, n)) {
            queens[col] = row;
            rows[row] = hills[row - col + n - 1] = dales[row + col] = 1;

            if (solve(col + 1, n, queens, rows, hills, dales, fixedCol))
                return true;

            // Backtrack
            queens[col] = -1;
            rows[row] = hills[row - col + n - 1] = dales[row + col] = 0;
        }
    }
    return false;
}

// Print the N-Queens board
void printBoard(const vector<int>& queens, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            cout << (queens[j] == i ? "Q " : ". ");
        cout << endl;
    }
}

int main() {
    int n, fixedRow, fixedCol;
    cout << "Enter value of N: ";
    cin >> n;
    cout << "Enter row and column of first queen (0-indexed): ";
    cin >> fixedRow >> fixedCol;

    if (fixedRow < 0 || fixedRow >= n || fixedCol < 0 || fixedCol >= n) {
        cout << "Invalid position.\n";
        return 1;
    }

    vector<int> queens(n, -1), rows(n, 0), hills(2 * n - 1, 0), dales(2 * n - 1, 0);
    queens[fixedCol] = fixedRow;
    rows[fixedRow] = hills[fixedRow - fixedCol + n - 1] = dales[fixedRow + fixedCol] = 1;

    auto start = high_resolution_clock::now();
    bool solved = solve(0, n, queens, rows, hills, dales, fixedCol);
    auto end = high_resolution_clock::now();

    double duration = duration_cast<nanoseconds>(end - start).count();
    size_t spaceUsed = sizeof(int) * (queens.size() * 4 + (2 * n - 1) * 2);

    cout << "\n-----------------------------------------\n";
    cout << "             [ N-Queens Results ]         \n";
    cout << "-----------------------------------------\n";
    if (solved) {
        cout << "Solution Found:\n";
        printBoard(queens, n);
    } else {
        cout << "No valid solution.\n";
    }
    cout << "Execution Time  : " << duration << " ns\n";
    cout << "Estimated Space : " << spaceUsed << " bytes\n";
    cout << "-----------------------------------------\n";

    return 0;
}