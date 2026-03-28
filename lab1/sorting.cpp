#include "sorting.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <future>
#include <iostream>
#include <random>
#include <thread>
#include <tuple>

using namespace std;

// 交换函数
void swap(int& a, int& b) {
    int temp = a;
    a = b;
    b = temp;
}

// ========== 基础手写快速排序 ==========

int simplePartition(vector<int>& arr, int left, int right) {
    int pivot = arr[right];
    int i = left - 1;
    for (int j = left; j < right; ++j) {
        if (arr[j] <= pivot) {
            ++i;
            swap(arr[i], arr[j]);
        }
    }
    swap(arr[i + 1], arr[right]);
    return i + 1;
}

void simpleQuicksort(vector<int>& arr, int left, int right) {
    if (left >= right) {
        return;
    }
    int pivotIndex = simplePartition(arr, left, right);
    simpleQuicksort(arr, left, pivotIndex - 1);
    simpleQuicksort(arr, pivotIndex + 1, right);
}

void simpleQuicksort(vector<int>& arr) {
    if (!arr.empty()) {
        simpleQuicksort(arr, 0, static_cast<int>(arr.size()) - 1);
    }
}

// 插入排序（用于小数组）
void insertionSort(vector<int>& vals, int left, int right) {
    for (int i = left + 1; i <= right; i++) {
        int key = vals[i];
        int j = i - 1;
        
        while (j >= left && vals[j] > key) {
            vals[j + 1] = vals[j];
            j--;
        }
        vals[j + 1] = key;
    }
}

// 划分函数
int partition(vector<int>& vals, int left, int right, int pivotIndex) {
    // 将基准交换到最右边
    swap(vals[pivotIndex], vals[right]);
    
    int pivot = vals[right];
    int i = left - 1;
    
    for (int j = left; j < right; j++) {
        if (vals[j] <= pivot) {
            i++;
            swap(vals[i], vals[j]);
        }
    }
    swap(vals[i + 1], vals[right]);
    return i + 1;
}

// ========== 三种基准选择方式 ==========

// 1. 固定基准（选择最右边元素）
int choosePivotFixed(vector<int>& vals, int left, int right) {
    return right; // 总是选择最右边的元素
}

// 2. 随机基准
int choosePivotRandom(vector<int>& vals, int left, int right) {
    return left + rand() % (right - left + 1);
}

// 3. 三数取中
int choosePivotMedianOfThree(vector<int>& vals, int left, int right) {
    int mid = left + (right - left) / 2;
    
    // 对左、中、右三个元素进行排序，取中间值
    if (vals[left] > vals[mid])
        swap(vals[left], vals[mid]);
    if (vals[left] > vals[right])
        swap(vals[left], vals[right]);
    if (vals[mid] > vals[right])
        swap(vals[mid], vals[right]);
    
    return mid;
}

// 选择基准
inline int selectPivotIndex(vector<int>& vals, int left, int right, PivotStrategy strategy) {
    switch (strategy) {
        case FIXED:
            return choosePivotFixed(vals, left, right);
        case RANDOM:
            return choosePivotRandom(vals, left, right);
        case MEDIAN_OF_THREE:
        default:
            return choosePivotMedianOfThree(vals, left, right);
    }
}
 
// 快速排序递归函数（支持三种基准选择策略）
void quicksortHelper(vector<int>& vals, int left, int right, 
                    int threshold, PivotStrategy strategy) {
    // 如果子数组长度小于阈值，直接返回，留待最后用插入排序
    if (right - left + 1 <= threshold) {
        return;
    }
    
    // 根据策略选择基准
    int pivotIndex = selectPivotIndex(vals, left, right, strategy);
    
    // 划分
    int partitionIndex = partition(vals, left, right, pivotIndex);
    
    // 递归排序
    quicksortHelper(vals, left, partitionIndex - 1, threshold, strategy);
    quicksortHelper(vals, partitionIndex + 1, right, threshold, strategy);
}

tuple<int, int> threeWayPartition(vector<int>& vals, int left, int right, int pivotIndex);
void parallelQuicksortRange(vector<int>& vals, int left, int right, PivotStrategy strategy,
                            int threshold, int depth, int maxDepth);
void parallelQuicksort(vector<int>& vals, PivotStrategy strategy,
                       int threshold);

