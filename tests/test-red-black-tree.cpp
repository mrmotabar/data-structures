#include "search-trees/red-black-tree.hpp"

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
    std::cout << "  [Red-Black] Running testBasicOperations...\n";
    search_trees::RedBlackTree<int> tree;
    assert(tree.isEmpty());
    assert(tree.getSize() == 0);
    assert(tree.getBlackHeight() == 0);
    assert(!tree.contains(15));
    assert(tree.getMinimum() == std::nullopt);
    assert(tree.getMaximum() == std::nullopt);

    // Insert elements
    assert(tree.insert(20));
    assert(tree.insert(10));
    assert(tree.insert(30));
    assert(!tree.insert(20)); // Duplicate

    assert(tree.getSize() == 3);
    assert(tree.contains(10));
    assert(tree.contains(20));
    assert(tree.contains(30));
    assert(!tree.contains(99));

    assert(tree.getMinimum() == 10);
    assert(tree.getMaximum() == 30);
    assert(tree.verifyProperties());

    // In-order traversal
    std::vector<int> expected = {10, 20, 30};
    assert(tree.toInOrderVector() == expected);

    // Deletion
    assert(tree.remove(20)); // Root deletion
    assert(!tree.contains(20));
    assert(tree.getSize() == 2);
    assert(tree.verifyProperties());

    assert(tree.remove(10));
    assert(tree.remove(30));
    assert(!tree.remove(50));
    assert(tree.isEmpty());
    assert(tree.verifyProperties());
}

void testRebalancingAndInvariants() {
    std::cout << "  [Red-Black] Running testRebalancingAndInvariants...\n";

    // Sequential ascending insert (tests right-leaning balance)
    {
        search_trees::RedBlackTree<int> tree;
        for (int i = 1; i <= 50; ++i) {
            assert(tree.insert(i));
            assert(tree.verifyProperties());
        }
        assert(tree.getSize() == 50);
        assert(tree.getBlackHeight() > 0);
    }

    // Sequential descending insert (tests left-leaning balance)
    {
        search_trees::RedBlackTree<int> tree;
        for (int i = 50; i >= 1; --i) {
            assert(tree.insert(i));
            assert(tree.verifyProperties());
        }
        assert(tree.getSize() == 50);
        assert(tree.getBlackHeight() > 0);
    }
}

void testDeletionCases() {
    std::cout << "  [Red-Black] Running testDeletionCases...\n";
    search_trees::RedBlackTree<int> tree;

    // Insert 1 to 30
    for (int i = 1; i <= 30; ++i) {
        tree.insert(i);
    }
    assert(tree.verifyProperties());

    // Delete in reverse order
    for (int i = 30; i >= 15; --i) {
        assert(tree.remove(i));
        assert(tree.verifyProperties());
    }

    // Delete odd numbers
    for (int i = 1; i <= 14; i += 2) {
        assert(tree.remove(i));
        assert(tree.verifyProperties());
    }

    // Remaining even numbers: 2, 4, 6, 8, 10, 12, 14
    for (int i = 2; i <= 14; i += 2) {
        assert(tree.contains(i));
        assert(tree.remove(i));
        assert(tree.verifyProperties());
    }

    assert(tree.isEmpty());
}

void testCopyAndMoveSemantics() {
    std::cout << "  [Red-Black] Running testCopyAndMoveSemantics...\n";
    search_trees::RedBlackTree<int> original = {10, 20, 30, 40, 50, 25, 15};
    assert(original.verifyProperties());

    // Copy constructor
    search_trees::RedBlackTree<int> copied = original;
    assert(copied.verifyProperties());
    assert(copied.getSize() == original.getSize());
    assert(copied.toInOrderVector() == original.toInOrderVector());

    copied.insert(999);
    assert(copied.contains(999));
    assert(!original.contains(999));

    // Copy assignment
    search_trees::RedBlackTree<int> assigned;
    assigned = original;
    assert(assigned.verifyProperties());
    assert(assigned.toInOrderVector() == original.toInOrderVector());

    // Move constructor
    search_trees::RedBlackTree<int> moved = std::move(copied);
    assert(moved.verifyProperties());
    assert(moved.contains(999));

    // Move assignment
    search_trees::RedBlackTree<int> moveAssigned;
    moveAssigned = std::move(assigned);
    assert(moveAssigned.verifyProperties());
}

void testRandomOperationsAgainstStdSet() {
    std::cout << "  [Red-Black] Running testRandomOperationsAgainstStdSet (10,000 operations)...\n";
    search_trees::RedBlackTree<int> tree;
    std::set<int> referenceSet;

    std::mt19937 randomEngine(4242);
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
    std::cout << "=== Red-Black Tree Correctness Test Suite ===\n";
    testBasicOperations();
    testRebalancingAndInvariants();
    testDeletionCases();
    testCopyAndMoveSemantics();
    testRandomOperationsAgainstStdSet();
    std::cout << ">>> All Red-Black Tree tests passed successfully!\n\n";
    return 0;
}
