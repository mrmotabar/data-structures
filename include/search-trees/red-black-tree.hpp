#ifndef SEARCH_TREES_RED_BLACK_TREE_HPP
#define SEARCH_TREES_RED_BLACK_TREE_HPP

#include <algorithm>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace search_trees {

/**
 * @brief Node color representation for the Red-Black Tree.
 */
enum class NodeColor : bool {
    Red = false,
    Black = true
};

/**
 * @brief Internal node representation for the Red-Black Tree.
 */
template <typename KeyType>
class RedBlackNode {
public:
    KeyType key_;
    NodeColor color_;
    RedBlackNode* leftChild_;
    RedBlackNode* rightChild_;
    RedBlackNode* parent_;

    RedBlackNode()
        : key_(KeyType{}),
          color_(NodeColor::Black),
          leftChild_(nullptr),
          rightChild_(nullptr),
          parent_(nullptr) {}

    explicit RedBlackNode(const KeyType& key,
                          NodeColor color = NodeColor::Red,
                          RedBlackNode* nilNode = nullptr)
        : key_(key),
          color_(color),
          leftChild_(nilNode),
          rightChild_(nilNode),
          parent_(nilNode) {}

    explicit RedBlackNode(KeyType&& key,
                          NodeColor color = NodeColor::Red,
                          RedBlackNode* nilNode = nullptr)
        : key_(std::move(key)),
          color_(color),
          leftChild_(nilNode),
          rightChild_(nilNode),
          parent_(nilNode) {}

    [[nodiscard]] bool isRed() const noexcept {
        return color_ == NodeColor::Red;
    }

    [[nodiscard]] bool isBlack() const noexcept {
        return color_ == NodeColor::Black;
    }
};

/**
 * @brief Self-balancing Red-Black Binary Search Tree.
 *
 * Guarantees O(log N) worst-case time complexity for search, insertion, and deletion.
 * Enforces the five fundamental Red-Black invariants:
 * 1. Every node is either Red or Black.
 * 2. The root node is always Black.
 * 3. Every leaf (NIL sentinel) is Black.
 * 4. If a node is Red, then both its children are Black (no two Red nodes in a row).
 * 5. Every path from any node to its descendant NIL leaves contains the same number of Black nodes.
 *
 * @tparam KeyType The type of keys stored in the tree.
 * @tparam Compare Comparator functor conforming to strict weak ordering (default: std::less<KeyType>).
 */
template <typename KeyType, typename Compare = std::less<KeyType>>
class RedBlackTree {
public:
    using ValueType = KeyType;
    using SizeType = std::size_t;

    /**
     * @brief Constructs an empty Red-Black tree.
     */
    RedBlackTree()
        : nilNode_(createNilNode()),
          root_(nilNode_),
          size_(0),
          compare_(Compare()) {}

    /**
     * @brief Constructs an empty Red-Black tree with a custom comparator.
     */
    explicit RedBlackTree(const Compare& compare)
        : nilNode_(createNilNode()),
          root_(nilNode_),
          size_(0),
          compare_(compare) {}

    /**
     * @brief Constructs a Red-Black tree initialized with elements from an initializer list.
     */
    RedBlackTree(std::initializer_list<KeyType> initialElements, const Compare& compare = Compare())
        : nilNode_(createNilNode()),
          root_(nilNode_),
          size_(0),
          compare_(compare) {
        for (const auto& element : initialElements) {
            insert(element);
        }
    }

    /**
     * @brief Destructor. Releases all allocated nodes including the sentinel.
     */
    ~RedBlackTree() {
        clear();
        delete nilNode_;
    }

    /**
     * @brief Copy constructor. Deep copies all nodes.
     */
    RedBlackTree(const RedBlackTree& other)
        : nilNode_(createNilNode()),
          root_(nilNode_),
          size_(other.size_),
          compare_(other.compare_) {
        root_ = copySubtree(other.root_, other.nilNode_, nilNode_);
    }

