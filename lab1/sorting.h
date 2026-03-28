#pragma once

#include <string>
#include <vector>

enum PivotStrategy {
    FIXED,
    RANDOM,
    MEDIAN_OF_THREE
};

void simpleQuicksort(std::vector<int>& arr);
void optimizedQuicksort(std::vector<int>& vals, PivotStrategy strategy = MEDIAN_OF_THREE,
                        int threshold = 10);
void quicksortWithGathering(std::vector<int>& vals, PivotStrategy strategy = MEDIAN_OF_THREE,
                            int threshold = 10);
void parallelQuicksort(std::vector<int>& vals, PivotStrategy strategy = MEDIAN_OF_THREE,
                       int threshold = 2048);
void quicksort(std::vector<int>& vals, PivotStrategy strategy = MEDIAN_OF_THREE,
               bool useGathering = true);

void mergeSort(std::vector<int>& vals);
void heapSort(std::vector<int>& vals);
void bubbleSort(std::vector<int>& vals);
void selectionSort(std::vector<int>& vals);
void fullInsertionSort(std::vector<int>& vals);

std::string getStrategyName(PivotStrategy strategy);
bool validateSort(const std::vector<int>& vals);
void outputToFile(const std::vector<int>& sortedVals, const std::string& filename = "sorted.txt");
