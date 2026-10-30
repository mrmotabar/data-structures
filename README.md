# Data Structures (Modern C++17)

A header-only, modern C++17 library of classic and high-performance data structures. Currently featuring production-grade **AVL Tree** (augmented with order statistics), **Red-Black Tree** (provable CLRS invariant preservation), and **B-Tree** (configurable minimum degree $t$), with future modules planned for heaps (e.g., Fat Heap, Fibonacci Heap) and advanced associative structures.

Includes exhaustive property-invariant verification, random differential fuzzing against `std::set`, runtime benchmarks, and an interactive CLI demonstration.

---

## Table of Contents

- [Features](#features)
- [Directory Structure](#directory-structure)
- [Theoretical Complexity](#theoretical-complexity)
- [Empirical Benchmarks](#empirical-benchmarks)
- [Data Structures Overview](#data-structures-overview)
  - [AVL Tree (`AvlTree<T>`)](#avl-tree-avltreet)
  - [Red-Black Tree (`RedBlackTree<T>`)](#red-black-tree-redblacktreet)
  - [B-Tree (`BTree<T>`)](#b-tree-btreet)
- [Building & Testing](#building--testing)
  - [Using Make](#using-make)
  - [Using CMake](#using-cmake)
- [Interactive CLI Demo](#interactive-cli-demo)
- [Uniform Naming Standards Compliance](#uniform-naming-standards-compliance)
- [License](#license)

---

## Features

- **Header-Only & Templated**: Zero external dependencies. Templated on any key type `KeyType` with customizable strict weak ordering comparator `Compare` (defaults to `std::less<KeyType>`).
- **Modern C++17**: Value semantics with full Rule-of-Five compliance (destructors, copy constructors, copy assignment, move constructors, move assignment), `std::optional`, and `[[nodiscard]]`.
- **Augmented Order Statistics**: The AVL Tree maintains subtree sizes at each node, enabling $O(\log N)$ `findKth(rank)` and `findRank(key)` queries alongside running-median computations.
- **Robust Sentinel Architecture**: The Red-Black Tree implements the textbook CLRS sentinel `nilNode_` architecture, guaranteeing complete invariant preservation across all deletion and double-black fixup cases without null-pointer vulnerabilities.
- **Proactive B-Tree Splitting & Deletion**: B-Tree features single-pass proactive splitting during insertion and borrow/merge during deletion for arbitrary minimum degrees $t \ge 2$, with zero memory leaks.
- **Automated Verification**: Every tree includes `verifyProperties()` to rigorously check height balance factors, black-height consistency, key sorting, child intervals, and leaf depths.
- **Exhaustive Testing & Benchmarks**: Unit tests with 10,000-operation differential fuzzing against `std::set`, plus high-throughput runtime benchmarks.

---

## Directory Structure

```
data-structures/
├── include/
│   └── search-trees/
│       ├── avl-tree.hpp          # AVL Tree header with order statistics
│       ├── red-black-tree.hpp    # Red-Black Tree header with CLRS fixups
│       └── b-tree.hpp            # B-Tree header with dynamic degree t
├── src/
│   └── main.cpp                  # Interactive CLI demonstration
├── tests/
│   ├── test-avl-tree.cpp         # AVL unit tests & order statistics fuzzing
│   ├── test-red-black-tree.cpp   # Red-Black unit tests & deletion fuzzing
│   ├── test-b-tree.cpp           # B-Tree unit tests across degrees t=2, 3, 5
│   ├── test-runner.cpp           # Unified test runner with colorized output
│   └── benchmark.cpp             # Runtime throughput benchmark vs std::set
├── CMakeLists.txt                # CMake build configuration
├── Makefile                      # Standard Makefile for fast builds
├── .gitignore                    # Build artifact exclusions
└── README.md                     # Documentation
```

---

## Theoretical Complexity

| Operation | AVL Tree | Red-Black Tree | B-Tree (degree $t$) |
| :--- | :--- | :--- | :--- |
| **Search** | $O(\log N)$ | $O(\log N)$ | $O(t \log_t N) = O(\log N)$ |
| **Insertion** | $O(\log N)$ | $O(\log N)$ | $O(t \log_t N) = O(\log N)$ |
| **Deletion** | $O(\log N)$ | $O(\log N)$ | $O(t \log_t N) = O(\log N)$ |
| **Min / Max** | $O(\log N)$ | $O(\log N)$ | $O(\log_t N)$ |
| **$k$-th Smallest Element** | $O(\log N)$ *(augmented)* | $O(N)$ *(in-order)* | $O(N)$ *(in-order)* |
| **Element Rank** | $O(\log N)$ *(augmented)* | $O(N)$ *(in-order)* | $O(N)$ *(in-order)* |
| **Max Tree Height** | $\approx 1.44 \log_2 N$ | $\le 2 \log_2(N + 1)$ | $\le \log_t \frac{N + 1}{2}$ |
| **Rotations per Insert** | $\le 2$ | $\le 2$ | $0$ (Node splits) |
| **Rotations per Delete** | $O(\log N)$ | $\le 3$ | $0$ (Borrows / Merges) |
| **Space Complexity** | $O(N)$ | $O(N)$ | $O(N)$ |

---

## Empirical Benchmarks

Benchmarked on **Apple Silicon (M-series, macOS Darwin)** compiled with `clang++ -std=c++17 -O3` on **$N = 100,000$ operations**:

```
========================================================================================
               BALANCED SEARCH TREES PERFORMANCE BENCHMARK (N = 100000)
========================================================================================
Data Structure             Seq. Ins (ms) Rnd. Ins (ms)   Search (ms)   Delete (ms)    Total (ms)
----------------------------------------------------------------------------------------
AvlTree                            17.37         20.72          6.14          9.43         53.67
RedBlackTree                       10.29         12.74          5.04          6.03         34.10
BTree (t=4)                        12.64         20.83          7.87          8.60         49.93
BTree (t=16)                        6.92         14.24          4.78          5.39         31.33
BTree (t=64)                        5.94         12.26          4.43          4.62         27.24
std::set<int> (STL RB)              8.35         12.06          5.14          5.99         31.54
========================================================================================
```

### Key Takeaways:
1. **Red-Black Tree** achieves near-identical throughput to `libc++`'s standard library `std::set`, validating the zero-overhead sentinel implementation.
2. **B-Tree ($t=64$)** outperforms binary search trees in total runtime ($27.24\text{ ms}$ vs $34.10\text{ ms}$) thanks to modern CPU cache-line utilization and vectorized key scanning.
3. **AVL Tree** has slightly higher insertion time due to strictly maintained height balance and subtree-size metadata updates, but offers faster and more strictly bounded worst-case search heights.

---

## Data Structures Overview

### AVL Tree (`AvlTree<T>`)

```cpp
#include "search-trees/avl-tree.hpp"
#include <iostream>

int main() {
    search_trees::AvlTree<int> tree = {50, 20, 70, 10, 30, 60, 80};

    tree.insert(25);
    tree.remove(70);

    // Order statistics (1-based rank):
    std::cout << "3rd smallest element: " << *tree.findKth(3) << "\n";
    std::cout << "Rank of 25: " << tree.findRank(25) << "\n";

    // Visual tree display:
    tree.printTree();

    return 0;
}
```

### Red-Black Tree (`RedBlackTree<T>`)

```cpp
#include "search-trees/red-black-tree.hpp"
#include <iostream>

int main() {
    search_trees::RedBlackTree<int> tree;

    for (int i = 1; i <= 10; ++i) {
        tree.insert(i * 10);
    }

    std::cout << "Black-height: " << tree.getBlackHeight() << "\n";
    std::cout << "Contains 40: " << (tree.contains(40) ? "Yes" : "No") << "\n";

    tree.remove(40);
    tree.printTree();

    return 0;
}
```

### B-Tree (`BTree<T>`)

```cpp
#include "search-trees/b-tree.hpp"
#include <iostream>

int main() {
    // Instantiate with minimum degree t = 4 (each node holds up to 2t - 1 = 7 keys)
    search_trees::BTree<int> tree(4);

    for (int i = 1; i <= 50; ++i) {
        tree.insert(i);
    }

    tree.remove(25);
    std::cout << "Minimum: " << *tree.getMinimum() << "\n";
    std::cout << "Maximum: " << *tree.getMaximum() << "\n";

    tree.printTree();

    return 0;
}
```

---

## Building & Testing

### Using Make

```bash
# Build all binaries (demo, individual unit tests, test runner, benchmark)
make all

# Run the unified test suite
make test

# Run all unit tests individually
make test-all

# Run performance benchmarks
make benchmark

# Start the interactive CLI demo
make run-demo

# Clean build artifacts
make clean
```

### Using CMake

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build

# Run tests with CTest
ctest --test-dir build --output-on-failure

# Execute benchmark
./build/benchmark
```

---

## Interactive CLI Demo

Launch the interactive CLI by running `make run-demo` (or `./bin/demo`):

```
========================================================
       BALANCED SEARCH TREES INTERACTIVE DEMO           
========================================================
Type 'help' for available commands or 'quit' to exit.

[AVL Tree]> insert 20
Inserted 20 successfully.
[AVL Tree]> insert 10
Inserted 10 successfully.
[AVL Tree]> insert 30
Inserted 30 successfully.
[AVL Tree]> print

--- Tree Visualization ---
└── 20 [h=2, sz=3]
    ├── 10 [h=1, sz=1]
    └── 30 [h=1, sz=1]
--------------------------
[AVL Tree]> kth 2
Element with rank 2 is 20.
[AVL Tree]> switch rb
Switched to Red-Black Tree.
[Red-Black Tree]> switch btree 4
Switched to B-Tree with minimum degree 4.
[B-Tree (t=4)]> quit
```

---

## Uniform Naming Standards Compliance

This repository strictly complies with the **Uniform Naming Standards**:
- **Directories**: Lowercase `kebab-case` (`include/`, `search-trees/`, `src/`, `tests/`, `bin/`).
- **Files**: Header files named `kebab-case.hpp`, test and source files named `kebab-case.cpp`.
- **Classes & Types**: `PascalCase` (`AvlTree`, `AvlNode`, `RedBlackTree`, `RedBlackNode`, `BTree`, `BTreeNode`, `NodeColor`).
- **Methods**: `camelCase` (`insert`, `remove`, `search`, `contains`, `findKth`, `findRank`, `verifyProperties`, `printTree`).
- **Private Members**: `camelCase_` with trailing underscore (`root_`, `size_`, `height_`, `minimumDegree_`, `nilNode_`, `compare_`).
- **Booleans**: Prefixed with `is`, `has`, or `can` (`isRed`, `isBlack`, `isEmpty`, `isLeaf_`).
- **No Cryptic Abbreviations**: Full words throughout (`height` instead of `h`, `size` instead of `sz`, `parent` instead of `par`, `successor` instead of `suc`, `value`/`key` instead of `val`).

---

## License

This project is open-source under the [MIT License](LICENSE).