    /**
     * @brief Copy assignment operator.
     */
    RedBlackTree& operator=(const RedBlackTree& other) {
        if (this != &other) {
            clear();
            root_ = copySubtree(other.root_, other.nilNode_, nilNode_);
            size_ = other.size_;
            compare_ = other.compare_;
        }
        return *this;
    }

    /**
     * @brief Move constructor.
     */
    RedBlackTree(RedBlackTree&& other) noexcept
        : nilNode_(other.nilNode_),
          root_(other.root_),
          size_(other.size_),
          compare_(std::move(other.compare_)) {
        other.nilNode_ = other.createNilNode();
        other.root_ = other.nilNode_;
        other.size_ = 0;
    }

    /**
     * @brief Move assignment operator.
     */
    RedBlackTree& operator=(RedBlackTree&& other) noexcept {
        if (this != &other) {
            clear();
            delete nilNode_;

            nilNode_ = other.nilNode_;
            root_ = other.root_;
            size_ = other.size_;
            compare_ = std::move(other.compare_);

            other.nilNode_ = other.createNilNode();
            other.root_ = other.nilNode_;
            other.size_ = 0;
        }
        return *this;
    }

    /**
     * @brief Inserts a key into the Red-Black tree.
     *
     * If the key already exists, insertion is ignored (set semantics).
     * Maintains Red-Black invariants using recoloring and rotations.
     *
     * @param key The key to insert.
     * @return true if inserted, false if the key already existed.
     * @note Time complexity: O(log N).
     */
    bool insert(const KeyType& key) {
        return insertInternal(key);
    }

    /**
     * @brief Inserts an rvalue key into the Red-Black tree.
     */
    bool insert(KeyType&& key) {
        return insertInternal(std::move(key));
    }

    /**
     * @brief Removes a key from the Red-Black tree if present.
     *
     * Maintains Red-Black invariants through deletion fixup (resolving double-black nodes).
     *
     * @param key The key to remove.
     * @return true if found and removed; false otherwise.
     * @note Time complexity: O(log N).
     */
    bool remove(const KeyType& key) {
        RedBlackNode<KeyType>* targetNode = findNode(key);
        if (targetNode == nilNode_) {
            return false;
        }
        deleteNode(targetNode);
        --size_;
        return true;
    }

    /**
     * @brief Checks if a key exists in the tree.
     */
    [[nodiscard]] bool contains(const KeyType& key) const {
        return findNode(key) != nilNode_;
    }

    /**
     * @brief Searches for a key and returns a pointer to the stored key if found.
     */
    [[nodiscard]] const KeyType* search(const KeyType& key) const {
        const RedBlackNode<KeyType>* node = findNode(key);
        return node == nilNode_ ? nullptr : &(node->key_);
    }

    /**
     * @brief Returns the total number of keys stored in the tree.
     */
    [[nodiscard]] SizeType getSize() const noexcept {
        return size_;
    }

    /**
     * @brief Checks if the tree contains zero keys.
     */
    [[nodiscard]] bool isEmpty() const noexcept {
        return root_ == nilNode_;
    }

    /**
     * @brief Clears all keys from the tree, freeing memory.
     */
    void clear() noexcept {
        destroySubtree(root_);
        root_ = nilNode_;
        size_ = 0;
    }

    /**
     * @brief Returns the minimum key stored in the tree.
     */
    [[nodiscard]] std::optional<KeyType> getMinimum() const {
        if (root_ == nilNode_) {
            return std::nullopt;
        }
        return getMinimumSubtreeNode(root_)->key_;
    }

    /**
     * @brief Returns the maximum key stored in the tree.
     */
    [[nodiscard]] std::optional<KeyType> getMaximum() const {
        if (root_ == nilNode_) {
            return std::nullopt;
        }
        return getMaximumSubtreeNode(root_)->key_;
    }

