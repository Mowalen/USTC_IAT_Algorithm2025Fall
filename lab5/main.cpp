#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

// 用于返回标准DP的结果结构
struct LcsResult {
    std::string sequence;
    int length;
};

/**
 * 方法 1: 标准二维动态规划
 * 对应实验要求 2 
 * 时间复杂度: O(m*n)
 * 空间复杂度: O(m*n)
 * 功能: 可以通过回溯 DP 表重建具体的 LCS 字符串
 */
LcsResult lcs_standard(const std::string &a, const std::string &b) {
    const size_t m = a.size();
    const size_t n = b.size();
    
    // dp[i][j] 存储 a的前i个字符 和 b的前j个字符 的LCS长度
    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1, 0));

    // 1. 填表过程
    for (size_t i = 1; i <= m; ++i) {
        for (size_t j = 1; j <= n; ++j) {
            if (a[i - 1] == b[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1] + 1;
            } else {
                dp[i][j] = std::max(dp[i - 1][j], dp[i][j - 1]);
            }
        }
    }

    // 2. 回溯过程 (Reconstruction) - 只有保留了完整表格才能轻松做到
    std::string seq;
    seq.reserve(dp[m][n]);
    size_t i = m, j = n;
    while (i > 0 && j > 0) {
        if (a[i - 1] == b[j - 1]) {
            seq.push_back(a[i - 1]); // 找到公共字符
            --i;
            --j;
        } else if (dp[i - 1][j] >= dp[i][j - 1]) {
            --i; // 向上回退
        } else {
            --j; // 向左回退
        }
    }
    std::reverse(seq.begin(), seq.end()); // 因为是倒推的，所以要反转

    return {seq, dp[m][n]};
}

/**
 * 方法 2: 滚动数组优化 (两行)
 * 对应实验要求 3 
 * 时间复杂度: O(m*n)
 * 空间复杂度: O(2 * min(m, n))
 * 说明: 只能计算长度。若要存字符串，每个格子存string会导致空间爆炸。
 */
int lcs_length_two_rows(const std::string &a, const std::string &b) {
    // 确保 cols 是较短的那个，以保证空间是 O(min(m,n))
    const std::string &shorter = (a.size() < b.size()) ? a : b;
    const std::string &longer = (a.size() < b.size()) ? b : a;
    
    const size_t rows = longer.size();
    const size_t cols = shorter.size();

    // 只需要两行：prev(旧的一行), curr(当前计算的一行)
    std::vector<int> prev(cols + 1, 0), curr(cols + 1, 0);

    for (size_t i = 1; i <= rows; ++i) {
        for (size_t j = 1; j <= cols; ++j) {
            if (longer[i - 1] == shorter[j - 1]) {
                curr[j] = prev[j - 1] + 1;
            } else {
                curr[j] = std::max(prev[j], curr[j - 1]);
            }
        }
        // 滚动更新：当前行变旧行
        prev = curr; 
    }
    return prev[cols];
}

/**
 * 方法 3: 一维数组极致优化
 * 对应实验要求 4 
 * 时间复杂度: O(m*n)
 * 空间复杂度: O(min(m, n))
 * 说明: 利用变量 prev_diag 暂存左上角的值 dp[i-1][j-1]
 */
int lcs_length_one_row(const std::string &a, const std::string &b) {
    const std::string &shorter = (a.size() < b.size()) ? a : b;
    const std::string &longer = (a.size() < b.size()) ? b : a;
    
    const size_t rows = longer.size();
    const size_t cols = shorter.size();

    std::vector<int> dp(cols + 1, 0);

    for (size_t i = 1; i <= rows; ++i) {
        int prev_diag = 0; // 记录左上角的值 (相当于 dp[i-1][j-1])
        for (size_t j = 1; j <= cols; ++j) {
            int temp = dp[j]; // 在dp[j]被更新前，它是上一行的值 (即 dp[i-1][j])
            
            if (longer[i - 1] == shorter[j - 1]) {
                dp[j] = prev_diag + 1;
            } else {
                // dp[j] (旧值, 相当于上) vs dp[j-1] (新值, 相当于左)
                dp[j] = std::max(dp[j], dp[j - 1]);
            }
            
            prev_diag = temp; // 当前的 temp 将成为下一次循环的 "左上角"
        }
    }
    return dp[cols];
}

int main() {
    // 优化 I/O 速度
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    
    std::string text1, text2;
    if (!(std::cin >> text1 >> text2)) {
        return 0;
    }

    // 1. 标准方法 (输出序列和长度)
    LcsResult result = lcs_standard(text1, text2);
    
    // 2. 空间优化方法 (仅验证长度)
    int len_opt1 = lcs_length_two_rows(text1, text2);
    int len_opt2 = lcs_length_one_row(text1, text2);

    std::cout << "\n---------------- 实验结果 ----------------\n";
    
    // 按实验示例格式输出 [cite: 24, 27]
    if (result.length == 0) {
        std::cout << "LCS: \"\", 长度: 0" << "\n";
    } else {
        std::cout << "LCS: \"" << result.sequence << "\", 长度: " << result.length << "\n";
    }

    std::cout << "---------------- 算法验证 ----------------\n";
    std::cout << "标准DP (O(mn) Space) 长度: " << result.length << "\n";
    std::cout << "两行DP (O(2n) Space) 长度: " << len_opt1 << "\n";
    std::cout << "单行DP (O(n)  Space) 长度: " << len_opt2 << "\n";

    if (result.length == len_opt1 && result.length == len_opt2) {
        std::cout << ">> 所有算法结果一致，验证通过。\n";
    } else {
        std::cout << ">> 警告：算法结果不一致！\n";
    }

    return 0;
}