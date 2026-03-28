#include <fstream>
#include <iostream>
#include <queue>
#include <string>

enum Color { RED, BLACK };

// 红黑树节点结构体，记录关键字、颜色与父子指针
struct Node {
    int key{};
    Color color{RED};
    Node *left{nullptr};
    Node *right{nullptr};
    Node *parent{nullptr};
};

// 红黑树封装，包含根指针、哨兵并实现插入与遍历辅助例程
struct RBTree {
    Node *root;
    Node *nil;

    RBTree() {
        // 构造时创建一个共享的 nil 哨兵节点
        nil = new Node{};
        nil->color = BLACK;
        nil->left = nil->right = nil->parent = nil;
        root = nil;
    }

    ~RBTree() {
        // 释放树中所有节点与哨兵
        destroy(root);
        delete nil;
    }

    // 插入新的关键字并记录修正过程
    void insert(int key, std::string &case_log) {
        Node *z = new Node{};
        z->key = key;
        z->color = RED;
        z->left = z->right = z->parent = nil;
        rbInsert(z, case_log);
    }

private:
    // 后序删除整棵树
    void destroy(Node *node) {
        if (node == nil) {
            return;
        }
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

    // 常规二叉搜索树插入，随后调用修复逻辑
    void rbInsert(Node *z, std::string &case_log) {
        Node *y = nil;
        Node *x = root;
        while (x != nil) {
            y = x;
            if (z->key < x->key) {
                x = x->left;
            } else {
                x = x->right;
            }
        }
        z->parent = y;
        if (y == nil) {
            root = z;
        } else if (z->key < y->key) {
            y->left = z;
        } else {
            y->right = z;
        }
        z->left = nil;
        z->right = nil;
        z->color = RED;
        rbInsertFixup(z, case_log);
    }

    // 按 CLRS 中的 6 种情况修复红黑树性质
    void rbInsertFixup(Node *z, std::string &case_log) {
        while (z->parent->color == RED) {
            if (z->parent == z->parent->parent->left) {
                Node *y = z->parent->parent->right;
                if (y->color == RED) {               // 情况1：叔节点为红
                    case_log.push_back('1');
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->right) {     // 情况2：叔黑且 z 在右侧
                        case_log.push_back('2');
                        z = z->parent;
                        leftRotate(z);
                    }
                    case_log.push_back('3');         // 情况3：叔黑且 z 在左侧
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    rightRotate(z->parent->parent);
                }
            } else {
                Node *y = z->parent->parent->left;
                if (y->color == RED) {               // 情况4：镜像，叔节点为红
                    case_log.push_back('4');
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->left) {      // 情况5：镜像，叔黑且 z 在左
                        case_log.push_back('5');
                        z = z->parent;
                        rightRotate(z);
                    }
                    case_log.push_back('6');         // 情况6：镜像，叔黑且 z 在右
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    leftRotate(z->parent->parent);
                }
            }
        }
        root->color = BLACK;
    }

    // 左旋：右孩子成为父节点
    void leftRotate(Node *x) {
        Node *y = x->right;
        x->right = y->left;
        if (y->left != nil) {
            y->left->parent = x;
        }
        y->parent = x->parent;
        if (x->parent == nil) {
            root = y;
        } else if (x == x->parent->left) {
            x->parent->left = y;
        } else {
            x->parent->right = y;
        }
        y->left = x;
        x->parent = y;
    }

    // 右旋：左孩子成为父节点
    void rightRotate(Node *x) {
        Node *y = x->left;
        x->left = y->right;
        if (y->right != nil) {
            y->right->parent = x;
        }
        y->parent = x->parent;
        if (x->parent == nil) {
            root = y;
        } else if (x == x->parent->right) {
            x->parent->right = y;
        } else {
            x->parent->left = y;
        }
        y->right = x;
        x->parent = y;
    }
};

// 将颜色枚举转成输出所需的字符串
static std::string colorToString(Color color) {
    return color == RED ? "red" : "black";
}

// 递归先序遍历：根-左-右
void preorder(Node *node, Node *nil, std::ofstream &out) {
    if (node == nil) {
        return;
    }
    out << node->key << " " << colorToString(node->color) << "\n";
    preorder(node->left, nil, out);
    preorder(node->right, nil, out);
}

// 递归中序遍历：左-根-右
void inorder(Node *node, Node *nil, std::ofstream &out) {
    if (node == nil) {
        return;
    }
    inorder(node->left, nil, out);
    out << node->key << " " << colorToString(node->color) << "\n";
    inorder(node->right, nil, out);
}

// 层序遍历：利用队列逐层访问
void levelOrder(Node *root, Node *nil, std::ofstream &out) {
    if (root == nil) {
        return;
    }
    std::queue<Node *> q;
    q.push(root);
    while (!q.empty()) {
        Node *node = q.front();
        q.pop();
        out << node->key << " " << colorToString(node->color) << "\n";
        if (node->left != nil) {
            q.push(node->left);
        }
        if (node->right != nil) {
            q.push(node->right);
        }
    }
}

int main() {
    // 打开输入文件
    std::ifstream input("insert.txt");
    if (!input) {
        std::cerr << "Failed to open insert.txt\n";
        return 1;
    }

    int n = 0;
    // 读取需要插入的关键字数量
    if (!(input >> n)) {
        std::cerr << "Failed to read number of keys\n";
        return 1;
    }

    RBTree tree;
    std::string case_log;
    // 顺序读入每个关键字并插入
    for (int i = 0; i < n; ++i) {
        int key = 0;
        if (!(input >> key)) {
            std::cerr << "Failed to read key " << i + 1 << "\n";
            return 1;
        }
        tree.insert(key, case_log);
    }

    if (!case_log.empty()) {
        std::string spaced;
        spaced.reserve(case_log.size() * 2);
        for (size_t i = 0; i < case_log.size(); ++i) {
            if (i != 0) {
                spaced.push_back(' ');
            }
            spaced.push_back(case_log[i]);
        }
        std::cout << "Fixup case sequence (CLRS cases 1-6, mirrored as 4/5/6): "
                  << spaced << "\n";
    } else {
        std::cout << "Fixup case sequence: <empty>\n";
    }

    std::ofstream nlr("NLR.txt");
    std::ofstream lnr("LNR.txt");
    std::ofstream lot("LOT.txt");
    if (!nlr || !lnr || !lot) {
        std::cerr << "Failed to open output files\n";
        return 1;
    }

    // 输出三种遍历结果
    preorder(tree.root, tree.nil, nlr);
    inorder(tree.root, tree.nil, lnr);
    levelOrder(tree.root, tree.nil, lot);
    std::cout << "Traversal output files generated: "
              << "NLR.txt (preorder), LNR.txt (inorder), LOT.txt (level-order)\n";

    return 0;
}