    /**
     * @brief Performs an in-order traversal, calling the visitor function for each key.
     */
    void inOrderTraversal(const std::function<void(const KeyType&)>& visitor) const {
        inOrderTraversalHelper(root_, visitor);
    }

    /**
     * @brief Returns a sorted std::vector containing all keys in the tree.
     */
    [[nodiscard]] std::vector<KeyType> toInOrderVector() const {
        std::vector<KeyType> result;
        result.reserve(size_);
        inOrderTraversal([&result](const KeyType& key) {
            result.push_back(key);
        });
        return result;
    }

    /**
     * @brief Computes the black-height of the tree.
     */
    [[nodiscard]] int getBlackHeight() const noexcept {
        int blackHeight = 0;
        const RedBlackNode<KeyType>* currentNode = root_;
        while (currentNode != nilNode_) {
            if (currentNode->isBlack()) {
                ++blackHeight;
            }
            currentNode = currentNode->leftChild_;
        }
        return blackHeight;
    }

    /**
     * @brief Rigorously verifies all Red-Black tree invariants.
     *
     * Checks:
     * 1. Root is black.
     * 2. No red node has a red child (no consecutive red nodes).
     * 3. Every path from root to leaf has identical black-height.
     * 4. Strict BST ordering.
     */
    [[nodiscard]] bool verifyProperties() const {
        if (root_ == nilNode_) {
            return size_ == 0;
        }

        // Invariant 2: Root must be black
        if (root_->isRed()) {
            return false;
        }

        int blackHeight = -1;
        bool isValid = true;
        verifySubtree(root_, nullptr, nullptr, 0, blackHeight, isValid);
        return isValid;
    }

    /**
     * @brief Prints a visual representation of the Red-Black tree structure.
     */
    void printTree(std::ostream& outputStream = std::cout) const {
        if (root_ == nilNode_) {
            outputStream << "(empty tree)\n";
            return;
        }
        printTreeHelper(root_, "", true, outputStream);
    }

private:
    RedBlackNode<KeyType>* nilNode_;
    RedBlackNode<KeyType>* root_;
    SizeType size_;
    Compare compare_;

    static RedBlackNode<KeyType>* createNilNode() {
        auto* nil = new RedBlackNode<KeyType>();
        nil->color_ = NodeColor::Black;
        nil->leftChild_ = nil;
        nil->rightChild_ = nil;
        nil->parent_ = nil;
        return nil;
    }

    template <typename ForwardKey>
    bool insertInternal(ForwardKey&& key) {
        RedBlackNode<KeyType>* parentNode = nilNode_;
        RedBlackNode<KeyType>* currentNode = root_;

        while (currentNode != nilNode_) {
            parentNode = currentNode;
            if (compare_(key, currentNode->key_)) {
                currentNode = currentNode->leftChild_;
            } else if (compare_(currentNode->key_, key)) {
                currentNode = currentNode->rightChild_;
            } else {
                return false; // Key already present
            }
        }

        auto* newNode = new RedBlackNode<KeyType>(std::forward<ForwardKey>(key),
                                                 NodeColor::Red,
                                                 nilNode_);
        newNode->parent_ = parentNode;

        if (parentNode == nilNode_) {
            root_ = newNode;
        } else if (compare_(newNode->key_, parentNode->key_)) {
            parentNode->leftChild_ = newNode;
        } else {
            parentNode->rightChild_ = newNode;
        }

        fixAfterInsertion(newNode);
        ++size_;
        return true;
    }

