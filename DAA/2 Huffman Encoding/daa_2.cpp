#include <iostream>
#include <vector>
#include <queue>
#include <chrono>
#include <map>
#include <cmath>
#include <iomanip>
#include <algorithm>
using namespace std;

class Node {
public:
    int freq;
    char symbol;
    Node* left;
    Node* right;
    char huff;

    Node(int freq, char symbol, Node* left = nullptr, Node* right = nullptr)
        : freq(freq), symbol(symbol), left(left), right(right), huff(0) {}

    bool operator<(const Node& other) const {
        return freq > other.freq; // For min-heap behavior in priority_queue
    }
};

void calculateHuffmanCodes(const Node* node, const string& code, map<char, string>& huffmanCodes) {
    if (node) {
        if (!node->left && !node->right) {
            huffmanCodes[node->symbol] = code;
        }
        calculateHuffmanCodes(node->left, code + "0", huffmanCodes);
        calculateHuffmanCodes(node->right, code + "1", huffmanCodes);
    }
}

int main() {
    int n;
    cout << "Enter the number of characters: ";
    cin >> n;

    vector<char> chars(n);
    vector<int> freq(n);

    cout << "\nEnter characters and their frequencies:\n";
    for (int i = 0; i < n; ++i) {
        cout << "Character " << i + 1 << ": ";
        cin >> chars[i];
        cout << "Frequency of " << chars[i] << ": ";
        cin >> freq[i];
    }

    priority_queue<Node> nodes;
    for (int i = 0; i < n; ++i) {
        nodes.push(Node(freq[i], chars[i]));
    }

    cout << "\n------------------------------------------\n";
    cout << "[ Huffman Tree Construction ]\n";

    auto start_time = chrono::high_resolution_clock::now();

    while (nodes.size() > 1) {
        Node* left = new Node(nodes.top());
        nodes.pop();
        Node* right = new Node(nodes.top());
        nodes.pop();

        left->huff = '0';
        right->huff = '1';

        Node* merged = new Node(left->freq + right->freq, left->symbol + right->symbol, left, right);
        nodes.push(*merged);
    }

    auto end_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::nanoseconds>(end_time - start_time);
    cout << "Time required             : " << duration.count() << " nanoseconds\n";

    cout << "\n[ Huffman Code Generation ]\n";

    auto code_start = chrono::high_resolution_clock::now();
    map<char, string> huffmanCodes;
    calculateHuffmanCodes(&nodes.top(), "", huffmanCodes);
    auto code_end = chrono::high_resolution_clock::now();

    auto code_duration = chrono::duration_cast<chrono::nanoseconds>(code_end - code_start);
    cout << "Time required             : " << code_duration.count() << " nanoseconds\n";

    cout << "\n[ Space Estimation ]\n";

    auto space_start = chrono::high_resolution_clock::now();
    double spaceUsed = 0;
    for (const auto& kv : huffmanCodes) {
        spaceUsed += kv.second.length() * freq[find(chars.begin(), chars.end(), kv.first) - chars.begin()];
    }
    spaceUsed = ceil(spaceUsed / 8); // Convert bits to bytes
    auto space_end = chrono::high_resolution_clock::now();

    auto space_duration = chrono::duration_cast<chrono::nanoseconds>(space_end - space_start);
    cout << "Estimated space used      : " << spaceUsed << " bytes\n";
    cout << "Space calculation time    : " << space_duration.count() << " nanoseconds\n";

    cout << "\n[ Huffman Codes ]\n";
    cout << left << setw(10) << "Char" << setw(10) << "Code" << endl;
    cout << "---------------------------\n";
    for (const auto& kv : huffmanCodes) {
        cout << left << setw(10) << kv.first << setw(10) << kv.second << endl;
    }

    cout << "------------------------------------------\n";
    return 0;
}