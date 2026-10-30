#include "search-trees/avl-tree.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <iostream>
#include <random>
#include <set>
#include <string>
#include <vector>

namespace {

void testBasicOperations() {
    std::cout << "  [AVL] Running testBasicOperations...\n";
    search_trees::AvlTree<int> tree;
    assert(tree.isEmpty());
    assert(tree.getSize() == 0);
    assert(tree.getHeight() == 0);
    assert(!tree.contains(10));
    assert(tree.getMinimum() == std::nullopt);
    assert(tree.getMaximum() == std::nullopt);

    // Insert elements
    assert(tree.insert(10));
    assert(tree.insert(20));
    assert(tree.insert(30));
    assert(!tree.insert(10)); // Duplicate

    assert(tree.getSize() == 3);
    assert(tree.contains(10));
    assert(tree.contains(20));
    assert(tree.contains(30));
    assert(!tree.contains(40));

    assert(tree.getMinimum() == 10);
    assert(tree.getMaximum() == 30);
    assert(tree.verifyProperties());

    // In-order vector check
    std::vector<int> expected = {10, 20, 30};
    assert(tree.toInOrderVector() == expected);

    // Deletion
    assert(tree.remove(20));
    assert(!tree.contains(20));
    assert(tree.getSize() == 2);
    assert(tree.verifyProperties());

    assert(tree.remove(10));
    assert(tree.remove(30));
    assert(!tree.remove(50));
    assert(tree.isEmpty());
    assert(tree.verifyProperties());
}

void testOrderStatistics() {
    std::cout << "  [AVL] Running testOrderStatistics...\n";
    search_trees::AvlTree<int> tree = {50, 20, 70, 10, 30, 60, 80};
    assert(tree.verifyProperties());
    assert(tree.getSize() == 7);

    // Sorted order: 10, 20, 30, 50, 60, 70, 80
    assert(tree.findKth(1) == 10);
    assert(tree.findKth(2) == 20);
    assert(tree.findKth(3) == 30);
    assert(tree.findKth(4) == 50);
    assert(tree.findKth(5) == 60);
    assert(tree.findKth(6) == 70);
    assert(tree.findKth(7) == 80);
    assert(tree.findKth(0) == std::nullopt); // Out of bounds
    assert(tree.findKth(8) == std::nullopt);

    // Rank test
    assert(tree.findRank(10) == 1);
    assert(tree.findRank(50) == 4);
    assert(tree.findRank(80) == 7);
    assert(tree.findRank(999) == 0); // Not found

    // Remove an element and recheck order statistics
    tree.remove(20);
    assert(tree.verifyProperties());
    // Remaining: 10, 30, 50, 60, 70, 80
    assert(tree.findKth(2) == 30);
    assert(tree.findRank(30) == 2);
    assert(tree.findRank(20) == 0);
}

void testRotationsAndBalancing() {
    std::cout << "  [AVL] Running testRotationsAndBalancing...\n";

    // Test Left-Left (LL) rotation: sequential descending insert
    {
        search_trees::AvlTree<int> tree;
        for (int i = 10; i >= 1; --i) {
            tree.insert(i);
            assert(tree.verifyProperties());
        }
        assert(tree.getHeight() <= 4);
    }

    // Test Right-Right (RR) rotation: sequential ascending insert
    {
        search_trees::AvlTree<int> tree;
        for (int i = 1; i <= 10; ++i) {
            tree.insert(i);
            assert(tree.verifyProperties());
        }
        assert(tree.getHeight() <= 4);
    }

    // Test LR and RL rotations
    {
        search_trees::AvlTree<int> tree = {10, 5, 7, 20, 15};
        assert(tree.verifyProperties());
    }
}

void testCopyAndMoveSemantics() {
    std::cout << "  [AVL] Running testCopyAndMoveSemantics...\n";
    search_trees::AvlTree<int> original = {15, 6, 18, 3, 7, 17, 20};
    assert(original.verifyProperties());

    // Copy constructor
    search_trees::AvlTree<int> copied = original;
    assert(copied.verifyProperties());
    assert(copied.getSize() == original.getSize());
    assert(copied.toInOrderVector() == original.toInOrderVector());

    // Modify copy without affecting original
    copied.insert(100);
    assert(copied.contains(100));
    assert(!original.contains(100));

    // Copy assignment
    search_trees::AvlTree<int> assigned;
    assigned = original;
    assert(assigned.verifyProperties());
    assert(assigned.toInOrderVector() == original.toInOrderVector());

    // Move constructor
    search_trees::AvlTree<int> moved = std::move(copied);
    assert(moved.verifyProperties());
    assert(moved.contains(100));

    // Move assignment
    search_trees::AvlTree<int> moveAssigned;
    moveAssigned = std::move(assigned);
    assert(moveAssigned.verifyProperties());
}

void testRandomOperationsAgainstStdSet() {
    std::cout << "  [AVL] Running testRandomOperationsAgainstStdSet (10,000 operations)...\n";
    search_trees::AvlTree<int> tree;
    std::set<int> referenceSet;

    std::mt19937 randomEngine(1337);
    std::uniform_int_distribution<int> operationDist(0, 2);
    std::uniform_int_distribution<int> valueDist(1, 2000);

    for (int step = 0; step < 10000; ++step) {
        int operation = operationDist(randomEngine);
        int value = valueDist(randomEngine);

        if (operation == 0) {
            // Insert
            bool treeResult = tree.insert(value);
            bool setResult = referenceSet.insert(value).second;
            assert(treeResult == setResult);
        } else if (operation == 1) {
            // Remove
            bool treeResult = tree.remove(value);
            bool setResult = (referenceSet.erase(value) > 0);
            assert(treeResult == setResult);
        } else {
            // Contains
            bool treeResult = tree.contains(value);
            bool setResult = (referenceSet.find(value) != referenceSet.end());
            assert(treeResult == setResult);
        }

        assert(tree.getSize() == referenceSet.size());
    }

    assert(tree.verifyProperties());
    assert(tree.toInOrderVector() == std::vector<int>(referenceSet.begin(), referenceSet.end()));
}

} // namespace

int main() {
    std::cout << "=== AVL Tree Correctness Test Suite ===\n";
    testBasicOperations();
    testOrderStatistics();
    testRotationsAndBalancing();
    testCopyAndMoveSemantics();
    testRandomOperationsAgainstStdSet();
    std::cout << ">>> All AVL Tree tests passed successfully!\n\n";
    return 0;
}