    /**
     * @brief CLRS Insertion Fixup Algorithm.
     */
    void fixAfterInsertion(RedBlackNode<KeyType>* node) {
        while (node->parent_->isRed()) {
            if (node->parent_ == node->parent_->parent_->leftChild_) {
                RedBlackNode<KeyType>* uncle = node->parent_->parent_->rightChild_;
                if (uncle->isRed()) {
                    // Case 1: Uncle is Red
                    node->parent_->color_ = NodeColor::Black;
                    uncle->color_ = NodeColor::Black;
                    node->parent_->parent_->color_ = NodeColor::Red;
                    node = node->parent_->parent_;
                } else {
                    if (node == node->parent_->rightChild_) {
                        // Case 2: Triangle -> Left rotate on parent
                        node = node->parent_;
                        rotateLeft(node);
                    }
                    // Case 3: Line -> Right rotate on grandparent
                    node->parent_->color_ = NodeColor::Black;
                    node->parent_->parent_->color_ = NodeColor::Red;
                    rotateRight(node->parent_->parent_);
                }
            } else {
                // Symmetric case: parent is right child of grandparent
                RedBlackNode<KeyType>* uncle = node->parent_->parent_->leftChild_;
                if (uncle->isRed()) {
                    // Case 1: Uncle is Red
                    node->parent_->color_ = NodeColor::Black;
                    uncle->color_ = NodeColor::Black;
                    node->parent_->parent_->color_ = NodeColor::Red;
                    node = node->parent_->parent_;
                } else {
                    if (node == node->parent_->leftChild_) {
                        // Case 2: Triangle -> Right rotate on parent
                        node = node->parent_;
                        rotateRight(node);
                    }
                    // Case 3: Line -> Left rotate on grandparent
                    node->parent_->color_ = NodeColor::Black;
                    node->parent_->parent_->color_ = NodeColor::Red;
                    rotateLeft(node->parent_->parent_);
                }
            }
        }
        root_->color_ = NodeColor::Black;
    }

    /**
     * @brief CLRS Deletion Algorithm.
     */
    void deleteNode(RedBlackNode<KeyType>* z) {
        RedBlackNode<KeyType>* y = z;
        RedBlackNode<KeyType>* x = nilNode_;
        NodeColor yOriginalColor = y->color_;

        if (z->leftChild_ == nilNode_) {
            x = z->rightChild_;
            transplant(z, z->rightChild_);
        } else if (z->rightChild_ == nilNode_) {
            x = z->leftChild_;
            transplant(z, z->leftChild_);
        } else {
            y = getMinimumSubtreeNode(z->rightChild_);
            yOriginalColor = y->color_;
            x = y->rightChild_;

            if (y->parent_ == z) {
                x->parent_ = y;
            } else {
                transplant(y, y->rightChild_);
                y->rightChild_ = z->rightChild_;
                y->rightChild_->parent_ = y;
            }

            transplant(z, y);
            y->leftChild_ = z->leftChild_;
            y->leftChild_->parent_ = y;
            y->color_ = z->color_;
        }

        delete z;

        if (yOriginalColor == NodeColor::Black) {
            fixAfterDeletion(x);
        }
    }

