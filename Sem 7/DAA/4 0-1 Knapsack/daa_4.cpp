#include <iostream>
#include <vector>
#include <chrono>
using namespace std;
using namespace std::chrono;

struct Item {
    int value, weight;
};

int knapsackDP(int capacity, const vector<Item>& items) {
    int n = items.size();
    vector<vector<int>> dp(n + 1, vector<int>(capacity + 1, 0));

    for (int i = 1; i <= n; i++) {
        for (int w = 0; w <= capacity; w++) {
            if (items[i - 1].weight <= w)
                dp[i][w] = max(dp[i - 1][w],
                               items[i - 1].value + dp[i - 1][w - items[i - 1].weight]);
            else
                dp[i][w] = dp[i - 1][w];
        }
    }

    return dp[n][capacity];
}

int main() {
    int n, capacity;
    cout << "Enter number of items: ";
    cin >> n;

    vector<Item> items(n);
    for (int i = 0; i < n; i++) {
        cout << "Enter value and weight for item " << i + 1 << ": ";
        cin >> items[i].value >> items[i].weight;
    }

    cout << "Enter knapsack capacity: ";
    cin >> capacity;

    // Measure time
    auto start = high_resolution_clock::now();
    int maxValue = knapsackDP(capacity, items);
    auto end = high_resolution_clock::now();

    // Compute time and space
    double duration = duration_cast<nanoseconds>(end - start).count();
    size_t spaceUsed = sizeof(int) * (items.size() + 1) * (capacity + 1)
                     + sizeof(Item) * items.size();

    cout << "\n-----------------------------------------\n";
    cout << "          [ 0-1 Knapsack Results ]       \n";
    cout << "-----------------------------------------\n";
    cout << "Maximum Value   : " << maxValue << endl;
    cout << "Execution Time  : " << duration << " ns\n";
    cout << "Estimated Space : " << spaceUsed << " bytes\n";
    cout << "-----------------------------------------\n";

    return 0;
}
