#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>

// 点结构体，记录编号及平面坐标
struct Point {
    int id{};
    double x{};
    double y{};
};

// 结果结构体，保存最小距离平方及对应的两个点
struct Result {
    double dist2 = std::numeric_limits<double>::infinity();
    Point a{};
    Point b{};
};

// 计算两点间欧氏距离的平方，避免重复开方
double distanceSquared(const Point& p, const Point& q) {
    const double dx = p.x - q.x;
    const double dy = p.y - q.y;
    return dx * dx + dy * dy;
}

// 合并左右子问题的最佳解，取距离更小者
Result combineBest(Result left, Result right) {
    return (left.dist2 <= right.dist2) ? left : right;
}

// 暴力枚举子区间内的点对，并保持该区间按 y 坐标有序
Result bruteForce(std::vector<Point>& pts, int l, int r) {
    Result best;
    for (int i = l; i < r; ++i) {
        for (int j = i + 1; j < r; ++j) {
            double d2 = distanceSquared(pts[i], pts[j]);
            if (d2 < best.dist2) {
                best.dist2 = d2;
                best.a = pts[i];
                best.b = pts[j];
            }
        }
    }
    std::sort(pts.begin() + l, pts.begin() + r, [](const Point& a, const Point& b) {
        return a.y < b.y || (a.y == b.y && a.x < b.x);
    });
    return best;
}

// 朴素 O(n^2) 最接近点对算法，用于对比运行时间
Result naiveClosestPair(const std::vector<Point>& pts) {
    Result best;
    for (size_t i = 0; i < pts.size(); ++i) {
        for (size_t j = i + 1; j < pts.size(); ++j) {
            double d2 = distanceSquared(pts[i], pts[j]);
            if (d2 < best.dist2) {
                best.dist2 = d2;
                best.a = pts[i];
                best.b = pts[j];
            }
        }
    }
    if (best.a.id > best.b.id) {
        Result ordered = best;
        std::swap(ordered.a, ordered.b);
        return ordered;
    }
    return best;
}

// 分治递归：pts[l:r) 保持按 x 有序，buffer 为临时数组
Result closestUtil(std::vector<Point>& pts, std::vector<Point>& buffer, int l, int r) {
    if (r - l <= 3) {
        return bruteForce(pts, l, r);
    }

    int mid = l + (r - l) / 2;
    double midX = pts[mid].x;

    Result leftBest = closestUtil(pts, buffer, l, mid);
    Result rightBest = closestUtil(pts, buffer, mid, r);
    Result best = combineBest(leftBest, rightBest);

    // 归并排序按 y 坐标合并，使 pts[l:r) 始终按 y 有序
    int i = l;
    int j = mid;
    int k = l;
    while (i < mid && j < r) {
        if (pts[i].y < pts[j].y || (pts[i].y == pts[j].y && pts[i].x < pts[j].x)) {
            buffer[k++] = pts[i++];
        } else {
            buffer[k++] = pts[j++];
        }
    }
    while (i < mid) buffer[k++] = pts[i++];
    while (j < r) buffer[k++] = pts[j++];
    for (int idx = l; idx < r; ++idx) {
        pts[idx] = buffer[idx];
    }

    // 构建“条带”区域，仅保留与分割线横坐标差值小于 delta 的点
    std::vector<Point> strip;
    strip.reserve(r - l);
    double delta = std::sqrt(best.dist2);
    for (int idx = l; idx < r; ++idx) {
        if (std::abs(pts[idx].x - midX) < delta) {
            strip.push_back(pts[idx]);
        }
    }

    // 条带内每个点只需与后续至多 7 个点比较
    for (size_t a = 0; a < strip.size(); ++a) {
        for (size_t b = a + 1; b < strip.size(); ++b) {
            if (strip[b].y - strip[a].y >= delta) {
                break;
            }
            double d2 = distanceSquared(strip[a], strip[b]);
            if (d2 < best.dist2) {
                best.dist2 = d2;
                best.a = strip[a];
                best.b = strip[b];
                delta = std::sqrt(best.dist2);
            }
        }
    }

    return best;
}

// 对输入点集排序后调用分治过程
Result closestPair(std::vector<Point> pts) {
    if (pts.size() < 2) {
        return Result{};
    }

    std::sort(pts.begin(), pts.end(), [](const Point& a, const Point& b) {
        return a.x < b.x || (a.x == b.x && a.y < b.y);
    });

    std::vector<Point> buffer(pts.size());
    Result best = closestUtil(pts, buffer, 0, static_cast<int>(pts.size()));

    if (best.a.id > best.b.id) {
        std::swap(best.a, best.b);
    }
    return best;
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::ifstream input("data.txt");
    if (!input) {
        std::cerr << "Failed to open data.txt\n";
        return 1;
    }

    std::vector<Point> pts;
    Point p;
    while (input >> p.id >> p.x >> p.y) {
        pts.push_back(p);
    }

    if (pts.size() < 2) {
        std::cout << "Insufficient points\n";
        return 0;
    }

    const auto naiveStart = std::chrono::high_resolution_clock::now();
    Result naiveBest = naiveClosestPair(pts);
    const auto naiveEnd = std::chrono::high_resolution_clock::now();
    auto naiveDuration =
        std::chrono::duration_cast<std::chrono::microseconds>(naiveEnd - naiveStart).count();

    const auto dcStart = std::chrono::high_resolution_clock::now();
    Result best = closestPair(pts);
    const auto dcEnd = std::chrono::high_resolution_clock::now();
    auto dcDuration =
        std::chrono::duration_cast<std::chrono::microseconds>(dcEnd - dcStart).count();

    double distance = std::sqrt(best.dist2);
    double naiveDistance = std::sqrt(naiveBest.dist2);

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Naive algorithm: (" << naiveBest.a.id << ", " << naiveBest.a.x << ", " << naiveBest.a.y << ") and ("
              << naiveBest.b.id << ", " << naiveBest.b.x << ", " << naiveBest.b.y << ")\n";
    std::cout << "Naive Distance: " << naiveDistance << "\n";
    std::cout << "Divide and conquer: (" << best.a.id << ", " << best.a.x << ", " << best.a.y << ") and ("
              << best.b.id << ", " << best.b.x << ", " << best.b.y << ")\n";
    std::cout << "Divide and conquer Distance: " << distance << "\n";

    std::cout << "Naive algorithm time: " << naiveDuration << " microseconds "<<"\n";
    std::cout << "Divide and conquer time: " << dcDuration << " microseconds\n";
    return 0;
}