    /**
     * @brief CLRS Deletion Fixup Algorithm.
     */
    void fixAfterDeletion(RedBlackNode<KeyType>* x) {
        while (x != root_ && x->isBlack()) {
            if (x == x->parent_->leftChild_) {
                RedBlackNode<KeyType>* sibling = x->parent_->rightChild_;
                if (sibling->isRed()) {
                    // Case 1: Sibling is Red
                    sibling->color_ = NodeColor::Black;
                    x->parent_->color_ = NodeColor::Red;
                    rotateLeft(x->parent_);
                    sibling = x->parent_->rightChild_;
                }
                if (sibling->leftChild_->isBlack() && sibling->rightChild_->isBlack()) {
                    // Case 2: Both sibling's children are Black
                    sibling->color_ = NodeColor::Red;
                    x = x->parent_;
                } else {
                    if (sibling->rightChild_->isBlack()) {
                        // Case 3: Sibling's right child is Black, left child is Red
                        sibling->leftChild_->color_ = NodeColor::Black;
                        sibling->color_ = NodeColor::Red;
                        rotateRight(sibling);
                        sibling = x->parent_->rightChild_;
                    }
                    // Case 4: Sibling's right child is Red
                    sibling->color_ = x->parent_->color_;
                    x->parent_->color_ = NodeColor::Black;
                    sibling->rightChild_->color_ = NodeColor::Black;
                    rotateLeft(x->parent_);
                    x = root_;
                }
            } else {
                // Symmetric case: x is right child
                RedBlackNode<KeyType>* sibling = x->parent_->leftChild_;
                if (sibling->isRed()) {
                    // Case 1: Sibling is Red
                    sibling->color_ = NodeColor::Black;
                    x->parent_->color_ = NodeColor::Red;
                    rotateRight(x->parent_);
                    sibling = x->parent_->leftChild_;
                }
                if (sibling->rightChild_->isBlack() && sibling->leftChild_->isBlack()) {
                    // Case 2: Both sibling's children are Black
                    sibling->color_ = NodeColor::Red;
                    x = x->parent_;
                } else {
                    if (sibling->leftChild_->isBlack()) {
                        // Case 3: Sibling's left child is Black, right child is Red
                        sibling->rightChild_->color_ = NodeColor::Black;
                        sibling->color_ = NodeColor::Red;
                        rotateLeft(sibling);
                        sibling = x->parent_->leftChild_;
                    }
                    // Case 4: Sibling's left child is Red
                    sibling->color_ = x->parent_->color_;
                    x->parent_->color_ = NodeColor::Black;
                    sibling->leftChild_->color_ = NodeColor::Black;
                    rotateRight(x->parent_);
                    x = root_;
                }
            }
        }
        x->color_ = NodeColor::Black;
    }

    void transplant(RedBlackNode<KeyType>* sourceNode, RedBlackNode<KeyType>* targetNode) {
        if (sourceNode->parent_ == nilNode_) {
            root_ = targetNode;
        } else if (sourceNode == sourceNode->parent_->leftChild_) {
            sourceNode->parent_->leftChild_ = targetNode;
        } else {
            sourceNode->parent_->rightChild_ = targetNode;
        }
        targetNode->parent_ = sourceNode->parent_;
    }

    void rotateLeft(RedBlackNode<KeyType>* pivot) {
        RedBlackNode<KeyType>* rightChild = pivot->rightChild_;
        pivot->rightChild_ = rightChild->leftChild_;

        if (rightChild->leftChild_ != nilNode_) {
            rightChild->leftChild_->parent_ = pivot;
        }

        rightChild->parent_ = pivot->parent_;
        if (pivot->parent_ == nilNode_) {
            root_ = rightChild;
        } else if (pivot == pivot->parent_->leftChild_) {
            pivot->parent_->leftChild_ = rightChild;
        } else {
            pivot->parent_->rightChild_ = rightChild;
        }

        rightChild->leftChild_ = pivot;
        pivot->parent_ = rightChild;
    }

    void rotateRight(RedBlackNode<KeyType>* pivot) {
        RedBlackNode<KeyType>* leftChild = pivot->leftChild_;
        pivot->leftChild_ = leftChild->rightChild_;

        if (leftChild->rightChild_ != nilNode_) {
            leftChild->rightChild_->parent_ = pivot;
        }

        leftChild->parent_ = pivot->parent_;
        if (pivot->parent_ == nilNode_) {
            root_ = leftChild;
        } else if (pivot == pivot->parent_->rightChild_) {
            pivot->parent_->rightChild_ = leftChild;
        } else {
            pivot->parent_->leftChild_ = leftChild;
        }

        leftChild->rightChild_ = pivot;
        pivot->parent_ = leftChild;
    }

    [[nodiscard]] RedBlackNode<KeyType>* findNode(const KeyType& key) const {
        RedBlackNode<KeyType>* currentNode = root_;
        while (currentNode != nilNode_) {
            if (compare_(key, currentNode->key_)) {
                currentNode = currentNode->leftChild_;
            } else if (compare_(currentNode->key_, key)) {
                currentNode = currentNode->rightChild_;
            } else {
                return currentNode;
            }
        }
        return nilNode_;
    }

