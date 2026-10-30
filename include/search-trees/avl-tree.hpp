#ifndef SEARCH_TREES_AVL_TREE_HPP
#define SEARCH_TREES_AVL_TREE_HPP

#include <algorithm>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <optional>
#include <queue>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace search_trees {

/**
 * @brief Internal node representation for the AVL Tree.
 *
 * Each node maintains:
 * - key_: The stored value.
 * - height_: Height of the subtree rooted at this node (leaf height = 1).
 * - size_: Number of nodes in the subtree rooted at this node (useful for order statistics).
 * - leftChild_: Pointer to the left child node.
 * - rightChild_: Pointer to the right child node.
 */
template <typename KeyType>
class AvlNode {
public:
    KeyType key_;
    int height_;
    std::size_t size_;
    AvlNode* leftChild_;
    AvlNode* rightChild_;

    explicit AvlNode(const KeyType& key)
        : key_(key),
          height_(1),
          size_(1),
          leftChild_(nullptr),
          rightChild_(nullptr) {}

    explicit AvlNode(KeyType&& key)
        : key_(std::move(key)),
          height_(1),
          size_(1),
          leftChild_(nullptr),
          rightChild_(nullptr) {}
};

/**
 * @brief Self-balancing AVL (Adelson-Velsky and Landis) Binary Search Tree.
 *
 * Guarantees O(log N) worst-case time complexity for search, insertion, and deletion.
 * In addition to standard BST operations, this implementation augments each node with
 * its subtree size to provide O(log N) order statistics:
 * - findKth(rank): Retrieves the k-th smallest element.
 * - findRank(key): Computes the 1-based rank of a given key.
 *
 * @tparam KeyType The type of keys stored in the tree. Must be comparable using Compare.
 * @tparam Compare Comparator functor conforming to strict weak ordering (default: std::less<KeyType>).
 */
template <typename KeyType, typename Compare = std::less<KeyType>>
class AvlTree {
public:
    using ValueType = KeyType;
    using SizeType = std::size_t;

    /**
     * @brief Constructs an empty AVL tree.
     */
    AvlTree() : root_(nullptr), compare_(Compare()) {}

    /**
     * @brief Constructs an empty AVL tree with a custom comparator.
     */
    explicit AvlTree(const Compare& compare) : root_(nullptr), compare_(compare) {}

    /**
     * @brief Constructs an AVL tree initialized with elements from an initializer list.
     */
    AvlTree(std::initializer_list<KeyType> initialElements, const Compare& compare = Compare())
        : root_(nullptr), compare_(compare) {
        for (const auto& element : initialElements) {
            insert(element);
        }
    }

    /**
     * @brief Destructor. Releases all allocated nodes.
     */
    ~AvlTree() {
        clear();
    }

    /**
     * @brief Copy constructor. Deep copies all nodes from the other tree.
     */
    AvlTree(const AvlTree& other)
        : root_(copySubtree(other.root_)), compare_(other.compare_) {}

    /**
     * @brief Copy assignment operator.
     */
    AvlTree& operator=(const AvlTree& other) {
        if (this != &other) {
            clear();
            root_ = copySubtree(other.root_);
            compare_ = other.compare_;
        }
        return *this;
    }

    /**
     * @brief Move constructor. Takes ownership of nodes from other tree.
     */
    AvlTree(AvlTree&& other) noexcept
        : root_(other.root_), compare_(std::move(other.compare_)) {
        other.root_ = nullptr;
    }

    /**
     * @brief Move assignment operator.
     */
    AvlTree& operator=(AvlTree&& other) noexcept {
        if (this != &other) {
            clear();
            root_ = other.root_;
            compare_ = std::move(other.compare_);
            other.root_ = nullptr;
        }
        return *this;
    }

