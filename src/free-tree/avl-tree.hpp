#ifndef __ANALLOC2_AVL_TREE_HPP__
#define __ANALLOC2_AVL_TREE_HPP__

#include "dynamic-binary-tree.hpp"
#include <ansa/nocopy>

namespace analloc {

/**
 * A self-balancing binary search tree with a relatively high modification cost
 * but a relatively low search cost.
 */
template <class T>
class AvlTree : public DynamicBinaryTree<T>, public ansa::NoCopy {
public:
  typedef DynamicBinaryTree<T> super;
  
  class Node : public super::Node {
  public:
    Node(const T & val) : super::Node(val) {}
    
    virtual super::Node * GetLeft() {
      return static_cast<super::Node>(left);
    }
    
    virtual super::Node * GetRight() {
      return static_cast<super::Node>(right);
    }
    
    Node * left = NULL;
    Node * right = NULL;
    int depth = 0;
  };
  
  /**
   * Create a new, empty AVL tree.
   */
  AvlTree(VirtualAllocator & allocator) : super(allocator) {}
  
  /**
   * Deallocate the AVL tree and all of its nodes.
   */
  virtual ~AvlTree() {
    Clear();
  }
  
  /**
   * Returns the "mutable" root node.
   */
  virtual super::Node * GetRoot() {
    return static_cast<super::Node>(root);
  }
  
  /**
   * Returns an immutable root node with more information.
   */
  const Node * GetImmutableRoot() const {
    return root;
  }
  
  /**
   * Add a value to the tree. This runs in O(log(n)) time.
   */
  virtual bool Add(const T & value) {
    Node * node = AllocNode(value);
    if (!node) return false;
    
    // Trivial insertion case: the tree was empty
    if (!root) {
      root = node;
      return true;
    } else {
      return AddTo(root, node);
    }
  }
  
  /**
   * Recursively free all the nodes in this tree. This performs O(n)
   * deallocations and runs in O(n) time (assuming constant-time deallocation).
   */
  virtual void Clear() {
    RecursivelyDeallocNode(root);
    root = nullptr;
  }
  
  /**
   * Get the depth of the tree.
   *
   * An empty tree has a depth of 0. Otherwise, the depth is one more than the
   * depth of the root node.
   */
  inline int GetDepth() {
    if (!root) return 0;
    return root->depth + 1;
  }
  
protected:
  /**
   * The root node in the tree.
   */
  Node * root = nullptr;
  
  /**
   * Return a node's memory to the tree's allocator.
   */
  void DeallocNode(Node * node) {
    this->GetAllocator().Dealloc((uintptr_t)node, sizeof(Node));
  }
  
  /**
   * Allocate a new node from the tree's allocator.
   */
  Node * AllocNode(const T & value) {
    uintptr_t ptr;
    if (!this->GetAllocator().Alloc(ptr, sizeof(Node))) {
      return nullptr;
    }
    return new((Node *)ptr) Node(value);
  }
  
  /**
   * Used by the destructor to deallocate a node and its descendants.
   */
  void RecursivelyDeallocNode(Node * node) {
    if (!node) return;
    RecursivelyDeallocNode(node->left);
    RecursivelyDeallocNode(node->right);
    DeallocNode(node);
  }
  
  bool AddTo(Node * parent, Node * leaf) {
    if (leaf->GetValue() < parent->GetValue()) {
      Node * subnode = parent->left;
      if (!subnode) {
        parent->left = leaf;
        // TODO: update depth
        return false;
      } else {
        if (AddTo(parent, leaf)) {
          return true;
        }
        // TODO: here, rebalance
      }
    } else {
      Node * subnode = parent->right;
      if (!subnode) {
        parent->right = leaf;
        // TODO: update depth
        return false;
      } else {
        if (AddTo(parent, leaf)) {
          return true;
        }
        // TODO: here, rebalance
      }
    }
  }
  
  /**
   * Remove a [node] from the tree and deallocate it.
   */
  void RemoveNode(Node * node) {
    assert(node != nullptr);
    Node ** parentSlot = NodeParentSlot(node);
    if (!node->left) {
      // Trivial case #1: replace the node with it's right child.
      (*parentSlot) = node->right;
      if (node->right) {
        node->right->parent = node->parent;
      }
      Rebalance(node->parent);
    } else if (!node->right) {
      // Trivial case #2: replace the node with it's left child.
      (*parentSlot) = node->left;
      node->left->parent = node->parent;
      Rebalance(node->parent);
    } else {
      // Find the rightmost subnode of the node's left child (a.k.a. the 
      // in-order predecessor of [node]).
      Node * rightmost = node->left;
      while (rightmost->right) {
        rightmost = rightmost->right;
      }
      
      // Replace the rightmost node with it's left child (which might not exist
      // if the rightmost node is a leaf).
      (*NodeParentSlot(rightmost)) = rightmost->left;
      if (rightmost->left) {
        assert(rightmost->left->left == nullptr);
        assert(rightmost->left->right == nullptr);
        rightmost->left->parent = rightmost->parent;
      }

      // We will balance upwards from the parent of the rightmost node.
      Node * balanceStart = rightmost->parent;
      if (balanceStart == node) {
        // This case occurs when [rightmost] was the left child of [node].
        balanceStart = rightmost;
      }

      // Replace [node] with its in-order predecessor.
      (*parentSlot) = rightmost;
      rightmost->parent = node->parent;
      rightmost->left = node->left;
      rightmost->right = node->right;
      assert(rightmost->right != nullptr);
      rightmost->right->parent = rightmost;
      
      // rightmost->left will be nullptr in the case where [rightmost]'s left
      // child was nullptr and [rightmost] was the left child of [node].
      if (rightmost->left) {
        rightmost->left->parent = rightmost;
      }
      
      // Before rebalancing, we set [rightmost]'s depth to [node]'s old depth
      // so that [Rebalance] knows if subtrees above [rightmost] need to be
      // rebalanced as well.
      rightmost->depth = node->depth;
      
      // Rebalance the old parent of the rightmost node.
      Rebalance(balanceStart);
    }
    // Deallocate the node which was removed.
    DeallocNode(node);
  }
  
  /**
   * Rebalance a given [node] and all its ancestors. If an ancestor's depth is
   * not changed, the balancing process can be safely halted.
   */
  void Rebalance(Node * node) {
    while (node) {
      int oldDepth = node->depth;
      node->RecomputeDepth();
      Node ** parentSlot = NodeParentSlot(node);
      Node * parent = node->parent;
      (*parentSlot) = node->Rebalance();
      assert(*parentSlot != nullptr);
      if ((*parentSlot)->depth == oldDepth) {
        break;
      }
      node = parent;
    }
  }
  
  /**
   * Returns a pointer to the field which points to a given [node].
   *
   * Doing `(*NodeParentSlot(node)) = someNode` essentially replaces [node]
   * with a new node (in this case called [someNode]).
   */
  Node ** NodeParentSlot(Node * node) {
    if (!node->parent) {
      return &root;
    } else {
      if (node == node->parent->right) {
        return &node->parent->right;
      } else {
        return &node->parent->left;
      }
    }
  }
};

}

#endif
