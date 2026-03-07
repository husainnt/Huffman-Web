#include <bits/stdc++.h>
using namespace std;

class TreeNode
{
public:
    char data;
    int freq;
    TreeNode *left, *right;

    TreeNode(char d, int f)
    {
        data = d;
        freq = f;
        left = right = nullptr;
    }
};

class Compare
{
public:
    bool operator()(TreeNode *a, TreeNode *b)
    {
        return a->freq > b->freq;
    }
};

class Huffman
{

public:
    unordered_map<char, string> encodeMap;
    unordered_map<string, char> decodeMap;

    void generateCodes(TreeNode *root, string code);
    void Encode(string inputFile, string outputFile);
    void Decode(string inputFile, string outputFile);
};

void Huffman::generateCodes(TreeNode *root, string code)
{
    if (!root)
        return;

    if (!root->left && !root->right)
    {
        encodeMap[root->data] = code;
        decodeMap[code] = root->data;
    }

    generateCodes(root->left, code + "0");
    generateCodes(root->right, code + "1");
}

void Huffman::Encode(string inputFile, string outputFile)
{
    encodeMap.clear();
    decodeMap.clear();

    ifstream input(inputFile, ios::binary);
    ofstream output(outputFile, ios::binary);

    if (!input || !output)
    {
        cout << "{\"status\":\"error\",\"message\":\"file error\"}";
        return;
    }

    input.seekg(0, ios::end);
    int originalSize = input.tellg();
    input.seekg(0);

    unordered_map<char, int> freq;
    char ch;

    while (input.get(ch))
        freq[ch]++;

    input.clear();
    input.seekg(0);

    priority_queue<TreeNode *, vector<TreeNode *>, Compare> pq;

    for (auto &p : freq)
        pq.push(new TreeNode(p.first, p.second));

    while (pq.size() > 1)
    {
        TreeNode *left = pq.top();
        pq.pop();
        TreeNode *right = pq.top();
        pq.pop();

        TreeNode *parent = new TreeNode('\0', left->freq + right->freq);
        parent->left = left;
        parent->right = right;

        pq.push(parent);
    }

    TreeNode *root = pq.top();

    generateCodes(root, "");

    int mapSize = encodeMap.size();
    output.write((char *)&mapSize, sizeof(int));

    for (auto &p : encodeMap)
    {
        output.write(&p.first, sizeof(char));

        int len = p.second.size();
        output.write((char *)&len, sizeof(int));

        output.write(p.second.c_str(), len);
    }

    string encoded = "";
    while (input.get(ch))
        encoded += encodeMap[ch];

    int extraBits = (8 - encoded.size() % 8) % 8;

    for (int i = 0; i < extraBits; i++)
        encoded += '0';

    output.write((char *)&extraBits, sizeof(int));

    unsigned char buffer = 0;
    int count = 0;

    for (char bit : encoded)
    {
        buffer <<= 1;

        if (bit == '1')
            buffer |= 1;

        count++;

        if (count == 8)
        {
            output.write((char *)&buffer, 1);
            buffer = 0;
            count = 0;
        }
    }

    input.close();
    output.close();

    ifstream compressed(outputFile, ios::binary);
    compressed.seekg(0, ios::end);
    int compressedSize = compressed.tellg();
    compressed.close();

    double ratio = (double)compressedSize / originalSize;
    double percent = (1 - ratio) * 100;

    cout << "{";
    cout << "\"status\":\"compressed\",";
    cout << "\"original\":" << originalSize << ",";
    cout << "\"compressed\":" << compressedSize << ",";
    cout << "\"ratio\":" << ratio << ",";
    cout << "\"percentage\":" << percent;
    cout << "}";
}

void Huffman::Decode(string inputFile, string outputFile)
{
    decodeMap.clear();

    ifstream input(inputFile, ios::binary);
    ofstream output(outputFile, ios::binary);

    if (!input || !output)
    {
        cout << "{\"status\":\"error\",\"message\":\"file error\"}";
        return;
    }

    int mapSize;
    input.read((char *)&mapSize, sizeof(int));

    for (int i = 0; i < mapSize; i++)
    {
        char ch;
        int len;

        input.read(&ch, sizeof(char));
        input.read((char *)&len, sizeof(int));

        string code(len, ' ');
        input.read(&code[0], len);

        decodeMap[code] = ch;
    }

    int extraBits;
    input.read((char *)&extraBits, sizeof(int));

    string bits = "";
    char byte;

    while (input.read(&byte, 1))
        bits += bitset<8>((unsigned char)byte).to_string();

    bits = bits.substr(0, bits.size() - extraBits);

    string current = "";

    for (char b : bits)
    {
        current += b;

        if (decodeMap.count(current))
        {
            output.put(decodeMap[current]);
            current = "";
        }
    }

    input.close();
    output.close();

    cout << "{\"status\":\"decompressed\"}";
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        cout << "{\"status\":\"error\",\"message\":\"invalid arguments\"}";
        return 1;
    }

    string input = argv[1];
    string output = argv[2];
    string mode = argv[3];

    Huffman h;

    if (mode == "compress")
        h.Encode(input, output);
    else if (mode == "decompress")
        h.Decode(input, output);
    else
        cout << "{\"status\":\"error\",\"message\":\"invalid mode\"}";

    return 0;
}