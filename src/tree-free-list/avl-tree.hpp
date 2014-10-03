#ifndef __ANALLOC2_AVL_TREE_HPP__
#define __ANALLOC2_AVL_TREE_HPP__

#include "dynamic-tree.hpp"
#include <ansa/math>

namespace analloc {

/**
 * An AVL tree is a self-balancing binary search tree.
 */
template <class T>
class AVLTree : public DynamicTree<T> {
public:
  typedef DynamicTree<T> super;
  
  using typename super::Node;
  
  /**
   * A node in an AVL tree.
   */
  class AVLNode : public Node {
  public:
    /**
     * Get the left subnode of this node, or `NULL` if there is none.
     */
    inline const AVLNode * GetLeft() const {
      return left;
    }
    
    /**
     * Get the left subnode of this node, or `NULL` if there is none.
     */
    inline AVLNode * GetLeft() {
      return left;
    }
    
    /**
     * Get the right subnode of this node, or `NULL` if there is none.
     */
    inline const AVLNode * GetRight() const {
      return right;
    }
    
    /**
     * Get the right subnode of this node, or `NULL` if there is none.
     */
    inline AVLNode * GetRight() {
      return right;
    }
    
    /**
     * Get the parent of this node, or `NULL` if there is none.
     */
    inline const AVLNode * GetParent() const {
      return parent;
    }
    
    /**
     * Get the parent of this node, or `NULL` if there is none.
     */
    inline AVLNode * GetParent() {
      return parent;
    }
    
    /**
     * The depth of this node. If the node has no children, its depth is 0.
     */
    inline int GetDepth() const {
      return depth;
    }
    
    /**
     * One more than the depth of the left subnode of this node, or zero if no
     * left subnode exists.
     */
    inline int GetLeftDepth() const {
      if (!left) return 0;
      return left->GetDepth() + 1;
    }
    
    /**
     * One more than the depth of the right subnode of this node, or zero if no
     * right subnode exists.
     */
    inline int GetRightDepth() const {
      if (!right) return 0;
      return right->GetDepth() + 1;
    }
    
    /**
     * Returns `true` if the right subnode's depth is greater than the left
     * subnode's depth, otherwise `false`.
     */
    inline bool IsRightHeavy() const {
      return GetLeftDepth() < GetRightDepth();
    }
    
  private:
    AVLNode(const T & val) : super::Node(val) {}
    
    AVLNode * parent = NULL;
    AVLNode * left = NULL;
    AVLNode * right = NULL;
    int depth = 0; // depth 0 = no children
    
    inline void ComputeDepth() {
      depth = ansa::Max(GetLeftDepth(), GetRightDepth());
    }
  };
  
  /**
   * Create a new, empty AVL tree.
   */
  AVLTree(VirtualAllocator & allocator) : super(allocator) {}
  
  virtual AVLNode * FindAboveOrEqual(const T & value) {
    return RecursiveAboveOrEqual(root, value);
  }
  
  virtual AVLNode * FindEqual(const T & value) {
    AVLNode * current = root;
    while (current) {
      if (current->GetContent() == value) {
        return current;
      } else if (current->GetContent() < value) {
        current = current->right;
      } else { // current->GetContent() > value
        current = current->left;
      }
    }
    return NULL;
  }
  
  /**
   * Statically casts [node] to an [AVLNode] and removes it.
   */
  virtual void Remove(Node * node) {
    Remove(static_cast<AVLNode *>(node));
  }
  
  virtual void Remove(AVLNode * node) {
    if (!node) return;
    if (!node->parent) {
      assert(root == node);
      root = NULL;
    } else if (node == node->parent->left) {
      node->parent->left = NULL;
      RecursivelyComputeDepth(node->parent);
    } else {
      node->parent->right = NULL;
      RecursivelyComputeDepth(node->parent);
    }
    this->GetAllocator().Dealloc((uintptr_t)node, sizeof(AVLNode));
    Balance();
  }
  
  virtual AVLNode * Add(const T & value) {
    uintptr_t allocated;
    if (!this->GetAllocator().Alloc(allocated, sizeof(AVLNode))) {
      return NULL;
    }
    AVLNode * node = (AVLNode *)allocated;
    
    // Check if the tree is empty (trivial case)
    if (!root) {
      root = node;
      return node;
    }
    
    // Perform an insertion by searching the tree
    AVLNode * current = root;
    while (true) {
      if (current->GetContent() > value) {
        // Go to the left of the current node
        if (!current->left) {
          current->left = node;
          node->parent = current;
          return node;
        } else {
          current = current->left;
        }
      } else {
        // Go to the right of the current node
        if (!current->right) {
          current->right = node;
          node->parent = current;
          return node;
        } else {
          current = current->right;
        }
      }
    }
  }
  