    RedBlackNode<KeyType>* getMinimumSubtreeNode(RedBlackNode<KeyType>* node) const noexcept {
        while (node != nilNode_ && node->leftChild_ != nilNode_) {
            node = node->leftChild_;
        }
        return node;
    }

    RedBlackNode<KeyType>* getMaximumSubtreeNode(RedBlackNode<KeyType>* node) const noexcept {
        while (node != nilNode_ && node->rightChild_ != nilNode_) {
            node = node->rightChild_;
        }
        return node;
    }

    void destroySubtree(RedBlackNode<KeyType>* node) noexcept {
        if (node != nilNode_) {
            destroySubtree(node->leftChild_);
            destroySubtree(node->rightChild_);
            delete node;
        }
    }

    RedBlackNode<KeyType>* copySubtree(const RedBlackNode<KeyType>* sourceNode,
                                        const RedBlackNode<KeyType>* sourceNil,
                                        RedBlackNode<KeyType>* parent) {
        if (sourceNode == sourceNil) {
            return nilNode_;
        }
        auto* newNode = new RedBlackNode<KeyType>(sourceNode->key_, sourceNode->color_, nilNode_);
        newNode->parent_ = parent;
        newNode->leftChild_ = copySubtree(sourceNode->leftChild_, sourceNil, newNode);
        newNode->rightChild_ = copySubtree(sourceNode->rightChild_, sourceNil, newNode);
        return newNode;
    }

    void inOrderTraversalHelper(const RedBlackNode<KeyType>* node,
                                const std::function<void(const KeyType&)>& visitor) const {
        if (node != nilNode_) {
            inOrderTraversalHelper(node->leftChild_, visitor);
            visitor(node->key_);
            inOrderTraversalHelper(node->rightChild_, visitor);
        }
    }

    void verifySubtree(const RedBlackNode<KeyType>* node,
                       const KeyType* minKey,
                       const KeyType* maxKey,
                       int currentBlackCount,
                       int& expectedBlackHeight,
                       bool& isValid) const {
        if (node == nilNode_) {
            if (expectedBlackHeight == -1) {
                expectedBlackHeight = currentBlackCount;
            } else if (currentBlackCount != expectedBlackHeight) {
                isValid = false; // Invariant 5 violated
            }
            return;
        }

        // Invariant 4: Red node cannot have red child
        if (node->isRed()) {
            if (node->leftChild_->isRed() || node->rightChild_->isRed()) {
                isValid = false;
            }
        }

        // BST ordering
        if (minKey != nullptr && !compare_(*minKey, node->key_)) {
            isValid = false;
        }
        if (maxKey != nullptr && !compare_(node->key_, *maxKey)) {
            isValid = false;
        }

        int nextBlackCount = currentBlackCount + (node->isBlack() ? 1 : 0);
        verifySubtree(node->leftChild_, minKey, &(node->key_), nextBlackCount, expectedBlackHeight, isValid);
        verifySubtree(node->rightChild_, &(node->key_), maxKey, nextBlackCount, expectedBlackHeight, isValid);
    }

    void printTreeHelper(const RedBlackNode<KeyType>* node,
                         const std::string& prefix,
                         bool isTail,
                         std::ostream& outputStream) const {
        if (node == nilNode_) {
            return;
        }

        outputStream << prefix << (isTail ? "└── " : "├── ")
                     << node->key_ << (node->isRed() ? " [R]" : " [B]") << "\n";

        std::string childPrefix = prefix + (isTail ? "    " : "│   ");
        if (node->leftChild_ != nilNode_ || node->rightChild_ != nilNode_) {
            if (node->leftChild_ != nilNode_) {
                printTreeHelper(node->leftChild_, childPrefix, node->rightChild_ == nilNode_, outputStream);
            }
            if (node->rightChild_ != nilNode_) {
                printTreeHelper(node->rightChild_, childPrefix, true, outputStream);
            }
        }
    }
};

} // namespace search_trees

#endif // SEARCH_TREES_RED_BLACK_TREE_HPP
