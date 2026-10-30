#include "search-trees/avl-tree.hpp"
#include "search-trees/b-tree.hpp"
#include "search-trees/red-black-tree.hpp"

#include <chrono>
#include <iostream>
#include <random>
#include <set>
#include <string>
#include <vector>

namespace {

// Terminal color helpers
constexpr const char* COLOR_RESET = "\033[0m";
constexpr const char* COLOR_GREEN = "\033[32m";
constexpr const char* COLOR_RED = "\033[31m";
constexpr const char* COLOR_CYAN = "\033[36m";
constexpr const char* COLOR_BOLD = "\033[1m";

void reportPass(const std::string& testName) {
    std::cout << "  " << COLOR_GREEN << "✔ [PASS]" << COLOR_RESET << " " << testName << "\n";
}

void reportFail(const std::string& testName, const std::string& reason) {
    std::cerr << "  " << COLOR_RED << "✖ [FAIL]" << COLOR_RESET << " " << testName
              << ": " << reason << "\n";
}

bool runAvlTreeTests() {
    std::cout << "\n" << COLOR_CYAN << COLOR_BOLD << "--- Running AVL Tree Test Suite ---" << COLOR_RESET << "\n";
    bool allPassed = true;

    // Test 1: Invariant validation on random data
    {
        search_trees::AvlTree<int> tree;
        std::set<int> reference;
        std::mt19937 rng(42);
        std::uniform_int_distribution<int> dist(1, 10000);

        for (int i = 0; i < 5000; ++i) {
            int val = dist(rng);
            tree.insert(val);
            reference.insert(val);
        }

        if (tree.getSize() == reference.size() && tree.verifyProperties()) {
            reportPass("AVL 5,000 random insertions & invariant check");
        } else {
            reportFail("AVL 5,000 random insertions", "Size mismatch or invariant failure");
            allPassed = false;
        }

        // Test 2: Order statistics
        bool orderStatsValid = true;
        auto sortedReference = std::vector<int>(reference.begin(), reference.end());
        for (std::size_t rank = 1; rank <= sortedReference.size(); rank += 500) {
            auto kth = tree.findKth(rank);
            if (!kth.has_value() || *kth != sortedReference[rank - 1]) {
                orderStatsValid = false;
                break;
            }
            if (tree.findRank(sortedReference[rank - 1]) != rank) {
                orderStatsValid = false;
                break;
            }
        }

        if (orderStatsValid) {
            reportPass("AVL Order statistics (findKth and findRank)");
        } else {
            reportFail("AVL Order statistics", "findKth or findRank mismatch");
            allPassed = false;
        }

        // Test 3: Deletions
        for (std::size_t i = 0; i < sortedReference.size() / 2; ++i) {
            tree.remove(sortedReference[i]);
            reference.erase(sortedReference[i]);
        }

        if (tree.getSize() == reference.size() && tree.verifyProperties()) {
            reportPass("AVL 2,500 deletions & invariant check");
        } else {
            reportFail("AVL Deletions", "Invariant failure after deletion");
            allPassed = false;
        }
    }

    return allPassed;
}

bool runRedBlackTreeTests() {
    std::cout << "\n" << COLOR_CYAN << COLOR_BOLD << "--- Running Red-Black Tree Test Suite ---" << COLOR_RESET << "\n";
    bool allPassed = true;

    search_trees::RedBlackTree<int> tree;
    std::set<int> reference;
    std::mt19937 rng(84);
    std::uniform_int_distribution<int> dist(1, 10000);

    for (int i = 0; i < 5000; ++i) {
        int val = dist(rng);
        tree.insert(val);
        reference.insert(val);
    }

    if (tree.getSize() == reference.size() && tree.verifyProperties()) {
        reportPass("Red-Black 5,000 random insertions & invariant check");
    } else {
        reportFail("Red-Black 5,000 random insertions", "Size mismatch or invariant failure");
        allPassed = false;
    }

    auto sortedReference = std::vector<int>(reference.begin(), reference.end());
    for (std::size_t i = 0; i < sortedReference.size() / 2; ++i) {
        tree.remove(sortedReference[i]);
        reference.erase(sortedReference[i]);
    }

    if (tree.getSize() == reference.size() && tree.verifyProperties()) {
        reportPass("Red-Black 2,500 deletions & invariant check");
    } else {
        reportFail("Red-Black Deletions", "Invariant failure after deletion");
        allPassed = false;
    }

    return allPassed;
}

bool runBTreeTests() {
    std::cout << "\n" << COLOR_CYAN << COLOR_BOLD << "--- Running B-Tree Test Suite ---" << COLOR_RESET << "\n";
    bool allPassed = true;

    for (std::size_t degree : {2, 3, 8}) {
        search_trees::BTree<int> tree(degree);
        std::set<int> reference;
        std::mt19937 rng(126);
        std::uniform_int_distribution<int> dist(1, 10000);

        for (int i = 0; i < 5000; ++i) {
            int val = dist(rng);
            tree.insert(val);
            reference.insert(val);
        }

        std::string testDesc = "B-Tree (t=" + std::to_string(degree) + ") 5,000 insertions & invariant check";
        if (tree.getSize() == reference.size() && tree.verifyProperties()) {
            reportPass(testDesc);
        } else {
            reportFail(testDesc, "Size mismatch or invariant failure");
            allPassed = false;
        }

        auto sortedReference = std::vector<int>(reference.begin(), reference.end());
        for (std::size_t i = 0; i < sortedReference.size() / 2; ++i) {
            tree.remove(sortedReference[i]);
            reference.erase(sortedReference[i]);
        }

        std::string delDesc = "B-Tree (t=" + std::to_string(degree) + ") 2,500 deletions & invariant check";
        if (tree.getSize() == reference.size() && tree.verifyProperties()) {
            reportPass(delDesc);
        } else {
            reportFail(delDesc, "Invariant failure after deletion");
            allPassed = false;
        }
    }

    return allPassed;
}

} // namespace

int main() {
    std::cout << COLOR_BOLD << "========================================================\n"
              << "       BALANCED SEARCH TREES UNIFIED TEST RUNNER       \n"
              << "========================================================" << COLOR_RESET << "\n";

    auto startTime = std::chrono::high_resolution_clock::now();

    bool avlSuccess = runAvlTreeTests();
    bool rbSuccess = runRedBlackTreeTests();
    bool btreeSuccess = runBTreeTests();

    auto endTime = std::chrono::high_resolution_clock::now();
    double totalElapsedMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    std::cout << "\n--------------------------------------------------------\n";
    if (avlSuccess && rbSuccess && btreeSuccess) {
        std::cout << COLOR_GREEN << COLOR_BOLD << "ALL TEST SUITES PASSED" << COLOR_RESET
                  << " in " << totalElapsedMs << " ms.\n";
        return 0;
    } else {
        std::cerr << COLOR_RED << COLOR_BOLD << "ONE OR MORE TEST SUITES FAILED!" << COLOR_RESET << "\n";
        return 1;
    }
}