    /**
     * @brief Inserts a key into the AVL tree.
     *
     * If the key already exists, the insertion is ignored (set semantics).
     * Rebalances the tree along the insertion path to maintain the AVL invariant.
     *
     * @param key The key to insert.
     * @return true if the key was successfully inserted; false if it already existed.
     * @note Time complexity: O(log N).
     */
    bool insert(const KeyType& key) {
        bool wasInserted = false;
        root_ = insertHelper(root_, key, wasInserted);
        return wasInserted;
    }

    /**
     * @brief Inserts an rvalue key into the AVL tree.
     */
    bool insert(KeyType&& key) {
        bool wasInserted = false;
        root_ = insertHelper(root_, std::move(key), wasInserted);
        return wasInserted;
    }

    /**
     * @brief Removes a key from the AVL tree if it exists.
     *
     * Rebalances the tree along the deletion path to maintain the AVL invariant.
     *
     * @param key The key to remove.
     * @return true if the key was found and removed; false otherwise.
     * @note Time complexity: O(log N).
     */
    bool remove(const KeyType& key) {
        bool wasRemoved = false;
        root_ = removeHelper(root_, key, wasRemoved);
        return wasRemoved;
    }

    /**
     * @brief Checks if a key exists in the tree.
     *
     * @param key The key to search for.
     * @return true if the key exists, false otherwise.
     * @note Time complexity: O(log N).
     */
    [[nodiscard]] bool contains(const KeyType& key) const {
        return search(key) != nullptr;
    }

    /**
     * @brief Searches for a key and returns a pointer to the stored key if found.
     *
     * @param key The key to search for.
     * @return Pointer to the found key, or nullptr if not present.
     * @note Time complexity: O(log N).
     */
    [[nodiscard]] const KeyType* search(const KeyType& key) const {
        const AvlNode<KeyType>* currentNode = root_;
        while (currentNode != nullptr) {
            if (compare_(key, currentNode->key_)) {
                currentNode = currentNode->leftChild_;
            } else if (compare_(currentNode->key_, key)) {
                currentNode = currentNode->rightChild_;
            } else {
                return &(currentNode->key_);
            }
        }
        return nullptr;
    }

    /**
     * @brief Returns the total number of keys in the tree.
     */
    [[nodiscard]] SizeType getSize() const noexcept {
        return getNodeSize(root_);
    }

    /**
     * @brief Returns the height of the tree. An empty tree has height 0.
     */
    [[nodiscard]] int getHeight() const noexcept {
        return getNodeHeight(root_);
    }

    /**
     * @brief Checks whether the tree is empty.
     */
    [[nodiscard]] bool isEmpty() const noexcept {
        return root_ == nullptr;
    }

    /**
     * @brief Removes all keys and frees all nodes in the tree.
     */
    void clear() noexcept {
        destroySubtree(root_);
        root_ = nullptr;
    }

    /**
     * @brief Returns the minimum key stored in the tree, if not empty.
     */
    [[nodiscard]] std::optional<KeyType> getMinimum() const {
        if (root_ == nullptr) {
            return std::nullopt;
        }
        const AvlNode<KeyType>* currentNode = root_;
        while (currentNode->leftChild_ != nullptr) {
            currentNode = currentNode->leftChild_;
        }
        return currentNode->key_;
    }

    /**
     * @brief Returns the maximum key stored in the tree, if not empty.
     */
    [[nodiscard]] std::optional<KeyType> getMaximum() const {
        if (root_ == nullptr) {
            return std::nullopt;
        }
        const AvlNode<KeyType>* currentNode = root_;
        while (currentNode->rightChild_ != nullptr) {
            currentNode = currentNode->rightChild_;
        }
        return currentNode->key_;
    }

