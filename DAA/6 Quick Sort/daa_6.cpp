#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>
using namespace std;
using namespace std::chrono;

long long detComparisons = 0, randComparisons = 0;

// Swap helper
inline void swap(int &a, int &b) { int t = a; a = b; b = t; }

// Deterministic partition (pivot = last)
int detPartition(vector<int>& arr, int low, int high) {
    int pivot = arr[high], i = low - 1;
    for (int j = low; j < high; j++) {
        detComparisons++;
        if (arr[j] <= pivot) swap(arr[++i], arr[j]);
    }
    swap(arr[i + 1], arr[high]);
    return i + 1;
}

// Deterministic QuickSort
void detQuickSort(vector<int>& arr, int low, int high) {
    if (low < high) {
        int pi = detPartition(arr, low, high);
        detQuickSort(arr, low, pi - 1);
        detQuickSort(arr, pi + 1, high);
    }
}

// Randomized partition (random pivot)
int randPartition(vector<int>& arr, int low, int high) {
    int pivotIndex = low + rand() % (high - low + 1);
    swap(arr[pivotIndex], arr[high]);
    int pivot = arr[high], i = low - 1;
    for (int j = low; j < high; j++) {
        randComparisons++;
        if (arr[j] <= pivot) swap(arr[++i], arr[j]);
    }
    swap(arr[i + 1], arr[high]);
    return i + 1;
}

// Randomized QuickSort
void randQuickSort(vector<int>& arr, int low, int high) {
    if (low < high) {
        int pi = randPartition(arr, low, high);
        randQuickSort(arr, low, pi - 1);
        randQuickSort(arr, pi + 1, high);
    }
}

int main() {
    int n;
    cout << "Enter number of elements: ";
    cin >> n;

    vector<int> arr(n);
    cout << "Enter elements:\n";
    for (int i = 0; i < n; i++) cin >> arr[i];

    vector<int> detArr = arr, randArr = arr;
    srand(time(0));

    auto start = high_resolution_clock::now();
    detQuickSort(detArr, 0, n - 1);
    auto end = high_resolution_clock::now();
    double detTime = duration_cast<nanoseconds>(end - start).count();

    start = high_resolution_clock::now();
    randQuickSort(randArr, 0, n - 1);
    end = high_resolution_clock::now();
    double randTime = duration_cast<nanoseconds>(end - start).count();

    size_t spaceUsed = sizeof(int) * (arr.size() * 2) + sizeof(long long) * 2;

    cout << "\n-----------------------------------------\n";
    cout << "     [ QuickSort Performance Results ]   \n";
    cout << "-----------------------------------------\n";
    cout << "Deterministic QuickSort:\n";
    cout << "Comparisons : " << detComparisons << "\n";
    cout << "Time Taken  : " << detTime << " ns\n\n";
    cout << "Randomized QuickSort:\n";
    cout << "Comparisons : " << randComparisons << "\n";
    cout << "Time Taken  : " << randTime << " ns\n";
    cout << "\nEstimated Space : " << spaceUsed << " bytes\n";
    cout << "-----------------------------------------\n";

    return 0;
}