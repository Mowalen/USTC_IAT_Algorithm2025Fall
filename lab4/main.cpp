#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

// 区间的闭区间表示，low <= high
struct Interval {
    int low;
    int high;
};

// 以红黑树为基础的区间树，每个节点记录自身区间及子树中最高端点
class IntervalTree {
public:
    IntervalTree() {
        // 以哨兵 nil_ 代替空指针，简化红黑树的旋转与修复逻辑
        nil_ = new Node{};
        nil_->color = Color::BLACK;
        nil_->left = nil_->right = nil_->parent = nil_;
        nil_->max_high = std::numeric_limits<int>::min();
        root_ = nil_;
    }

    ~IntervalTree() {
        destroy(root_);
        delete nil_;
    }

    void insert(const Interval &interval) {
        // 标准二叉搜索树插入，按区间左端点排序；相等时比较右端点
        Node *z = create_node(interval);
        Node *y = nil_;
        Node *x = root_;

        while (x != nil_) {
            y = x;
            if (is_less(interval, x->interval)) {
                x = x->left;
            } else {
                x = x->right;
            }
        }

        z->parent = y;
        if (y == nil_) {
            root_ = z;
        } else if (is_less(interval, y->interval)) {
            y->left = z;
        } else {
            y->right = z;
        }

        z->left = z->right = nil_;
        z->color = Color::RED;
        z->max_high = interval.high;


        // 红黑树性质修复，保证平衡
        insert_fixup(z);

        update_upwards(z);
    }

    std::vector<Interval> search_overlapping(const Interval &query) const {
        std::vector<Interval> result;
        search_overlapping(root_, query, result);
        return result;
    }

private:
    enum class Color { RED, BLACK };
    struct Node {
        Interval interval{};
        int max_high{};
        Color color{Color::BLACK};
        Node *left{nullptr};
        Node *right{nullptr};
        Node *parent{nullptr};
    };

    Node *root_{nullptr};
    Node *nil_{nullptr};

    static bool is_less(const Interval &a, const Interval &b) {
        if (a.low == b.low) {
            return a.high < b.high;
        }
        return a.low < b.low;
    }

    // 判断两个区间是否有重叠
    static bool is_overlapping(const Interval &a, const Interval &b) {
        return a.low <= b.high && b.low <= a.high;
    }

    Node *create_node(const Interval &interval) {
        Node *node = new Node;
        node->interval = interval;
        node->max_high = interval.high;
        node->color = Color::RED;
        node->left = node->right = node->parent = nil_;
        return node;
    }