    /**
     * @brief Finds the k-th smallest element (1-based index).
     *
     * Example: findKth(1) returns the minimum element.
     *
     * @param rank The 1-based rank (must be between 1 and getSize()).
     * @return std::optional containing the key, or std::nullopt if rank is out of bounds.
     * @note Time complexity: O(log N).
     */
    [[nodiscard]] std::optional<KeyType> findKth(SizeType rank) const {
        if (rank < 1 || rank > getSize()) {
            return std::nullopt;
        }
        const AvlNode<KeyType>* currentNode = root_;
        while (currentNode != nullptr) {
            SizeType leftSubtreeSize = getNodeSize(currentNode->leftChild_);
            if (rank <= leftSubtreeSize) {
                currentNode = currentNode->leftChild_;
            } else if (rank == leftSubtreeSize + 1) {
                return currentNode->key_;
            } else {
                rank -= (leftSubtreeSize + 1);
                currentNode = currentNode->rightChild_;
            }
        }
        return std::nullopt;
    }

    /**
     * @brief Computes the 1-based rank of the given key in the sorted order.
     *
     * @param key The key whose rank is sought.
     * @return 1-based rank if the key is present, or 0 if the key is not in the tree.
     * @note Time complexity: O(log N).
     */
    [[nodiscard]] SizeType findRank(const KeyType& key) const {
        const AvlNode<KeyType>* currentNode = root_;
        SizeType accumulatedRank = 0;
        while (currentNode != nullptr) {
            if (compare_(key, currentNode->key_)) {
                currentNode = currentNode->leftChild_;
            } else if (compare_(currentNode->key_, key)) {
                accumulatedRank += getNodeSize(currentNode->leftChild_) + 1;
                currentNode = currentNode->rightChild_;
            } else {
                return accumulatedRank + getNodeSize(currentNode->leftChild_) + 1;
            }
        }
        return 0; // Not found
    }

    /**
     * @brief Performs an in-order traversal, calling the visitor function for each key.
     */
    void inOrderTraversal(const std::function<void(const KeyType&)>& visitor) const {
        inOrderTraversalHelper(root_, visitor);
    }

    /**
     * @brief Converts the tree's contents to a sorted std::vector.
     */
    [[nodiscard]] std::vector<KeyType> toInOrderVector() const {
        std::vector<KeyType> result;
        result.reserve(getSize());
        inOrderTraversal([&result](const KeyType& key) {
            result.push_back(key);
        });
        return result;
    }

    /**
     * @brief Validates AVL invariants, BST order, subtree sizes, and height properties.
     *
     * Useful for automated unit testing and correctness verification.
     * @return true if all invariants hold; false otherwise.
     */
    [[nodiscard]] bool verifyProperties() const {
        bool isValid = true;
        verifySubtree(root_, nullptr, nullptr, isValid);
        return isValid;
    }

    /**
     * @brief Prints a visual representation of the tree structure to an output stream.
     */
    void printTree(std::ostream& outputStream = std::cout) const {
        if (root_ == nullptr) {
            outputStream << "(empty tree)\n";
            return;
        }
        printTreeHelper(root_, "", true, outputStream);
    }

private:
    AvlNode<KeyType>* root_;
    Compare compare_;

    static int getNodeHeight(const AvlNode<KeyType>* node) noexcept {
        return node == nullptr ? 0 : node->height_;
    }

    static SizeType getNodeSize(const AvlNode<KeyType>* node) noexcept {
        return node == nullptr ? 0 : node->size_;
    }

    static int getBalanceFactor(const AvlNode<KeyType>* node) noexcept {
        return node == nullptr ? 0 : getNodeHeight(node->leftChild_) - getNodeHeight(node->rightChild_);
    }

    static void updateNodeMetrics(AvlNode<KeyType>* node) noexcept {
        if (node != nullptr) {
            node->height_ = 1 + std::max(getNodeHeight(node->leftChild_), getNodeHeight(node->rightChild_));
            node->size_ = 1 + getNodeSize(node->leftChild_) + getNodeSize(node->rightChild_);
        }
    }

