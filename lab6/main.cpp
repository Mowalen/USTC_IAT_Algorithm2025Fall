#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iostream>
#include <iterator>
#include <iomanip>
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

struct Node {
    char ch{};
    long long freq{};
    std::shared_ptr<Node> left{};
    std::shared_ptr<Node> right{};

    bool is_leaf() const { return !left && !right; }
};

struct NodeCompare {
    bool operator()(const std::shared_ptr<Node> &a, const std::shared_ptr<Node> &b) const {
        if (a->freq == b->freq) {
            return a->ch > b->ch;  // provide deterministic tie‑break
        }
        return a->freq > b->freq;
    }
};

using CodeMap = std::unordered_map<char, std::string>;

std::string printable_char(char c) {
    // Wrap the character so that punctuation remains visible in the table.
    std::string out;
    out.push_back(c);
    return out;
}

int display_width(const std::string &s) {
    int width = 0;
    for (std::size_t i = 0; i < s.size();) {
        const unsigned char uc = static_cast<unsigned char>(s[i]);
        if (uc < 0x80) {
            width += 1;
            ++i;
        } else if ((uc >> 5) == 0x6 && i + 1 < s.size()) {  // 2-byte UTF-8
            width += 2;
            i += 2;
        } else if ((uc >> 4) == 0xE && i + 2 < s.size()) {  // 3-byte UTF-8
            width += 2;
            i += 3;
        } else if ((uc >> 3) == 0x1E && i + 3 < s.size()) {  // 4-byte UTF-8
            width += 2;
            i += 4;
        } else {
            // Fallback for malformed sequences: treat the byte as width 1.
            width += 1;
            ++i;
        }
    }
    return width;
}

std::string pad_field(const std::string &text, int width, bool left_align) {
    const int pad = std::max(0, width - display_width(text));
    if (left_align) {
        return text + std::string(pad, ' ');
    }
    return std::string(pad, ' ') + text;
}

void build_codes(const std::shared_ptr<Node> &node, const std::string &path, CodeMap &codes) {
    if (!node) {
        return;
    }
    if (node->is_leaf()) {
        codes[node->ch] = path.empty() ? "0" : path;  // single symbol fallback
        return;
    }
    build_codes(node->left, path + "0", codes);
    build_codes(node->right, path + "1", codes);
}

int main() {
    const std::string input_path = "orignal.txt";
    const std::string table_path = "table.txt";
    const std::string encoded_path = "encoded.txt";

    std::ifstream in(input_path, std::ios::binary);
    if (!in) {
        std::cerr << "无法打开输入文件: " << input_path << '\n';
        return 1;
    }
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    std::unordered_map<char, long long> freq;
    long long total_chars = 0;
    for (unsigned char uc : content) {
        if (std::isspace(static_cast<unsigned char>(uc))) {
            continue;  // 跳过空格、换行等
        }
        freq[static_cast<char>(uc)]++;
        ++total_chars;
    }

    if (freq.empty()) {
        std::cerr << "没有需要编码的字符（均为空白字符）。\n";
        return 1;
    }

    std::priority_queue<std::shared_ptr<Node>, std::vector<std::shared_ptr<Node>>, NodeCompare> pq;
    for (const auto &p : freq) {
        pq.push(std::make_shared<Node>(Node{p.first, p.second}));
    }

    while (pq.size() > 1) {
        auto a = pq.top();
        pq.pop();
        auto b = pq.top();
        pq.pop();
        auto parent = std::make_shared<Node>(Node{'\0', a->freq + b->freq, a, b});
        pq.push(parent);
    }

    auto root = pq.top();
    CodeMap codes;
    build_codes(root, "", codes);

    struct Row {
        char ch;
        long long freq;
        std::string code;
    };
    std::vector<Row> rows;
    rows.reserve(codes.size());
    for (const auto &p : codes) {
        rows.push_back(Row{p.first, freq[p.first], p.second});
    }
    std::sort(rows.begin(), rows.end(), [](const Row &a, const Row &b) {
        if (a.freq == b.freq) {
            return a.ch < b.ch;
        }
        return a.freq > b.freq;
    });

    const std::string header_char = "字符";
    const std::string header_freq = "出现频率";
    const std::string header_code = "编码";
    int char_width = display_width(header_char);
    int freq_width = display_width(header_freq);
    int code_width = display_width(header_code);
    for (const auto &row : rows) {
        const std::string display_char = printable_char(row.ch);
        char_width = std::max(char_width, display_width(display_char));
        freq_width = std::max(freq_width, static_cast<int>(std::to_string(row.freq).size()));
        code_width = std::max(code_width, display_width(row.code));
    }

    std::ofstream table_out(table_path, std::ios::binary);
    if (!table_out) {
        std::cerr << "无法写入表格文件: " << table_path << '\n';
        return 1;
    }
    auto write_row = [&](const std::string &ch, const std::string &freq, const std::string &code) {
        table_out << pad_field(ch, char_width, true) << ' ' << pad_field(freq, freq_width, false) << ' '
                  << pad_field(code, code_width, true) << '\n';
    };

    write_row(header_char, header_freq, header_code);
    for (const auto &row : rows) {
        const std::string display_char = printable_char(row.ch);
        write_row(display_char, std::to_string(row.freq), row.code);
    }
    table_out.close();

    std::ofstream encoded_out(encoded_path, std::ios::binary);
    if (!encoded_out) {
        std::cerr << "无法写入编码文件: " << encoded_path << '\n';
        return 1;
    }
    long long huffman_bits = 0;
    for (unsigned char uc : content) {
        if (std::isspace(static_cast<unsigned char>(uc))) {
            continue;
        }
        const auto &code = codes[static_cast<char>(uc)];
        encoded_out << code;
        huffman_bits += static_cast<long long>(code.size());
    }
    encoded_out.close();

    const std::size_t symbols = freq.size();
    const int fixed_bits_per_char = symbols <= 1 ? 1 : static_cast<int>(std::ceil(std::log2(static_cast<double>(symbols))));
    const long long fixed_bits = total_chars * fixed_bits_per_char;
    const double ratio = fixed_bits == 0 ? 0.0 : static_cast<double>(huffman_bits) / static_cast<double>(fixed_bits);

    std::cout << "有效字符数: " << total_chars << "，不同字符数: " << symbols << '\n';
    std::cout << "Huffman编码总长度: " << huffman_bits << " bit\n";
    std::cout << "定长编码长度: " << fixed_bits << " bit (每字符 " << fixed_bits_per_char << " bit)\n";
    std::cout << "压缩率: " << std::fixed << std::setprecision(2) << ratio * 100.0 << "%\n";
    std::cout << "编码表已写入: " << table_path << "，01序列已写入: " << encoded_path << '\n';

    return 0;
}
