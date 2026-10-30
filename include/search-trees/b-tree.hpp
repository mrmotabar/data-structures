#ifndef SEARCH_TREES_B_TREE_HPP
#define SEARCH_TREES_B_TREE_HPP

#include <algorithm>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <optional>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace search_trees {

/**
 * @brief Node representation for a B-Tree of minimum degree t.
 *
 * Each node maintains:
 * - keys_: Vector of sorted keys (size between t-1 and 2t-1, root can have 1 to 2t-1).
 * - children_: Vector of child pointers (size keys_.size() + 1 for internal nodes; 0 for leaves).
 * - isLeaf_: Boolean flag indicating whether this node is a leaf.
 */
template <typename KeyType>
class BTreeNode {
public:
    std::vector<KeyType> keys_;
    std::vector<BTreeNode*> children_;
    bool isLeaf_;

    explicit BTreeNode(bool isLeaf) : isLeaf_(isLeaf) {}

    ~BTreeNode() = default;

    [[nodiscard]] std::size_t getKeyCount() const noexcept {
        return keys_.size();
    }

    [[nodiscard]] std::size_t getChildCount() const noexcept {
        return children_.size();
    }
};

/**
 * @brief Self-balancing B-Tree data structure parameterized by minimum degree t.
 *
 * Properties of B-Tree with minimum degree t (t >= 2):
 * 1. Every node has at most 2t - 1 keys.
 * 2. Every non-root node has at least t - 1 keys.
 * 3. An internal node with k keys has exactly k + 1 children.
 * 4. All leaves are at the exact same depth.
 * 5. Keys within each node are kept in strictly sorted order.
 *
 * Worst-case time complexity:
 * - Search: O(t * log_t N) = O(log N)
 * - Insert: O(t * log_t N) = O(log N)
 * - Delete: O(t * log_t N) = O(log N)
 *
 * @tparam KeyType The type of keys stored.
 * @tparam Compare Comparator functor conforming to strict weak ordering (default: std::less<KeyType>).
 */
template <typename KeyType, typename Compare = std::less<KeyType>>
class BTree {
public:
    using ValueType = KeyType;
    using SizeType = std::size_t;

    static constexpr SizeType DEFAULT_MINIMUM_DEGREE = 2;

    /**
     * @brief Constructs an empty B-Tree with a specified minimum degree.
     *
     * @param minimumDegree The minimum degree t (must be >= 2).
     * @param compare Comparator function.
     * @throws std::invalid_argument if minimumDegree < 2.
     */
    explicit BTree(SizeType minimumDegree = DEFAULT_MINIMUM_DEGREE,
                   const Compare& compare = Compare())
        : root_(nullptr),
          size_(0),
          minimumDegree_(minimumDegree),
          compare_(compare) {
        if (minimumDegree < 2) {
            throw std::invalid_argument("B-Tree minimum degree must be at least 2.");
        }
    }

    /**
     * @brief Constructs a B-Tree from an initializer list.
     */
    BTree(std::initializer_list<KeyType> initialElements,
          SizeType minimumDegree = DEFAULT_MINIMUM_DEGREE,
          const Compare& compare = Compare())
        : root_(nullptr),
          size_(0),
          minimumDegree_(minimumDegree),
          compare_(compare) {
        if (minimumDegree < 2) {
            throw std::invalid_argument("B-Tree minimum degree must be at least 2.");
        }
        for (const auto& element : initialElements) {
            insert(element);
        }
    }

    /**
     * @brief Destructor. Releases all allocated nodes.
     */
    ~BTree() {
        clear();
    }

    /**
     * @brief Copy constructor. Deep copies all nodes.
     */
    BTree(const BTree& other)
        : root_(copySubtree(other.root_)),
          size_(other.size_),
          minimumDegree_(other.minimumDegree_),
          compare_(other.compare_) {}