  /**
   * Returns the root node of the tree, or `NULL` if the tree is empty.
   */
  inline AVLNode * GetRoot() {
    return root;
  }
  
  /**
   * Get the depth of the tree.
   *
   * An empty tree has a depth of 0. Otherwise, the depth is one more than the
   * depth of the root node.
   */
  inline int GetDepth() {
    if (!root) return 0;
    return root->GetDepth() + 1;
  }
  
private:
  AVLNode * root = NULL;
  
  AVLNode * RecursiveAboveOrEqual(AVLNode * current, const T & value) {
    if (!current) return NULL;
    if (current->GetContent() == value) {
      return current;
    } else if (current->GetContent() < value) {
      return RecursiveAboveOrEqual(current->right);
    } else { // current->GetContent() > value
      AVLNode * res = RecursiveAboveOrEqual(current->left);
      if (res) return res;
      return current;
    }
  }
  
  void RecursivelyComputeDepth(AVLNode * node) {
    while (node) {
      node->ComputeDepth();
      node = node->parent;
    }
  }
  
  void Balance() {
    Balance(root, root);
  }
  
  void Balance(AVLNode *& parentSlot, AVLNode * nodeA) {
    if (!nodeA) return;
    
    // Balance the node's children
    Balance(nodeA->left, nodeA->left);
    Balance(nodeA->right, nodeA->right);
    nodeA->ComputeDepth();
    
    // See if this node is in one of the 4 imbalanced situations.
    int leftDepth = nodeA->GetLeftDepth();
    int rightDepth = nodeA->GetRightDepth();
    if (rightDepth > leftDepth + 1) {
      if (nodeA->right->IsRightHeavy()) {
        BalanceRightRight(parentSlot, nodeA);
      } else {
        BalanceRightLeft(parentSlot, nodeA);
      }
    } else {
      if (nodeA->left->IsRightHeavy()) {
        BalanceLeftRight(parentSlot, nodeA);
      } else {
        BalanceLeftLeft(parentSlot, nodeA);
      }
    }
  }
  
  void BalanceRightRight(AVLNode *& parentSlot, AVLNode * nodeA) {
    AVLNode * nodeB = nodeA->right;
    parentSlot = nodeB;
    nodeB->parent = nodeA->parent;
    
    // Move node A to the left of node B
    if ((nodeA->right = nodeB->left)) {
      nodeA->right->parent = nodeA;
    }
    nodeB->left = nodeA;
    nodeA->parent = nodeB;
    
    // Re-compute depths
    nodeA->ComputeDepth();
    nodeB->ComputeDepth();
  }
  
  void BalanceRightLeft(AVLNode *& parentSlot, AVLNode * nodeA) {
    AVLNode * nodeB = nodeA->right;
    AVLNode * nodeC = nodeA->right->left;
    parentSlot = nodeB;
    nodeB->parent = nodeA->parent;
    
    // Move node A
    if ((nodeA->right = nodeB->left)) {
      nodeA->right->parent = nodeA;
    }
    nodeB->left = nodeA;
    nodeA->parent = nodeB;
    
    // Move node C
    if ((nodeC->left = nodeB->right)) {
      nodeC->left->parent = nodeC;
    }
    nodeB->right = nodeC;
    nodeC->parent = nodeB;
    
    // Re-compute depths
    nodeA->ComputeDepth();
    nodeC->ComputeDepth();
    nodeB->ComputeDepth();
  }
  
  void BalanceLeftLeft(AVLNode *& parentSlot, AVLNode * nodeA) {
    AVLNode * nodeB = nodeA->left;
    parentSlot = nodeB;
    nodeB->parent = nodeA->parent;
    
    // Move node A to the right of node B
    if ((nodeA->left = nodeB->right)) {
      nodeA->left->parent = nodeA;
    }
    nodeB->right = nodeA;
    nodeA->parent = nodeB;
    
    // Re-compute depths
    nodeA->ComputeDepth();
    nodeB->ComputeDepth();
  }
  
  void BalanceLeftRight(AVLNode *& parentSlot, AVLNode * nodeA) {
    AVLNode * nodeB = nodeA->left;
    AVLNode * nodeC = nodeA->left->right;
    parentSlot = nodeB;
    nodeB->parent = nodeA->parent;
    
    // Move node A
    if ((nodeA->left = nodeB->right)) {
      nodeA->left->parent = nodeA;
    }
    nodeB->right = nodeA;
    nodeA->parent = nodeB;
    
    // Move node C
    if ((nodeC->right = nodeB->left)) {
      nodeC->right->parent = nodeC;
    }
    nodeB->left = nodeC;
    nodeC->parent = nodeB;
    
    // Re-compute depths
    nodeA->ComputeDepth();
    nodeC->ComputeDepth();
    nodeB->ComputeDepth();
  }
};

}

#endif
