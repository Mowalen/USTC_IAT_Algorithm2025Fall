#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "sorting.h"
#include "sorting.cpp"

using namespace std;
using namespace std::chrono;

int main() {
    ifstream input_file("data.txt");
    if (!input_file.is_open()) {
        cerr << "无法打开输入文件 data.txt" << endl;
        return 1;
    }

    string line;
    int val_nums = 0;
    vector<int> vals;

    if (getline(input_file, line)) {
        stringstream ss(line);
        ss >> val_nums;
    }

    if (getline(input_file, line)) {
        stringstream ss(line);
        int val;
        while (ss >> val) {
            vals.push_back(val);
        }
    }

    input_file.close();

    cout << "读取数据数量: " << val_nums << endl;
    cout << "实际读取数据个数: " << vals.size() << endl;

    if (val_nums != static_cast<int>(vals.size())) {
        cout << "警告: 声明的数据数量与实际读取数量不一致" << endl;
        cout << "使用实际读取数量: " << vals.size() << endl;
    }

    PivotStrategy strategy = MEDIAN_OF_THREE;
    cout << "选择的策略: " << getStrategyName(strategy) << endl;

    vector<int> originalVals = vals;
    cout << "数据量: " << originalVals.size() << " 个元素" << endl;

    struct SortBenchmark {
        string name;
        function<void(vector<int>&)> sorter;
        bool writeToFile;
    };

    const vector<SortBenchmark> benchmarks = {
        // {"优化快速排序", [&](vector<int>& data) { optimizedQuicksort(data, strategy); }, false},
        // {"三路聚集快速排序", [&](vector<int>& data) { quicksortWithGathering(data, strategy); }, false},
        {"手写快速排序", [](vector<int>& data) { simpleQuicksort(data); }, false},
        {"手写快速排序优化版", [&](vector<int>& data) { parallelQuicksort(data, strategy, 32); }, true},
        {"归并排序", [](vector<int>& data) { mergeSort(data); }, false},
        {"堆排序", [](vector<int>& data) { heapSort(data); }, false},
        // {"插入排序", [](vector<int>& data) { fullInsertionSort(data); }, false},
        // {"选择排序", [](vector<int>& data) { selectionSort(data); }, false},
        // {"冒泡排序", [](vector<int>& data) { bubbleSort(data); }, false},
        {"标准库std::sort", [](vector<int>& data) { sort(data.begin(), data.end()); }, false}
    };

    struct BenchmarkResult {
        string name;
        double averageMs;
        bool allOk;
        size_t dataSize;
    };

    const int kBenchmarkRuns = 10;
    vector<BenchmarkResult> results;

    for (const auto& benchmark : benchmarks) {
        long long totalMs = 0;
        bool allOk = true;
        vector<int> lastSorted;

        for (int run = 0; run < kBenchmarkRuns; ++run) {
            vector<int> data = originalVals;

            auto start = high_resolution_clock::now();
            benchmark.sorter(data);
            auto end = high_resolution_clock::now();
            auto duration = duration_cast<milliseconds>(end - start);
            totalMs += duration.count();

            bool ok = validateSort(data);
            allOk = allOk && ok;
            if (run == kBenchmarkRuns - 1) {
                lastSorted = data;
            }

            if (!ok) {
                cerr << benchmark.name << " 第 " << (run + 1)
                     << " 次排序验证失败，结果可能不正确。" << endl;
            }
        }

        double averageMs = static_cast<double>(totalMs) / kBenchmarkRuns;
        results.push_back({benchmark.name, averageMs, allOk, originalVals.size()});

        if (benchmark.writeToFile && allOk) {
            outputToFile(lastSorted,"sorted.txt");
        }
    }

    cout << fixed << setprecision(3);
    cout << "\n=== 排序算法耗时对比（" << kBenchmarkRuns << " 次平均）===\n";
    for (const auto& result : results) {
        cout << result.name << ": " << result.averageMs << " 毫秒 | 数据规模: "
             << result.dataSize << " | 验证: " << (result.allOk ? "成功" : "失败") << endl;
    }
    cout << "排序结果已写入 sorted.txt (快速排序结果)" << endl;

    return 0;
}