// 主快速排序函数
void optimizedQuicksort(vector<int>& vals, PivotStrategy strategy, 
                       int threshold) {
    if (vals.size() <= 1) return;
    
    threshold = max(threshold, 16);
    const int n = static_cast<int>(vals.size());
    
    struct Range {
        int left;
        int right;
    };
    
    vector<Range> stack;
    stack.reserve(64);
    stack.push_back({0, n - 1});
    
    while (!stack.empty()) {
        Range current = stack.back();
        stack.pop_back();
        int left = current.left;
        int right = current.right;
        
        while (right - left + 1 > threshold) {
            int pivotIndex = selectPivotIndex(vals, left, right, strategy);
            auto [lt, gt] = threeWayPartition(vals, left, right, pivotIndex);
            
            const int leftSize = lt - left;
            const int rightSize = right - gt;
            
            if (leftSize < rightSize) {
                if (gt + 1 < right) {
                    stack.push_back({gt + 1, right});
                }
                right = lt - 1;
            } else {
                if (left < lt - 1) {
                    stack.push_back({left, lt - 1});
                }
                left = gt + 1;
            }
        }
        
        if (left < right) {
            insertionSort(vals, left, right);
        }
    }
}

// ========== 三路划分（聚集优化）版本快速排序 ==========

// 三路划分函数
tuple<int, int> threeWayPartition(vector<int>& vals, int left, int right, int pivotIndex) {
    swap(vals[pivotIndex], vals[right]);
    int pivot = vals[right];
    
    int lt = left;      // 小于区的结束位置
    int gt = right;     // 大于区的开始位置
    int i = left;       // 当前处理位置
    
    while (i <= gt) {
        if (vals[i] < pivot) {
            swap(vals[lt], vals[i]);
            lt++;
            i++;
        } else if (vals[i] > pivot) {
            swap(vals[i], vals[gt]);
            gt--;
        } else {
            i++;
        }
    }
    
    return {lt, gt};
}


void quicksortWithGathering(vector<int>& vals, PivotStrategy strategy, 
                           int threshold) {
    if (vals.size() <= 1) return;
    
    if (vals.size() <= threshold) {
        insertionSort(vals, 0, vals.size() - 1);
        return;
    }
    
    struct Range {
        int left;
        int right;
    };
    
    vector<Range> stack;
    stack.reserve(64);
    stack.push_back({0, static_cast<int>(vals.size()) - 1});
    
    while (!stack.empty()) {
        Range current = stack.back();
        stack.pop_back();
        int left = current.left;
        int right = current.right;
        
        while (right - left + 1 > threshold) {
            int pivotIndex = selectPivotIndex(vals, left, right, strategy);
            auto [lt, gt] = threeWayPartition(vals, left, right, pivotIndex);
            
            const int leftSize = lt - left;
            const int rightSize = right - gt;
            
            if (leftSize < rightSize) {
                if (gt + 1 < right) {
                    stack.push_back({gt + 1, right});
                }
                right = lt - 1;
            } else {
                if (left < lt - 1) {
                    stack.push_back({left, lt - 1});
                }
                left = gt + 1;
            }
        }
        
        if (left < right) {
            insertionSort(vals, left, right);
        }
    }
    
    insertionSort(vals, 0, vals.size() - 1);
}

// ========== 多线程+优化快速排序 ==========


// 计算最大并行深度
int computeMaxParallelDepth() {
    unsigned concurrency = thread::hardware_concurrency();
    if (concurrency == 0) {
        concurrency = 4;
    }
    int depth = static_cast<int>(log2(static_cast<double>(concurrency)));
    return max(2, depth);
}

// 多线程+优化快速排序递归函数
void parallelQuicksortRange(vector<int>& vals, int left, int right, PivotStrategy strategy,
                            int threshold, int depth, int maxDepth) {
    if (left >= right) {
        return;
    }
    
    if (right - left + 1 <= threshold) {
        return;
    }
    
    int pivotIndex = selectPivotIndex(vals, left, right, strategy);
    auto [lt, gt] = threeWayPartition(vals, left, right, pivotIndex);
    
    const bool canSpawn = depth < maxDepth;
    const bool leftHasRange = left < lt - 1;
    const bool rightHasRange = gt + 1 < right;
    
    if (canSpawn && leftHasRange) {
        int subLeft = left;
        int subRight = lt - 1;
        auto leftFuture = std::async(launch::async, [&vals, subLeft, subRight, strategy, threshold, depth, maxDepth]() {
            parallelQuicksortRange(vals, subLeft, subRight, strategy, threshold, depth + 1, maxDepth);
        });
        
        if (rightHasRange) {
            parallelQuicksortRange(vals, gt + 1, right, strategy, threshold, depth + 1, maxDepth);
        }
        
        leftFuture.get();
    } else {
        if (leftHasRange) {
            parallelQuicksortRange(vals, left, lt - 1, strategy, threshold, depth + 1, maxDepth);
        }
        if (rightHasRange) {
            parallelQuicksortRange(vals, gt + 1, right, strategy, threshold, depth + 1, maxDepth);
        }
    }
}