    /**
     * @brief Copy assignment operator.
     */
    BTree& operator=(const BTree& other) {
        if (this != &other) {
            clear();
            root_ = copySubtree(other.root_);
            size_ = other.size_;
            minimumDegree_ = other.minimumDegree_;
            compare_ = other.compare_;
        }
        return *this;
    }

    /**
     * @brief Move constructor.
     */
    BTree(BTree&& other) noexcept
        : root_(other.root_),
          size_(other.size_),
          minimumDegree_(other.minimumDegree_),
          compare_(std::move(other.compare_)) {
        other.root_ = nullptr;
        other.size_ = 0;
    }

    /**
     * @brief Move assignment operator.
     */
    BTree& operator=(BTree&& other) noexcept {
        if (this != &other) {
            clear();
            root_ = other.root_;
            size_ = other.size_;
            minimumDegree_ = other.minimumDegree_;
            compare_ = std::move(other.compare_);
            other.root_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    /**
     * @brief Inserts a key into the B-Tree.
     *
     * If key is already present, insertion is ignored (set semantics).
     * Uses single-pass proactive splitting of full nodes along the descent path.
     *
     * @param key The key to insert.
     * @return true if inserted, false if the key already existed.
     */
    bool insert(const KeyType& key) {
        if (contains(key)) {
            return false;
        }

        if (root_ == nullptr) {
            root_ = new BTreeNode<KeyType>(true);
            root_->keys_.push_back(key);
            ++size_;
            return true;
        }

        // If root is full, split it and create a new root
        if (root_->keys_.size() == (2 * minimumDegree_ - 1)) {
            auto* newRoot = new BTreeNode<KeyType>(false);
            newRoot->children_.push_back(root_);
            splitChild(newRoot, 0);
            root_ = newRoot;
        }

        insertNonFull(root_, key);
        ++size_;
        return true;
    }

    /**
     * @brief Removes a key from the B-Tree if present.
     *
     * Uses standard top-down deletion with borrowing and merging.
     *
     * @param key The key to remove.
     * @return true if found and removed; false otherwise.
     */
    bool remove(const KeyType& key) {
        if (root_ == nullptr) {
            return false;
        }

        bool wasRemoved = removeFromNode(root_, key);
        if (wasRemoved) {
            --size_;
            // If root has no keys left
            if (root_->keys_.empty()) {
                BTreeNode<KeyType>* oldRoot = root_;
                if (root_->isLeaf_) {
                    root_ = nullptr;
                } else {
                    root_ = root_->children_[0];
                }
                delete oldRoot;
            }
        }
        return wasRemoved;
    }

    /**
     * @brief Checks if a key exists in the B-Tree.
     */
    [[nodiscard]] bool contains(const KeyType& key) const {
        return search(key) != nullptr;
    }

    /**
     * @brief Searches for a key in the B-Tree.
     *
     * @param key The key to search for.
     * @return Pointer to key if found, nullptr otherwise.
     */
    [[nodiscard]] const KeyType* search(const KeyType& key) const {
        return searchHelper(root_, key);
    }

    /**
     * @brief Returns the minimum degree parameter t.
     */
    [[nodiscard]] SizeType getMinimumDegree() const noexcept {
        return minimumDegree_;
    }

    /**
     * @brief Returns the total number of keys in the tree.
     */
    [[nodiscard]] SizeType getSize() const noexcept {
        return size_;
    }

    /**
     * @brief Checks if the B-Tree is empty.
     */
    [[nodiscard]] bool isEmpty() const noexcept {
        return size_ == 0;
    }

    /**
     * @brief Clears all keys and deallocates all nodes.
     */
    void clear() noexcept {
        destroySubtree(root_);
        root_ = nullptr;
        size_ = 0;
    }

    /**
     * @brief Returns the minimum key stored in the B-Tree.
     */
    [[nodiscard]] std::optional<KeyType> getMinimum() const {
        if (root_ == nullptr || root_->keys_.empty()) {
            return std::nullopt;
        }
        const BTreeNode<KeyType>* currentNode = root_;
        while (!currentNode->isLeaf_) {
            currentNode = currentNode->children_.front();
        }
        return currentNode->keys_.front();
    }

    /**
     * @brief Returns the maximum key stored in the B-Tree.
     */
    [[nodiscard]] std::optional<KeyType> getMaximum() const {
        if (root_ == nullptr || root_->keys_.empty()) {
            return std::nullopt;
        }
        const BTreeNode<KeyType>* currentNode = root_;
        while (!currentNode->isLeaf_) {
            currentNode = currentNode->children_.back();
        }
        return currentNode->keys_.back();
    }

    /**
     * @brief In-order traversal visiting all keys in sorted order.
     */
    void inOrderTraversal(const std::function<void(const KeyType&)>& visitor) const {
        inOrderTraversalHelper(root_, visitor);
    }

    /**
     * @brief Converts the tree into a sorted vector of keys.
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
     * @brief Verifies all B-Tree properties and invariants.
     *
     * 1. Key count within bounds [t-1, 2t-1] for non-root nodes; root has [1, 2t-1].
     * 2. Children count equals keys count + 1 for internal nodes.
     * 3. Keys strictly sorted.
     * 4. All leaves at identical depth.
     */
    [[nodiscard]] bool verifyProperties() const {
        if (root_ == nullptr) {
            return size_ == 0;
        }

        int leafDepth = -1;
        bool isValid = true;
        SizeType countedKeys = 0;
        verifyNode(root_, nullptr, nullptr, 0, leafDepth, true, isValid, countedKeys);
        return isValid && (countedKeys == size_);
    }

    /**
     * @brief Prints an ASCII visual representation of the B-Tree by levels.
     */
    void printTree(std::ostream& outputStream = std::cout) const {
        if (root_ == nullptr) {
            outputStream << "(empty tree)\n";
            return;
        }
        printTreeHelper(root_, 0, outputStream);
    }

private:
    BTreeNode<KeyType>* root_;
    SizeType size_;
    SizeType minimumDegree_;
    Compare compare_;

    /**
     * @brief Splits full child `children_[childIndex]` of `parentNode`.
     *
     * Moves median key up to `parentNode`, splits remaining keys and children
     * evenly between the existing child and a new sibling node.
     */
    void splitChild(BTreeNode<KeyType>* parentNode, SizeType childIndex) {
        BTreeNode<KeyType>* fullChild = parentNode->children_[childIndex];
        auto* newSibling = new BTreeNode<KeyType>(fullChild->isLeaf_);

        SizeType medianIndex = minimumDegree_ - 1;
        KeyType medianKey = fullChild->keys_[medianIndex];

        // Sibling gets keys after the median: [medianIndex + 1 ... 2t - 2]
        newSibling->keys_.assign(fullChild->keys_.begin() + (medianIndex + 1), fullChild->keys_.end());

        // If not a leaf, sibling also gets corresponding children
        if (!fullChild->isLeaf_) {
            newSibling->children_.assign(fullChild->children_.begin() + (medianIndex + 1), fullChild->children_.end());
            fullChild->children_.erase(fullChild->children_.begin() + (medianIndex + 1), fullChild->children_.end());
        }

        // Truncate full child keys to [0 ... medianIndex - 1]
        fullChild->keys_.erase(fullChild->keys_.begin() + medianIndex, fullChild->keys_.end());

        // Insert new sibling into parent's children
        parentNode->children_.insert(parentNode->children_.begin() + (childIndex + 1), newSibling);

        // Insert median key into parent's keys
        parentNode->keys_.insert(parentNode->keys_.begin() + childIndex, medianKey);
    }

    /**
     * @brief Inserts a key into a non-full node.
     */
    void insertNonFull(BTreeNode<KeyType>* node, const KeyType& key) {
        if (node->isLeaf_) {
            // Find insertion position
            auto position = std::lower_bound(node->keys_.begin(), node->keys_.end(), key, compare_);
            node->keys_.insert(position, key);
        } else {
            // Find child to recurse into
            auto position = std::lower_bound(node->keys_.begin(), node->keys_.end(), key, compare_);
            SizeType childIndex = std::distance(node->keys_.begin(), position);

            // Split child proactively if full
            if (node->children_[childIndex]->keys_.size() == (2 * minimumDegree_ - 1)) {
                splitChild(node, childIndex);
                if (compare_(node->keys_[childIndex], key)) {
                    ++childIndex;
                }
            }
            insertNonFull(node->children_[childIndex], key);
        }
    }

    /**
     * @brief Deletes a key from a subtree rooted at `node`.
     */
    bool removeFromNode(BTreeNode<KeyType>* node, const KeyType& key) {
        auto position = std::lower_bound(node->keys_.begin(), node->keys_.end(), key, compare_);
        SizeType index = std::distance(node->keys_.begin(), position);

        // Case 1: Key is in this node
        if (index < node->keys_.size() && !compare_(key, node->keys_[index])) {
            if (node->isLeaf_) {
                // Case 1a: Key is in a leaf -> remove directly
                node->keys_.erase(node->keys_.begin() + index);
                return true;
            } else {
                // Case 1b: Key is in an internal node
                BTreeNode<KeyType>* leftChild = node->children_[index];
                BTreeNode<KeyType>* rightChild = node->children_[index + 1];

                // If left child has at least t keys, find predecessor
                if (leftChild->keys_.size() >= minimumDegree_) {
                    KeyType predecessorKey = getPredecessorKey(leftChild);
                    node->keys_[index] = predecessorKey;
                    return removeFromNode(leftChild, predecessorKey);
                }
                // Else if right child has at least t keys, find successor
                else if (rightChild->keys_.size() >= minimumDegree_) {
                    KeyType successorKey = getSuccessorKey(rightChild);
                    node->keys_[index] = successorKey;
                    return removeFromNode(rightChild, successorKey);
                }
                // Else merge leftChild, key, and rightChild into leftChild
                else {
                    mergeChildren(node, index);
                    return removeFromNode(leftChild, key);
                }
            }
        } else {
            // Case 2: Key is not in this node
            if (node->isLeaf_) {
                return false; // Key not present in tree
            }

            // Flag whether key is in the last child
            bool isLastChild = (index == node->keys_.size());

            // Ensure child has at least t keys
            if (node->children_[index]->keys_.size() < minimumDegree_) {
                fillChild(node, index);
            }

            // If the last child was merged, adjust index
            if (isLastChild && index > node->keys_.size()) {
                return removeFromNode(node->children_[index - 1], key);
            } else {
                return removeFromNode(node->children_[index], key);
            }
        }
    }

    /**
     * @brief Ensures child `children_[childIndex]` has at least `minimumDegree_` keys
     * by borrowing from left/right siblings or merging.
     */
    void fillChild(BTreeNode<KeyType>* parentNode, SizeType childIndex) {
        // Try borrowing from left sibling
        if (childIndex > 0 &&
            parentNode->children_[childIndex - 1]->keys_.size() >= minimumDegree_) {
            borrowFromPrev(parentNode, childIndex);
        }
        // Try borrowing from right sibling
        else if (childIndex < parentNode->children_.size() - 1 &&
                 parentNode->children_[childIndex + 1]->keys_.size() >= minimumDegree_) {
            borrowFromNext(parentNode, childIndex);
        }
        // Merge with sibling
        else {
            if (childIndex < parentNode->children_.size() - 1) {
                mergeChildren(parentNode, childIndex);
            } else {
                mergeChildren(parentNode, childIndex - 1);
            }
        }
    }

    /**
     * @brief Borrows a key from the previous sibling `children_[childIndex - 1]`.
     */
    void borrowFromPrev(BTreeNode<KeyType>* parentNode, SizeType childIndex) {
        BTreeNode<KeyType>* child = parentNode->children_[childIndex];
        BTreeNode<KeyType>* sibling = parentNode->children_[childIndex - 1];

        // Parent key moves down into child's front
        child->keys_.insert(child->keys_.begin(), parentNode->keys_[childIndex - 1]);

        // Sibling's last key moves up to parent
        parentNode->keys_[childIndex - 1] = sibling->keys_.back();
        sibling->keys_.pop_back();

        // If not leaf, move sibling's last child to child's front
        if (!child->isLeaf_) {
            child->children_.insert(child->children_.begin(), sibling->children_.back());
            sibling->children_.pop_back();
        }
    }

    /**
     * @brief Borrows a key from the next sibling `children_[childIndex + 1]`.
     */
    void borrowFromNext(BTreeNode<KeyType>* parentNode, SizeType childIndex) {
        BTreeNode<KeyType>* child = parentNode->children_[childIndex];
        BTreeNode<KeyType>* sibling = parentNode->children_[childIndex + 1];

        // Parent key moves down into child's back
        child->keys_.push_back(parentNode->keys_[childIndex]);

        // Sibling's first key moves up to parent
        parentNode->keys_[childIndex] = sibling->keys_.front();
        sibling->keys_.erase(sibling->keys_.begin());

        // If not leaf, move sibling's first child to child's back
        if (!child->isLeaf_) {
            child->children_.push_back(sibling->children_.front());
            sibling->children_.erase(sibling->children_.begin());
        }
    }

    /**
     * @brief Merges `children_[childIndex]` with `children_[childIndex + 1]`
     * pulling down the separating key `keys_[childIndex]` from `parentNode`.
     */
    void mergeChildren(BTreeNode<KeyType>* parentNode, SizeType childIndex) {
        BTreeNode<KeyType>* leftChild = parentNode->children_[childIndex];
        BTreeNode<KeyType>* rightChild = parentNode->children_[childIndex + 1];

        // Pull separating key down from parent
        leftChild->keys_.push_back(parentNode->keys_[childIndex]);

        // Append right child's keys to left child
        leftChild->keys_.insert(leftChild->keys_.end(),
                                rightChild->keys_.begin(),
                                rightChild->keys_.end());

        // Append right child's child pointers if not leaf
        if (!leftChild->isLeaf_) {
            leftChild->children_.insert(leftChild->children_.end(),
                                        rightChild->children_.begin(),
                                        rightChild->children_.end());
        }

        // Remove separating key and rightChild pointer from parent
        parentNode->keys_.erase(parentNode->keys_.begin() + childIndex);
        parentNode->children_.erase(parentNode->children_.begin() + (childIndex + 1));

        delete rightChild;
    }

    KeyType getPredecessorKey(BTreeNode<KeyType>* node) const {
        BTreeNode<KeyType>* current = node;
        while (!current->isLeaf_) {
            current = current->children_.back();
        }
        return current->keys_.back();
    }

    KeyType getSuccessorKey(BTreeNode<KeyType>* node) const {
        BTreeNode<KeyType>* current = node;
        while (!current->isLeaf_) {
            current = current->children_.front();
        }
        return current->keys_.front();
    }

    const KeyType* searchHelper(const BTreeNode<KeyType>* node, const KeyType& key) const {
        if (node == nullptr) {
            return nullptr;
        }

        auto position = std::lower_bound(node->keys_.begin(), node->keys_.end(), key, compare_);
        SizeType index = std::distance(node->keys_.begin(), position);

        if (index < node->keys_.size() && !compare_(key, node->keys_[index])) {
            return &(node->keys_[index]);
        }

        if (node->isLeaf_) {
            return nullptr;
        }

        return searchHelper(node->children_[index], key);
    }

    static void destroySubtree(BTreeNode<KeyType>* node) noexcept {
        if (node != nullptr) {
            for (auto* child : node->children_) {
                destroySubtree(child);
            }
            delete node;
        }
    }

    static BTreeNode<KeyType>* copySubtree(const BTreeNode<KeyType>* node) {
        if (node == nullptr) {
            return nullptr;
        }
        auto* copy = new BTreeNode<KeyType>(node->isLeaf_);
        copy->keys_ = node->keys_;
        copy->children_.reserve(node->children_.size());
        for (const auto* child : node->children_) {
            copy->children_.push_back(copySubtree(child));
        }
        return copy;
    }

    void inOrderTraversalHelper(const BTreeNode<KeyType>* node,
                                const std::function<void(const KeyType&)>& visitor) const {
        if (node == nullptr) {
            return;
        }

        SizeType keyCount = node->keys_.size();
        for (SizeType i = 0; i < keyCount; ++i) {
            if (!node->isLeaf_) {
                inOrderTraversalHelper(node->children_[i], visitor);
            }
            visitor(node->keys_[i]);
        }
        if (!node->isLeaf_) {
            inOrderTraversalHelper(node->children_[keyCount], visitor);
        }
    }

    void verifyNode(const BTreeNode<KeyType>* node,
                    const KeyType* minBound,
                    const KeyType* maxBound,
                    int currentDepth,
                    int& expectedLeafDepth,
                    bool isRoot,
                    bool& isValid,
                    SizeType& keyCounter) const {
        if (node == nullptr) {
            return;
        }

        SizeType keyCount = node->keys_.size();
        keyCounter += keyCount;

        // Invariant 1: Key bounds
        if (isRoot) {
            if (keyCount > 2 * minimumDegree_ - 1) {
                isValid = false;
            }
        } else {
            if (keyCount < minimumDegree_ - 1 || keyCount > 2 * minimumDegree_ - 1) {
                isValid = false;
            }
        }

        // Invariant 2: Children count
        if (!node->isLeaf_) {
            if (node->children_.size() != keyCount + 1) {
                isValid = false;
            }
        }

        // Invariant 3: Keys sorted and within bounds
        for (SizeType i = 0; i < keyCount; ++i) {
            if (i > 0 && !compare_(node->keys_[i - 1], node->keys_[i])) {
                isValid = false;
            }
            if (minBound != nullptr && !compare_(*minBound, node->keys_[i])) {
                isValid = false;
            }
            if (maxBound != nullptr && !compare_(node->keys_[i], *maxBound)) {
                isValid = false;
            }
        }

        // Invariant 4: Leaf depth consistency
        if (node->isLeaf_) {
            if (expectedLeafDepth == -1) {
                expectedLeafDepth = currentDepth;
            } else if (expectedLeafDepth != currentDepth) {
                isValid = false;
            }
        } else {
            for (SizeType i = 0; i <= keyCount; ++i) {
                const KeyType* nextMin = (i == 0) ? minBound : &(node->keys_[i - 1]);
                const KeyType* nextMax = (i == keyCount) ? maxBound : &(node->keys_[i]);
                verifyNode(node->children_[i],
                           nextMin,
                           nextMax,
                           currentDepth + 1,
                           expectedLeafDepth,
                           false,
                           isValid,
                           keyCounter);
            }
        }
    }

    void printTreeHelper(const BTreeNode<KeyType>* node,
                         int level,
                         std::ostream& outputStream) const {
        if (node == nullptr) {
            return;
        }

        outputStream << std::string(level * 4, ' ') << "[Level " << level << "] [";
        for (SizeType i = 0; i < node->keys_.size(); ++i) {
            outputStream << node->keys_[i] << (i + 1 < node->keys_.size() ? ", " : "");
        }
        outputStream << "]" << (node->isLeaf_ ? " (leaf)" : "") << "\n";

        if (!node->isLeaf_) {
            for (const auto* child : node->children_) {
                printTreeHelper(child, level + 1, outputStream);
            }
        }
    }
};

} // namespace search_trees

#endif // SEARCH_TREES_B_TREE_HPP
