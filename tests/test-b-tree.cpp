#include "search-trees/b-tree.hpp"

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
    std::cout << "  [B-Tree] Running testBasicOperations...\n";
    search_trees::BTree<int> tree(3); // t = 3
    assert(tree.isEmpty());
    assert(tree.getSize() == 0);
    assert(tree.getMinimumDegree() == 3);
    assert(!tree.contains(50));
    assert(tree.getMinimum() == std::nullopt);
    assert(tree.getMaximum() == std::nullopt);

    // Insert keys
    assert(tree.insert(10));
    assert(tree.insert(20));
    assert(tree.insert(5));
    assert(tree.insert(6));
    assert(tree.insert(12));
    assert(tree.insert(30));
    assert(tree.insert(7));
    assert(tree.insert(17));
    assert(!tree.insert(10)); // Duplicate

    assert(tree.getSize() == 8);
    assert(tree.contains(12));
    assert(tree.contains(7));
    assert(!tree.contains(99));

    assert(tree.getMinimum() == 5);
    assert(tree.getMaximum() == 30);
    assert(tree.verifyProperties());

    // In-order traversal
    std::vector<int> expected = {5, 6, 7, 10, 12, 17, 20, 30};
    assert(tree.toInOrderVector() == expected);

    // Deletion
    assert(tree.remove(6));
    assert(!tree.contains(6));
    assert(tree.verifyProperties());

    assert(tree.remove(10));
    assert(tree.remove(5));
    assert(tree.remove(30));
    assert(!tree.remove(99));
    assert(tree.verifyProperties());
}

void testDegreesAndSplits() {
    std::cout << "  [B-Tree] Running testDegreesAndSplits (degrees 2, 3, 5)...\n";
    const std::vector<std::size_t> testDegrees = {2, 3, 5};

    for (std::size_t degree : testDegrees) {
        search_trees::BTree<int> tree(degree);
        for (int i = 1; i <= 200; ++i) {
            assert(tree.insert(i));
        }
        assert(tree.getSize() == 200);
        assert(tree.verifyProperties());

        // Sequential deletion
        for (int i = 1; i <= 100; ++i) {
            assert(tree.remove(i));
        }
        assert(tree.getSize() == 100);
        assert(tree.verifyProperties());

        // Reverse deletion
        for (int i = 200; i >= 101; --i) {
            assert(tree.remove(i));
        }
        assert(tree.isEmpty());
    }
}

void testCopyAndMoveSemantics() {
    std::cout << "  [B-Tree] Running testCopyAndMoveSemantics...\n";
    search_trees::BTree<int> original(2);
    for (int i = 1; i <= 30; ++i) {
        original.insert(i * 3);
    }
    assert(original.verifyProperties());

    // Copy constructor
    search_trees::BTree<int> copied = original;
    assert(copied.verifyProperties());
    assert(copied.getSize() == original.getSize());
    assert(copied.toInOrderVector() == original.toInOrderVector());

    copied.insert(1001);
    assert(copied.contains(1001));
    assert(!original.contains(1001));

    // Copy assignment
    search_trees::BTree<int> assigned(3);
    assigned = original;
    assert(assigned.verifyProperties());
    assert(assigned.toInOrderVector() == original.toInOrderVector());

    // Move constructor
    search_trees::BTree<int> moved = std::move(copied);
    assert(moved.verifyProperties());
    assert(moved.contains(1001));

    // Move assignment
    search_trees::BTree<int> moveAssigned;
    moveAssigned = std::move(assigned);
    assert(moveAssigned.verifyProperties());
}

void testRandomOperationsAgainstStdSet() {
    std::cout << "  [B-Tree] Running testRandomOperationsAgainstStdSet (10,000 operations, t=3)...\n";
    search_trees::BTree<int> tree(3);
    std::set<int> referenceSet;

    std::mt19937 randomEngine(9999);
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
    std::cout << "=== B-Tree Correctness Test Suite ===\n";
    testBasicOperations();
    testDegreesAndSplits();
    testCopyAndMoveSemantics();
    testRandomOperationsAgainstStdSet();
    std::cout << ">>> All B-Tree tests passed successfully!\n\n";
    return 0;
}
