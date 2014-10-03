#ifndef __ANALLOC2_Avl_TREE_HPP__
#define __ANALLOC2_Avl_TREE_HPP__

#include "dynamic-tree.hpp"
#include <ansa/math>

namespace analloc {

/**
 * An AVL tree is a self-balancing binary search tree.
 */
template <class T>
class AvlTree : public DynamicTree<T> {
public:
  typedef DynamicTree<T> super;
  
  using typename super::Node;
  
  /**
   * A node in an AVL tree.
   */
  class AvlNode : public Node {
  public:
    /**
     * Get the left subnode of this node, or `NULL` if there is none.
     */
    inline const AvlNode * GetLeft() const {
      return left;
    }
    
    /**
     * Get the left subnode of this node, or `NULL` if there is none.
     */
    inline AvlNode * GetLeft() {
      return left;
    }
    
    /**
     * Get the right subnode of this node, or `NULL` if there is none.
     */
    inline const AvlNode * GetRight() const {
      return right;
    }
    
    /**
     * Get the right subnode of this node, or `NULL` if there is none.
     */
    inline AvlNode * GetRight() {
      return right;
    }
    
    /**
     * Get the parent of this node, or `NULL` if there is none.
     */
    inline const AvlNode * GetParent() const {
      return parent;
    }
    
    /**
     * Get the parent of this node, or `NULL` if there is none.
     */
    inline AvlNode * GetParent() {
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
    AvlNode(const T & val) : super::Node(val) {}
    
    AvlNode * parent = NULL;
    AvlNode * left = NULL;
    AvlNode * right = NULL;
    int depth = 0; // depth 0 = no children
    
    inline void ComputeDepth() {
      depth = ansa::Max(GetLeftDepth(), GetRightDepth());
    }
  };
  
  /**
   * Create a new, empty AVL tree.
   */
  AvlTree(VirtualAllocator & allocator) : super(allocator) {}
  
  /**
   * Deallocate the AVL tree and all its nodes.
   */
  virtual ~AvlTree() {
    RecursivelyDelete(root);
  }
  
  virtual AvlNode * FindAboveOrEqual(const T & value) {
    return RecursiveAboveOrEqual(root, value);
  }
  
  virtual AvlNode * FindEqual(const T & value) {
    AvlNode * current = root;
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
   * Statically casts [node] to an [AvlNode] and removes it.
   */
  virtual void Remove(Node * node) {
    Remove(static_cast<AvlNode *>(node));
  }
  
  virtual void Remove(AvlNode * node) {
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
    this->GetAllocator().Dealloc((uintptr_t)node, sizeof(AvlNode));
    Balance();
  }
  
  virtual AvlNode * Add(const T & value) {
    uintptr_t allocated;
    if (!this->GetAllocator().Alloc(allocated, sizeof(AvlNode))) {
      return NULL;
    }
    AvlNode * node = (AvlNode *)allocated;
    
    // Check if the tree is empty (trivial case)
    if (!root) {
      root = node;
      return node;
    }
    
    // Perform an insertion by searching the tree
    AvlNode * current = root;
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
  inline AvlNode * GetRoot() {
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
  AvlNode * root = NULL;
  
  AvlNode * RecursiveAboveOrEqual(AvlNode * current, const T & value) {
    if (!current) return NULL;
    if (current->GetContent() == value) {
      return current;
    } else if (current->GetContent() < value) {
      return RecursiveAboveOrEqual(current->right);
    } else { // current->GetContent() > value
      AvlNode * res = RecursiveAboveOrEqual(current->left);
      if (res) return res;
      return current;
    }
  }
  
  void RecursivelyComputeDepth(AvlNode * node) {
    while (node) {
      node->ComputeDepth();
      node = node->parent;
    }
  }
  
  void Balance() {
    Balance(root, root);
  }
  
  void Balance(AvlNode *& parentSlot, AvlNode * nodeA) {
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
  
  void BalanceRightRight(AvlNode *& parentSlot, AvlNode * nodeA) {
    AvlNode * nodeB = nodeA->right;
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
  
  void BalanceRightLeft(AvlNode *& parentSlot, AvlNode * nodeA) {
    AvlNode * nodeB = nodeA->right;
    AvlNode * nodeC = nodeA->right->left;
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
  
  void BalanceLeftLeft(AvlNode *& parentSlot, AvlNode * nodeA) {
    AvlNode * nodeB = nodeA->left;
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
  
  void BalanceLeftRight(AvlNode *& parentSlot, AvlNode * nodeA) {
    AvlNode * nodeB = nodeA->left;
    AvlNode * nodeC = nodeA->left->right;
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
  
  void RecursivelyDelete(AvlNode * node) {
    if (!node) return;
    if (node->left) RecursivelyDelete(node->left);
    if (node->right) Recursivelydelete(node->right);
    this->GetAllocator().Dealloc((uintptr_t)node, sizeof(AvlNode));
  }
};

}

#endif
