#include <bits/stdc++.h>
using namespace std;

struct Scheduler {
    int jobCount{};
    int machineCount{};
    vector<int> durations;           
    vector<int> originalIndex;       
    vector<int> suffixSum;           
    vector<int> load;                
    vector<int> assign;              
    vector<int> bestAssign;          
    int bestMakespan = INT_MAX;
    int avgLowerBound = 0;          

    void prepare(const vector<int> &jobs, int machines) {
        machineCount = machines;
        jobCount = static_cast<int>(jobs.size());
        durations.resize(jobCount);
        originalIndex.resize(jobCount);
        iota(originalIndex.begin(), originalIndex.end(), 0);


        sort(originalIndex.begin(), originalIndex.end(), [&](int a, int b) {
            if (jobs[a] != jobs[b]) return jobs[a] > jobs[b];
            return a < b;
        });
        for (int i = 0; i < jobCount; ++i) durations[i] = jobs[originalIndex[i]];

        suffixSum.assign(jobCount + 1, 0);
        for (int i = jobCount - 1; i >= 0; --i) suffixSum[i] = suffixSum[i + 1] + durations[i];

        int total = suffixSum[0];
        avgLowerBound = (total + machineCount - 1) / machineCount;

        load.assign(machineCount, 0);
        assign.assign(jobCount, -1);
        bestAssign.assign(jobCount, -1);
    }

    void dfs(int idx, int currentMax) {
        if (idx == jobCount) {
            bestMakespan = currentMax;
            bestAssign = assign;
            return;
        }

        int job = durations[idx];

        unordered_set<int> seenLoads;

        for (int m = 0; m < machineCount; ++m) {
            if (seenLoads.count(load[m])) continue;
            seenLoads.insert(load[m]);

            int newLoad = load[m] + job;
            int nextMax = max(currentMax, newLoad);

            if (nextMax >= bestMakespan) continue; 


            int lowerBound = max(nextMax, avgLowerBound);
            if (lowerBound >= bestMakespan) continue;

            load[m] = newLoad;
            assign[idx] = m;

            dfs(idx + 1, nextMax);

            load[m] -= job;
            assign[idx] = -1;

            
            if (load[m] == 0) break;
        }
    }

    pair<int, vector<vector<int>>> solve(const vector<int> &jobs, int machines) {
        prepare(jobs, machines);
        dfs(0, 0);

        vector<vector<int>> plan(machineCount);
        for (int i = 0; i < jobCount; ++i) {
            int machine = bestAssign[i];
            int origIdx = originalIndex[i];
            plan[machine].push_back(origIdx);
        }
        return {bestMakespan, plan};
    }
};

struct InputData {
    int n{};
    int m{};
    vector<int> jobs;
};

bool readInput(istream &in, InputData &data) {
    if (!(in >> data.n >> data.m)) return false;
    data.jobs.resize(data.n);
    for (int i = 0; i < data.n; ++i) {
        if (!(in >> data.jobs[i])) return false;
    }
    return true;
}

void printPlan(const InputData &data, int bestTime, const vector<vector<int>> &plan) {
    cout << "最佳总时间: " << bestTime << "（单位与输入一致）\n";
    cout << "调度方案：\n";
    for (int i = 0; i < static_cast<int>(plan.size()); ++i) {
        long long sum = 0;
        cout << "  机器 " << (i + 1) << ": ";
        if (plan[i].empty()) {
            cout << "无任务\n";
            continue;
        }
        for (size_t j = 0; j < plan[i].size(); ++j) {
            int idx = plan[i][j];
            int duration = data.jobs[idx];
            sum += duration;
            cout << "任务" << (idx + 1) << "(" << duration << ")";
            if (j + 1 < plan[i].size()) cout << " -> ";
        }
        cout << " | 累计: " << sum << "\n";
    }
}

bool solveFile(const string &filePath) {
    InputData data;
    ifstream fin(filePath);
    if (!fin) {
        cerr << "无法打开文件: " << filePath << "\n";
        return false;
    }
    if (!readInput(fin, data)) {
        cerr << "读取输入失败（" << filePath << "），格式应为：第一行 n m，第二行 n 个任务时间。\n";
        return false;
    }

    Scheduler scheduler;
    auto [bestTime, plan] = scheduler.solve(data.jobs, data.m);

    cout << "===== " << filePath << " =====\n";
    printPlan(data, bestTime, plan);
    cout << "\n";
    return true;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<string> files = {"test1.txt", "test2.txt", "test3.txt"};
    bool anySucceeded = false;
    for (const auto &file : files) {
        anySucceeded = solveFile(file) || anySucceeded;
    }

    if (!anySucceeded) {
        cerr << "三个测试文件均未能成功读取，请检查文件路径或格式。\n";
        return 1;
    }
    return 0;
}