// 多线程快速排序主函数
void parallelQuicksort(vector<int>& vals, PivotStrategy strategy, int threshold) {
    if (vals.size() <= 1) {
        return;
    }
    
    const int maxDepth = computeMaxParallelDepth();
    parallelQuicksortRange(vals, 0, vals.size() - 1, strategy, threshold, 0, maxDepth);
    
    
    insertionSort(vals, 0, vals.size() - 1);
}

// ========== 其他排序算法实现 ==========

void merge(vector<int>& vals, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    vector<int> leftPart(n1);
    vector<int> rightPart(n2);
    
    for (int i = 0; i < n1; ++i) leftPart[i] = vals[left + i];
    for (int j = 0; j < n2; ++j) rightPart[j] = vals[mid + 1 + j];
    
    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (leftPart[i] <= rightPart[j]) {
            vals[k++] = leftPart[i++];
        } else {
            vals[k++] = rightPart[j++];
        }
    }
    while (i < n1) {
        vals[k++] = leftPart[i++];
    }
    while (j < n2) {
        vals[k++] = rightPart[j++];
    }
}

void mergeSortHelper(vector<int>& vals, int left, int right) {
    if (left >= right) return;
    int mid = left + (right - left) / 2;
    mergeSortHelper(vals, left, mid);
    mergeSortHelper(vals, mid + 1, right);
    merge(vals, left, mid, right);
}

void mergeSort(vector<int>& vals) {
    if (vals.empty()) return;
    mergeSortHelper(vals, 0, static_cast<int>(vals.size()) - 1);
}

void heapify(vector<int>& vals, int n, int i) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;
    
    if (left < n && vals[left] > vals[largest]) largest = left;
    if (right < n && vals[right] > vals[largest]) largest = right;
    
    if (largest != i) {
        swap(vals[i], vals[largest]);
        heapify(vals, n, largest);
    }
}

void heapSort(vector<int>& vals) {
    int n = static_cast<int>(vals.size());
    if (n <= 1) return;
    
    for (int i = n / 2 - 1; i >= 0; --i) {
        heapify(vals, n, i);
    }
    for (int i = n - 1; i > 0; --i) {
        swap(vals[0], vals[i]);
        heapify(vals, i, 0);
    }
}

void bubbleSort(vector<int>& vals) {
    int n = static_cast<int>(vals.size());
    if (n <= 1) return;
    
    for (int i = 0; i < n - 1; ++i) {
        bool swapped = false;
        for (int j = 0; j < n - i - 1; ++j) {
            if (vals[j] > vals[j + 1]) {
                swap(vals[j], vals[j + 1]);
                swapped = true;
            }
        }
        if (!swapped) break;
    }
}

void selectionSort(vector<int>& vals) {
    int n = static_cast<int>(vals.size());
    if (n <= 1) return;
    
    for (int i = 0; i < n - 1; ++i) {
        int minIndex = i;
        for (int j = i + 1; j < n; ++j) {
            if (vals[j] < vals[minIndex]) {
                minIndex = j;
            }
        }
        if (minIndex != i) {
            swap(vals[i], vals[minIndex]);
        }
    }
}

void fullInsertionSort(vector<int>& vals) {
    if (vals.empty()) return;
    insertionSort(vals, 0, static_cast<int>(vals.size()) - 1);
}

// ========== 工具函数 ==========

// 输出排序结果到文件
void outputToFile(const vector<int>& sortedVals, const string& filename) {
    ofstream output_file(filename);
    
    if (!output_file.is_open()) {
        cerr << "无法打开输出文件: " << filename << endl;
        return;
    }
    
    for (size_t i = 0; i < sortedVals.size(); i++) {
        output_file << sortedVals[i];
        if (i != sortedVals.size() - 1) {
            output_file << " ";
        }
    }
    output_file << endl;
    
    output_file.close();
    cout << "排序结果已保存到: " << filename << endl;
}

// 获取策略名称
string getStrategyName(PivotStrategy strategy) {
    switch (strategy) {
        case FIXED: return "固定基准";
        case RANDOM: return "随机基准";
        case MEDIAN_OF_THREE: return "三数取中";
        default: return "未知策略";
    }
}

// 验证排序结果
bool validateSort(const vector<int>& vals) {
    for (size_t i = 1; i < vals.size(); i++) {
        if (vals[i] < vals[i-1]) {
            return false;
        }
    }
    return true;
}

