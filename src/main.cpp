#include "search-trees/avl-tree.hpp"
#include "search-trees/b-tree.hpp"
#include "search-trees/red-black-tree.hpp"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>

namespace {

void printHelp() {
    std::cout << "\nAvailable Commands:\n"
              << "  insert <key>   - Insert key into active tree\n"
              << "  remove <key>   - Remove key from active tree\n"
              << "  search <key>   - Check if key exists\n"
              << "  kth <rank>     - Find k-th smallest element (AVL only)\n"
              << "  rank <key>     - Find 1-based rank of element (AVL only)\n"
              << "  min            - Print minimum element\n"
              << "  max            - Print maximum element\n"
              << "  print          - Visual display of active tree\n"
              << "  size           - Print number of elements in tree\n"
              << "  clear          - Clear all elements\n"
              << "  switch <type>  - Switch tree (avl, rb, btree [degree])\n"
              << "  help           - Print this help message\n"
              << "  quit / exit    - Terminate program\n\n";
}

} // namespace

int main() {
    std::cout << "========================================================\n"
              << "       BALANCED SEARCH TREES INTERACTIVE DEMO           \n"
              << "========================================================\n"
              << "Type 'help' for available commands or 'quit' to exit.\n\n";

    enum class TreeType {
        Avl,
        RedBlack,
        BTree
    };

    TreeType currentType = TreeType::Avl;
    search_trees::AvlTree<int> avlTree;
    search_trees::RedBlackTree<int> rbTree;
    std::unique_ptr<search_trees::BTree<int>> btree = std::make_unique<search_trees::BTree<int>>(3);

    auto getTreeTypeName = [&]() -> std::string {
        switch (currentType) {
            case TreeType::Avl: return "AVL Tree";
            case TreeType::RedBlack: return "Red-Black Tree";
            case TreeType::BTree: return "B-Tree (t=" + std::to_string(btree->getMinimumDegree()) + ")";
        }
        return "";
    };

    std::string line;
    while (true) {
        std::cout << "[" << getTreeTypeName() << "]> ";
        if (!std::getline(std::cin, line)) {
            break;
        }

        std::stringstream ss(line);
        std::string command;
        if (!(ss >> command)) {
            continue;
        }

        if (command == "quit" || command == "exit" || command == "q") {
            std::cout << "Exiting. Goodbye!\n";
            break;
        } else if (command == "help" || command == "h") {
            printHelp();
        } else if (command == "switch") {
            std::string type;
            if (ss >> type) {
                if (type == "avl") {
                    currentType = TreeType::Avl;
                    std::cout << "Switched to AVL Tree.\n";
                } else if (type == "rb" || type == "redblack") {
                    currentType = TreeType::RedBlack;
                    std::cout << "Switched to Red-Black Tree.\n";
                } else if (type == "btree" || type == "b") {
                    std::size_t degree = 3;
                    ss >> degree;
                    if (degree < 2) degree = 2;
                    btree = std::make_unique<search_trees::BTree<int>>(degree);
                    currentType = TreeType::BTree;
                    std::cout << "Switched to B-Tree with minimum degree " << degree << ".\n";
                } else {
                    std::cerr << "Unknown tree type. Options: avl, rb, btree [degree]\n";
                }
            }
        } else if (command == "insert" || command == "i") {
            int key;
            if (ss >> key) {
                bool inserted = false;
                if (currentType == TreeType::Avl) inserted = avlTree.insert(key);
                else if (currentType == TreeType::RedBlack) inserted = rbTree.insert(key);
                else inserted = btree->insert(key);

                if (inserted) {
                    std::cout << "Inserted " << key << " successfully.\n";
                } else {
                    std::cout << "Key " << key << " already exists.\n";
                }
            } else {
                std::cerr << "Usage: insert <key>\n";
            }
        } else if (command == "remove" || command == "delete" || command == "d") {
            int key;
            if (ss >> key) {
                bool removed = false;
                if (currentType == TreeType::Avl) removed = avlTree.remove(key);
                else if (currentType == TreeType::RedBlack) removed = rbTree.remove(key);
                else removed = btree->remove(key);

                if (removed) {
                    std::cout << "Removed " << key << " successfully.\n";
                } else {
                    std::cout << "Key " << key << " not found.\n";
                }
            } else {
                std::cerr << "Usage: remove <key>\n";
            }
        } else if (command == "search" || command == "s" || command == "find") {
            int key;
            if (ss >> key) {
                bool found = false;
                if (currentType == TreeType::Avl) found = avlTree.contains(key);
                else if (currentType == TreeType::RedBlack) found = rbTree.contains(key);
                else found = btree->contains(key);

                std::cout << (found ? "Found: Key is present.\n" : "Not Found: Key is not present.\n");
            } else {
                std::cerr << "Usage: search <key>\n";
            }
        } else if (command == "kth") {
            if (currentType != TreeType::Avl) {
                std::cerr << "'kth' is currently supported on AVL Tree (augmented order statistics).\n";
            } else {
                std::size_t rank;
                if (ss >> rank) {
                    auto result = avlTree.findKth(rank);
                    if (result.has_value()) {
                        std::cout << "Element with rank " << rank << " is " << *result << ".\n";
                    } else {
                        std::cout << "Rank " << rank << " is out of bounds [1, " << avlTree.getSize() << "].\n";
                    }
                } else {
                    std::cerr << "Usage: kth <rank>\n";
                }
            }
        } else if (command == "rank") {
            if (currentType != TreeType::Avl) {
                std::cerr << "'rank' is currently supported on AVL Tree (augmented order statistics).\n";
            } else {
                int key;
                if (ss >> key) {
                    std::size_t rank = avlTree.findRank(key);
                    if (rank > 0) {
                        std::cout << "Rank of " << key << " is " << rank << ".\n";
                    } else {
                        std::cout << "Key " << key << " is not present in the tree.\n";
                    }
                } else {
                    std::cerr << "Usage: rank <key>\n";
                }
            }
        } else if (command == "min") {
            std::optional<int> minVal;
            if (currentType == TreeType::Avl) minVal = avlTree.getMinimum();
            else if (currentType == TreeType::RedBlack) minVal = rbTree.getMinimum();
            else minVal = btree->getMinimum();

            if (minVal.has_value()) {
                std::cout << "Minimum: " << *minVal << "\n";
            } else {
                std::cout << "Tree is empty.\n";
            }
        } else if (command == "max") {
            std::optional<int> maxVal;
            if (currentType == TreeType::Avl) maxVal = avlTree.getMaximum();
            else if (currentType == TreeType::RedBlack) maxVal = rbTree.getMaximum();
            else maxVal = btree->getMaximum();

            if (maxVal.has_value()) {
                std::cout << "Maximum: " << *maxVal << "\n";
            } else {
                std::cout << "Tree is empty.\n";
            }
        } else if (command == "size") {
            std::size_t sz = 0;
            if (currentType == TreeType::Avl) sz = avlTree.getSize();
            else if (currentType == TreeType::RedBlack) sz = rbTree.getSize();
            else sz = btree->getSize();
            std::cout << "Tree size: " << sz << " elements.\n";
        } else if (command == "print" || command == "p" || command == "show") {
            std::cout << "\n--- Tree Visualization ---\n";
            if (currentType == TreeType::Avl) avlTree.printTree();
            else if (currentType == TreeType::RedBlack) rbTree.printTree();
            else btree->printTree();
            std::cout << "--------------------------\n";
        } else if (command == "clear") {
            if (currentType == TreeType::Avl) avlTree.clear();
            else if (currentType == TreeType::RedBlack) rbTree.clear();
            else btree->clear();
            std::cout << "Tree cleared.\n";
        } else {
            std::cerr << "Unknown command: '" << command << "'. Type 'help' for instructions.\n";
        }
    }

    return 0;
}
