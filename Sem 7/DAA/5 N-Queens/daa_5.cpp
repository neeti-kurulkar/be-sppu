#include <iostream>
#include <vector>
#include <chrono>
using namespace std;
using namespace std::chrono;

// Check if it's safe to place a queen at (row, col)
bool isSafe(const vector<int>& rows, const vector<int>& hills, const vector<int>& dales,
            int row, int col, int n) {
    int hill = row - col + n - 1;
    int dale = row + col;
    return !rows[row] && !hills[hill] && !dales[dale];
}

// Backtracking function to place queens column by column
bool solveNQueens(int col, int n, vector<int>& queens,
                  vector<int>& rows, vector<int>& hills, vector<int>& dales, int fixedCol) {
    if (col == n) return true; // All queens placed

    // Skip the column where the first queen is already fixed
    if (col == fixedCol)
        return solveNQueens(col + 1, n, queens, rows, hills, dales, fixedCol);

    for (int row = 0; row < n; row++) {
        if (isSafe(rows, hills, dales, row, col, n)) {
            // Place queen
            queens[col] = row;
            rows[row] = hills[row - col + n - 1] = dales[row + col] = 1;

            // Recursively try next column
            if (solveNQueens(col + 1, n, queens, rows, hills, dales, fixedCol))
                return true;

            // Backtrack
            queens[col] = -1;
            rows[row] = hills[row - col + n - 1] = dales[row + col] = 0;
        }
    }
    return false;
}

// Display board
void printBoard(const vector<int>& queens, int n) {
    for (int r = 0; r < n; r++) {
        for (int c = 0; c < n; c++)
            cout << (queens[c] == r ? "Q " : ". ");
        cout << "\n";
    }
}

int main() {
    int n, fixedRow, fixedCol;
    cout << "Enter N (board size): ";
    cin >> n;
    cout << "Enter row and column of the first queen (0-indexed): ";
    cin >> fixedRow >> fixedCol;

    // Input validation
    if (fixedRow < 0 || fixedRow >= n || fixedCol < 0 || fixedCol >= n) {
        cout << "Invalid position.\n";
        return 1;
    }

    // Initialize tracking structures
    vector<int> queens(n, -1);            // queens[col] = row
    vector<int> rows(n, 0);               // track rows
    vector<int> hills(2 * n - 1, 0);      // "/" diagonals
    vector<int> dales(2 * n - 1, 0);      // "\" diagonals

    // Place first queen
    queens[fixedCol] = fixedRow;
    rows[fixedRow] = hills[fixedRow - fixedCol + n - 1] = dales[fixedRow + fixedCol] = 1;

    auto start = high_resolution_clock::now();
    bool solved = solveNQueens(0, n, queens, rows, hills, dales, fixedCol);
    auto end = high_resolution_clock::now();

    double timeTaken = duration_cast<milliseconds>(end - start).count();
    size_t spaceUsed = sizeof(int) * (n * 4 + (2 * n - 1) * 2);

    cout << "\n--------------------------------------\n";
    cout << "           [ N-Queens Results ]        \n";
    cout << "--------------------------------------\n";

    if (solved)
        printBoard(queens, n);
    else
        cout << "No solution exists.\n";

    cout << "Execution Time  : " << timeTaken << " ms\n";
    cout << "Estimated Space : " << spaceUsed << " bytes\n";
    cout << "--------------------------------------\n";
    return 0;
}