    /**
     * @brief Performs a right rotation around the given node.
     *
     *       y                               x
     *      / \     Right Rotation          / \
     *     x   T3   -------------->        T1  y
     *    / \                                 / \
     *   T1  T2                              T2  T3
     */
    AvlNode<KeyType>* rotateRight(AvlNode<KeyType>* y) {
        AvlNode<KeyType>* x = y->leftChild_;
        AvlNode<KeyType>* t2 = x->rightChild_;

        x->rightChild_ = y;
        y->leftChild_ = t2;

        updateNodeMetrics(y);
        updateNodeMetrics(x);

        return x;
    }

    /**
     * @brief Performs a left rotation around the given node.
     *
     *     x                                   y
     *    / \        Left Rotation            / \
     *   T1  y      -------------->          x   T3
     *      / \                             / \
     *     T2  T3                          T1  T2
     */
    AvlNode<KeyType>* rotateLeft(AvlNode<KeyType>* x) {
        AvlNode<KeyType>* y = x->rightChild_;
        AvlNode<KeyType>* t2 = y->leftChild_;

        y->leftChild_ = x;
        x->rightChild_ = t2;

        updateNodeMetrics(x);
        updateNodeMetrics(y);

        return y;
    }

    /**
     * @brief Rebalances an AVL subtree if the balance factor exceeds [-1, 1].
     */
    AvlNode<KeyType>* rebalance(AvlNode<KeyType>* node) {
        if (node == nullptr) {
            return nullptr;
        }

        updateNodeMetrics(node);
        int balanceFactor = getBalanceFactor(node);

        // Case 1: Left-Heavy
        if (balanceFactor > 1) {
            // Left-Right Case: rotate left on left child first
            if (getBalanceFactor(node->leftChild_) < 0) {
                node->leftChild_ = rotateLeft(node->leftChild_);
            }
            // Left-Left Case
            return rotateRight(node);
        }

        // Case 2: Right-Heavy
        if (balanceFactor < -1) {
            // Right-Left Case: rotate right on right child first
            if (getBalanceFactor(node->rightChild_) > 0) {
                node->rightChild_ = rotateRight(node->rightChild_);
            }
            // Right-Right Case
            return rotateLeft(node);
        }

        return node;
    }

    template <typename ForwardKey>
    AvlNode<KeyType>* insertHelper(AvlNode<KeyType>* node, ForwardKey&& key, bool& wasInserted) {
        if (node == nullptr) {
            wasInserted = true;
            return new AvlNode<KeyType>(std::forward<ForwardKey>(key));
        }

        if (compare_(key, node->key_)) {
            node->leftChild_ = insertHelper(node->leftChild_, std::forward<ForwardKey>(key), wasInserted);
        } else if (compare_(node->key_, key)) {
            node->rightChild_ = insertHelper(node->rightChild_, std::forward<ForwardKey>(key), wasInserted);
        } else {
            // Duplicate key: ignore insertion
            wasInserted = false;
            return node;
        }

        return rebalance(node);
    }

    AvlNode<KeyType>* removeHelper(AvlNode<KeyType>* node, const KeyType& key, bool& wasRemoved) {
        if (node == nullptr) {
            wasRemoved = false;
            return nullptr;
        }

        if (compare_(key, node->key_)) {
            node->leftChild_ = removeHelper(node->leftChild_, key, wasRemoved);
        } else if (compare_(node->key_, key)) {
            node->rightChild_ = removeHelper(node->rightChild_, key, wasRemoved);
        } else {
            wasRemoved = true;

            // Node with only one child or no child
            if (node->leftChild_ == nullptr || node->rightChild_ == nullptr) {
                AvlNode<KeyType>* replacement = (node->leftChild_ != nullptr) ? node->leftChild_ : node->rightChild_;

                if (replacement == nullptr) {
                    // No child case
                    delete node;
                    return nullptr;
                } else {
                    // One child case
                    *node = *replacement; // Copy contents
                    // Free the redundant child node
                    delete replacement;
                }
            } else {
                // Node with two children: replace with in-order successor (minimum in right subtree)
                AvlNode<KeyType>* successor = findMinimumNode(node->rightChild_);
                node->key_ = successor->key_;
                bool dummyRemoved = false;
                node->rightChild_ = removeHelper(node->rightChild_, successor->key_, dummyRemoved);
            }
        }

        return rebalance(node);
    }

