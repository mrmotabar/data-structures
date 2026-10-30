#include "search-trees/avl-tree.hpp"
#include "search-trees/b-tree.hpp"
#include "search-trees/red-black-tree.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <set>
#include <string>
#include <vector>

namespace {

struct BenchmarkMetrics {
    std::string dataStructureName;
    double sequentialInsertMs = 0.0;
    double randomInsertMs = 0.0;
    double randomSearchMs = 0.0;
    double randomDeleteMs = 0.0;
    double totalRuntimeMs = 0.0;
};

template <typename TreeType>
BenchmarkMetrics runBenchmarkForTree(const std::string& name,
                                    const std::vector<int>& sequentialData,
                                    const std::vector<int>& randomData,
                                    const std::vector<int>& searchQueries,
                                    const std::vector<int>& deleteData,
                                    TreeType treeFactory) {
    BenchmarkMetrics metrics;
    metrics.dataStructureName = name;

    // 1. Sequential Insertion
    {
        auto tree = treeFactory();
        auto start = std::chrono::high_resolution_clock::now();
        for (int key : sequentialData) {
            tree.insert(key);
        }
        auto end = std::chrono::high_resolution_clock::now();
        metrics.sequentialInsertMs = std::chrono::duration<double, std::milli>(end - start).count();
    }

    // 2. Random Insertion
    auto benchmarkTree = treeFactory();
    {
        auto start = std::chrono::high_resolution_clock::now();
        for (int key : randomData) {
            benchmarkTree.insert(key);
        }
        auto end = std::chrono::high_resolution_clock::now();
        metrics.randomInsertMs = std::chrono::duration<double, std::milli>(end - start).count();
    }

    // 3. Random Search (mix of hits and misses)
    {
        volatile std::size_t hitCount = 0;
        auto start = std::chrono::high_resolution_clock::now();
        for (int query : searchQueries) {
            if (benchmarkTree.contains(query)) {
                hitCount = hitCount + 1;
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        metrics.randomSearchMs = std::chrono::duration<double, std::milli>(end - start).count();
    }

    // 4. Random Deletion
    {
        auto start = std::chrono::high_resolution_clock::now();
        for (int key : deleteData) {
            benchmarkTree.remove(key);
        }
        auto end = std::chrono::high_resolution_clock::now();
        metrics.randomDeleteMs = std::chrono::duration<double, std::milli>(end - start).count();
    }

    metrics.totalRuntimeMs = metrics.sequentialInsertMs + metrics.randomInsertMs +
                             metrics.randomSearchMs + metrics.randomDeleteMs;
    return metrics;
}

// Specialization for std::set
BenchmarkMetrics runBenchmarkForStdSet(const std::vector<int>& sequentialData,
                                      const std::vector<int>& randomData,
                                      const std::vector<int>& searchQueries,
                                      const std::vector<int>& deleteData) {
    BenchmarkMetrics metrics;
    metrics.dataStructureName = "std::set<int> (STL RB)";

    // 1. Sequential Insertion
    {
        std::set<int> set;
        auto start = std::chrono::high_resolution_clock::now();
        for (int key : sequentialData) {
            set.insert(key);
        }
        auto end = std::chrono::high_resolution_clock::now();
        metrics.sequentialInsertMs = std::chrono::duration<double, std::milli>(end - start).count();
    }

    // 2. Random Insertion
    std::set<int> benchmarkSet;
    {
        auto start = std::chrono::high_resolution_clock::now();
        for (int key : randomData) {
            benchmarkSet.insert(key);
        }
        auto end = std::chrono::high_resolution_clock::now();
        metrics.randomInsertMs = std::chrono::duration<double, std::milli>(end - start).count();
    }

    // 3. Random Search
    {
        volatile std::size_t hitCount = 0;
        auto start = std::chrono::high_resolution_clock::now();
        for (int query : searchQueries) {
            if (benchmarkSet.find(query) != benchmarkSet.end()) {
                hitCount = hitCount + 1;
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        metrics.randomSearchMs = std::chrono::duration<double, std::milli>(end - start).count();
    }

    // 4. Random Deletion
    {
        auto start = std::chrono::high_resolution_clock::now();
        for (int key : deleteData) {
            benchmarkSet.erase(key);
        }
        auto end = std::chrono::high_resolution_clock::now();
        metrics.randomDeleteMs = std::chrono::duration<double, std::milli>(end - start).count();
    }

    metrics.totalRuntimeMs = metrics.sequentialInsertMs + metrics.randomInsertMs +
                             metrics.randomSearchMs + metrics.randomDeleteMs;
    return metrics;
}

void printMetricsTable(const std::vector<BenchmarkMetrics>& results, std::size_t dataCount) {
    std::cout << "\n========================================================================================\n";
    std::cout << "               BALANCED SEARCH TREES PERFORMANCE BENCHMARK (N = " << dataCount << ")\n";
    std::cout << "========================================================================================\n";
    std::cout << std::left << std::setw(26) << "Data Structure"
              << std::right << std::setw(14) << "Seq. Ins (ms)"
              << std::setw(14) << "Rnd. Ins (ms)"
              << std::setw(14) << "Search (ms)"
              << std::setw(14) << "Delete (ms)"
              << std::setw(14) << "Total (ms)" << "\n";
    std::cout << "----------------------------------------------------------------------------------------\n";

    for (const auto& item : results) {
        std::cout << std::left << std::setw(26) << item.dataStructureName
                  << std::right << std::fixed << std::setprecision(2)
                  << std::setw(14) << item.sequentialInsertMs
                  << std::setw(14) << item.randomInsertMs
                  << std::setw(14) << item.randomSearchMs
                  << std::setw(14) << item.randomDeleteMs
                  << std::setw(14) << item.totalRuntimeMs << "\n";
    }
    std::cout << "========================================================================================\n\n";
}

} // namespace

int main(int argc, char* argv[]) {
    std::size_t dataSize = 100000;
    if (argc > 1) {
        try {
            dataSize = std::stoul(argv[1]);
        } catch (...) {
            std::cerr << "Invalid data size argument. Using default: " << dataSize << "\n";
        }
    }

    std::cout << "Generating dataset for benchmark (N = " << dataSize << " keys)...\n";

    // 1. Sequential dataset
    std::vector<int> sequentialData(dataSize);
    std::iota(sequentialData.begin(), sequentialData.end(), 1);

    // 2. Random dataset (with permutation to ensure unique keys)
    std::vector<int> randomData = sequentialData;
    std::mt19937 rng(54321);
    std::shuffle(randomData.begin(), randomData.end(), rng);

    // 3. Search queries (50% present, 50% not present)
    std::vector<int> searchQueries(dataSize);
    std::uniform_int_distribution<int> queryDist(1, static_cast<int>(dataSize * 2));
    for (std::size_t i = 0; i < dataSize; ++i) {
        searchQueries[i] = queryDist(rng);
    }

    // 4. Deletion dataset (subset of 50,000 keys from randomData)
    std::size_t deleteCount = dataSize / 2;
    std::vector<int> deleteData(randomData.begin(), randomData.begin() + deleteCount);

    std::vector<BenchmarkMetrics> results;

    std::cout << "Running benchmark on AVL Tree...\n";
    results.push_back(runBenchmarkForTree(
        "AvlTree",
        sequentialData, randomData, searchQueries, deleteData,
        []() { return search_trees::AvlTree<int>(); }));

    std::cout << "Running benchmark on Red-Black Tree...\n";
    results.push_back(runBenchmarkForTree(
        "RedBlackTree",
        sequentialData, randomData, searchQueries, deleteData,
        []() { return search_trees::RedBlackTree<int>(); }));

    std::cout << "Running benchmark on B-Tree (t=4)...\n";
    results.push_back(runBenchmarkForTree(
        "BTree (t=4)",
        sequentialData, randomData, searchQueries, deleteData,
        []() { return search_trees::BTree<int>(4); }));

    std::cout << "Running benchmark on B-Tree (t=16)...\n";
    results.push_back(runBenchmarkForTree(
        "BTree (t=16)",
        sequentialData, randomData, searchQueries, deleteData,
        []() { return search_trees::BTree<int>(16); }));

    std::cout << "Running benchmark on B-Tree (t=64)...\n";
    results.push_back(runBenchmarkForTree(
        "BTree (t=64)",
        sequentialData, randomData, searchQueries, deleteData,
        []() { return search_trees::BTree<int>(64); }));

    std::cout << "Running benchmark on std::set (STL baseline)...\n";
    results.push_back(runBenchmarkForStdSet(sequentialData, randomData, searchQueries, deleteData));

    printMetricsTable(results, dataSize);

    return 0;
}