    void destroy(Node *node) {
        if (node == nullptr || node == nil_) {
            return;
        }
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

    // 计算当前节点（含子树）最高的右端点
    int compute_max(Node *node) const {
        if (node == nil_) {
            return std::numeric_limits<int>::min();
        }
        return std::max(
            {node->interval.high, node->left->max_high, node->right->max_high});
    }

    // 更新单个节点的 max_high
    void update_node(Node *node) {
        if (node != nil_) {
            node->max_high = compute_max(node);
        }
    }

    // 从当前节点向上回溯，更新沿途的 max_high
    void update_upwards(Node *node) {
        while (node != nil_) {
            update_node(node);
            node = node->parent;
        }
    }

    // 左旋保持 BST 结构，同时更新旋转相关节点的 max_high
    void left_rotate(Node *x) {
        Node *y = x->right;
        x->right = y->left;
        if (y->left != nil_) {
            y->left->parent = x;
        }
        y->parent = x->parent;
        if (x->parent == nil_) {
            root_ = y;
        } else if (x == x->parent->left) {
            x->parent->left = y;
        } else {
            x->parent->right = y;
        }
        y->left = x;
        x->parent = y;

        update_node(x);
        update_node(y);
    }

    // 右旋
    void right_rotate(Node *y) {
        Node *x = y->left;
        y->left = x->right;
        if (x->right != nil_) {
            x->right->parent = y;
        }
        x->parent = y->parent;
        if (y->parent == nil_) {
            root_ = x;
        } else if (y == y->parent->left) {
            y->parent->left = x;
        } else {
            y->parent->right = x;
        }
        x->right = y;
        y->parent = x;

        update_node(y);
        update_node(x);
    }

    // 红黑树插入修复：通过重新着色与旋转恢复性质
    void insert_fixup(Node *z) {
        while (z->parent->color == Color::RED) {
            if (z->parent == z->parent->parent->left) {
                Node *y = z->parent->parent->right;
                if (y->color == Color::RED) {
                    z->parent->color = Color::BLACK;
                    y->color = Color::BLACK;
                    z->parent->parent->color = Color::RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->right) {
                        z = z->parent;
                        left_rotate(z);
                    }
                    z->parent->color = Color::BLACK;
                    z->parent->parent->color = Color::RED;
                    right_rotate(z->parent->parent);
                }
            } else {
                Node *y = z->parent->parent->left;
                if (y->color == Color::RED) {
                    z->parent->color = Color::BLACK;
                    y->color = Color::BLACK;
                    z->parent->parent->color = Color::RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->left) {
                        z = z->parent;
                        right_rotate(z);
                    }
                    z->parent->color = Color::BLACK;
                    z->parent->parent->color = Color::RED;
                    left_rotate(z->parent->parent);
                }
            }
        }
        root_->color = Color::BLACK;
    }

    // 利用 max_high 剪枝的重叠区间搜索
    void search_overlapping(Node *node, const Interval &query,
                            std::vector<Interval> &result) const {
        if (node == nil_) {
            return;
        }

        // 只有当左子树可能含有重叠（max_high >= query.low）时才递归左子树
        if (node->left != nil_ && node->left->max_high >= query.low) {
            search_overlapping(node->left, query, result);
        }

        if (is_overlapping(node->interval, query)) {
            result.push_back(node->interval);
        }

        // 若当前节点的左端点 <= 查询右端点，右子树仍可能存在重叠
        if (node->interval.low <= query.high) {
            search_overlapping(node->right, query, result);
        }
    }
};

bool read_intervals_from_file(const std::string &path,
                              std::vector<Interval> &intervals) {
    std::ifstream fin(path);
    if (!fin.is_open()) {
        std::cerr << "无法打开文件: " << path << '\n';
        return false;
    }

    int count = 0;
    if (!(fin >> count)) {
        std::cerr << "文件格式错误，未找到待插入数据个数。\n";
        return false;
    }
    intervals.reserve(count);
    for (int i = 0; i < count; ++i) {
        Interval interval{};
        if (!(fin >> interval.low >> interval.high)) {
            std::cerr << "读取第 " << (i + 1) << " 个区间失败。\n";
            return false;
        }
        if (interval.low > interval.high) {
            std::swap(interval.low, interval.high);
        }
        intervals.push_back(interval);
    }
    return true;
}

int main() {
    std::vector<Interval> intervals;
    if (!read_intervals_from_file("insert.txt", intervals)) {
        return 1;
    }

    IntervalTree tree;
    for (const auto &interval : intervals) {
        tree.insert(interval);
    }

    std::cout << "已从 insert.txt 插入 " << intervals.size()
              << " 个区间生成区间树。\n";
    std::cout << "请输入待查询区间（low high），例如：10 20\n> ";

    Interval query{};
    if (!(std::cin >> query.low >> query.high)) {
        std::cerr << "输入格式错误，期望两个整数。\n";
        return 1;
    }
    if (query.low > query.high) {
        std::swap(query.low, query.high);
    }

    const auto result = tree.search_overlapping(query);
    if (result.empty()) {
        std::cout << "未找到重叠区间。\n";
    } else {
        std::cout << "找到 " << result.size() << " 个重叠区间：\n";
        for (const auto &interval : result) {
            std::cout << "[" << interval.low << ", " << interval.high << "]\n";
        }
    }
    return 0;
}
