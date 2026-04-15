#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cmath>
using namespace std;
using namespace std::chrono;

struct Item {
    int weight, value;
};

double fractionalKnapsack(int capacity, vector<Item>& items) {
    // Step 1: Calculate value-to-weight ratio for each item
    vector<pair<double, Item>> ratios;
    for (auto& item : items) {
        double ratio = (double)item.value / item.weight;
        ratios.push_back({ratio, item});
    }

    // Step 2: Sort items in descending order of ratio
    sort(ratios.begin(), ratios.end(),
         [](auto& a, auto& b) { return a.first > b.first; });

    // Step 3: Pick items greedily
    double totalValue = 0.0;
    for (auto& entry : ratios) {
        Item item = entry.second;

        if (capacity >= item.weight) {
            // Take full item
            capacity -= item.weight;
            totalValue += item.value;
        } else {
            // Take fractional part
            double fraction = (double)capacity / item.weight;
            totalValue += item.value * fraction;
            break;
        }
    }

    return totalValue;
}


int main() {
    int n, capacity;
    cout << "Enter number of items: ";
    cin >> n;
    cout << "Enter knapsack capacity: ";
    cin >> capacity;

    vector<Item> items(n);
    for (int i = 0; i < n; i++) {
        cout << "Enter weight and value for item " << i + 1 << ": ";
        cin >> items[i].weight >> items[i].value;
    }

    auto start = high_resolution_clock::now();
    double maxValue = fractionalKnapsack(capacity, items);
    auto end = high_resolution_clock::now();

    double duration = duration_cast<nanoseconds>(end - start).count();
    int spaceUsed = sizeof(Item) * n + sizeof(int) * 2 + sizeof(double);

    cout << "\n----------------------------------------\n";
    cout << "[ Fractional Knapsack Results ]\n";
    cout << "----------------------------------------\n";
    cout << "Maximum Value  : " << maxValue << endl;
    cout << "Execution Time : " << duration << " ns\n";
    cout << "Space Used     : " << spaceUsed << " bytes\n";
    cout << "----------------------------------------\n";
    return 0;
}
