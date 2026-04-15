#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <chrono>
#include <cmath>
using namespace std;
using namespace std::chrono;

class Node {
public:
    char symbol;
    int freq;
    Node* left;
    Node* right;

    Node(char s, int f) : symbol(s), freq(f), left(nullptr), right(nullptr) {}
};

// Comparator for min-heap
struct Compare {
    bool operator()(Node* a, Node* b) {
        return a->freq > b->freq;
    }
};

// Recursive function to generate Huffman codes
void generateCodes(Node* root, string code, map<char, string>& codes) {
    if (!root) return;
    if (!root->left && !root->right) {
        codes[root->symbol] = code;
    }
    generateCodes(root->left, code + "0", codes);
    generateCodes(root->right, code + "1", codes);
}

int main() {
    int n;
    cout << "Enter number of characters: ";
    cin >> n;

    vector<char> chars(n);
    vector<int> freqs(n);

    cout << "\nEnter characters and their frequencies:\n";
    for (int i = 0; i < n; ++i) {
        cout << "Character " << i + 1 << ": ";
        cin >> chars[i];
        cout << "Frequency of " << chars[i] << ": ";
        cin >> freqs[i];
    }

    // Measure tree construction time
    auto start_tree = high_resolution_clock::now();

    priority_queue<Node*, vector<Node*>, Compare> pq;
    for (int i = 0; i < n; ++i) {
        pq.push(new Node(chars[i], freqs[i]));
    }

    while (pq.size() > 1) {
        Node* left = pq.top(); pq.pop();
        Node* right = pq.top(); pq.pop();

        Node* merged = new Node('\0', left->freq + right->freq);
        merged->left = left;
        merged->right = right;

        pq.push(merged);
    }

    Node* root = pq.top();
    auto end_tree = high_resolution_clock::now();

    // Measure code generation time
    auto start_code = high_resolution_clock::now();
    map<char, string> huffmanCodes;
    generateCodes(root, "", huffmanCodes);
    auto end_code = high_resolution_clock::now();

    // Measure space used
    auto start_space = high_resolution_clock::now();
    double totalBits = 0;
    for (size_t i = 0; i < n; ++i) {
        totalBits += freqs[i] * huffmanCodes[chars[i]].length();
    }
    double spaceUsed = ceil(totalBits / 8.0); // in bytes
    auto end_space = high_resolution_clock::now();

    // Compute durations in milliseconds
    auto treeTime = duration_cast<milliseconds>(end_tree - start_tree).count();
    auto codeTime = duration_cast<milliseconds>(end_code - start_code).count();
    auto spaceTime = duration_cast<milliseconds>(end_space - start_space).count();

    cout << "\n--------------------------------------------\n";
    cout << "        [ Huffman Coding Results ]          \n";
    cout << "--------------------------------------------\n";

    cout << "Tree Construction Time : " << treeTime << " ms\n";
    cout << "Code Generation Time   : " << codeTime << " ms\n";
    cout << "Space Calculation Time : " << spaceTime << " ms\n";
    cout << "Estimated Space Used   : " << spaceUsed << " bytes\n";

    cout << "\nHuffman Codes:\n";
    for (auto& kv : huffmanCodes) {
        cout << kv.first << " : " << kv.second << endl;
    }

    cout << "--------------------------------------------\n";
    return 0;
}