    static AvlNode<KeyType>* findMinimumNode(AvlNode<KeyType>* node) noexcept {
        AvlNode<KeyType>* current = node;
        while (current != nullptr && current->leftChild_ != nullptr) {
            current = current->leftChild_;
        }
        return current;
    }

    static void destroySubtree(AvlNode<KeyType>* node) noexcept {
        if (node != nullptr) {
            destroySubtree(node->leftChild_);
            destroySubtree(node->rightChild_);
            delete node;
        }
    }

    static AvlNode<KeyType>* copySubtree(const AvlNode<KeyType>* node) {
        if (node == nullptr) {
            return nullptr;
        }
        auto* newNode = new AvlNode<KeyType>(node->key_);
        newNode->height_ = node->height_;
        newNode->size_ = node->size_;
        newNode->leftChild_ = copySubtree(node->leftChild_);
        newNode->rightChild_ = copySubtree(node->rightChild_);
        return newNode;
    }

    void inOrderTraversalHelper(const AvlNode<KeyType>* node,
                                const std::function<void(const KeyType&)>& visitor) const {
        if (node != nullptr) {
            inOrderTraversalHelper(node->leftChild_, visitor);
            visitor(node->key_);
            inOrderTraversalHelper(node->rightChild_, visitor);
        }
    }

    int verifySubtree(const AvlNode<KeyType>* node,
                      const KeyType* minKey,
                      const KeyType* maxKey,
                      bool& isValid) const {
        if (node == nullptr) {
            return 0;
        }

        // Check BST ordering
        if (minKey != nullptr && !compare_(*minKey, node->key_)) {
            isValid = false;
        }
        if (maxKey != nullptr && !compare_(node->key_, *maxKey)) {
            isValid = false;
        }

        int leftHeight = verifySubtree(node->leftChild_, minKey, &(node->key_), isValid);
        int rightHeight = verifySubtree(node->rightChild_, &(node->key_), maxKey, isValid);

        // Check AVL balance invariant
        int balanceFactor = leftHeight - rightHeight;
        if (balanceFactor < -1 || balanceFactor > 1) {
            isValid = false;
        }

        // Check height metadata
        int expectedHeight = 1 + std::max(leftHeight, rightHeight);
        if (node->height_ != expectedHeight) {
            isValid = false;
        }

        // Check size metadata
        SizeType expectedSize = 1 + getNodeSize(node->leftChild_) + getNodeSize(node->rightChild_);
        if (node->size_ != expectedSize) {
            isValid = false;
        }

        return expectedHeight;
    }

    void printTreeHelper(const AvlNode<KeyType>* node,
                         const std::string& prefix,
                         bool isTail,
                         std::ostream& outputStream) const {
        if (node == nullptr) {
            return;
        }

        outputStream << prefix << (isTail ? "└── " : "├── ")
                     << node->key_ << " [h=" << node->height_ << ", sz=" << node->size_ << "]\n";

        std::string childPrefix = prefix + (isTail ? "    " : "│   ");
        if (node->leftChild_ != nullptr || node->rightChild_ != nullptr) {
            if (node->leftChild_ != nullptr) {
                printTreeHelper(node->leftChild_, childPrefix, node->rightChild_ == nullptr, outputStream);
            }
            if (node->rightChild_ != nullptr) {
                printTreeHelper(node->rightChild_, childPrefix, true, outputStream);
            }
        }
    }
};

} // namespace search_trees

#endif // SEARCH_TREES_AVL_TREE_HPP
