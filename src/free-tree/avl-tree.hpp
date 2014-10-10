#ifndef __ANALLOC2_AVL_TREE_HPP__
#define __ANALLOC2_AVL_TREE_HPP__

#include "avl-node.hpp"
#include <ansa/nocopy>

#include <iostream> // TODO: delete this

namespace analloc {

/**
 * A self-balancing binary search tree with a relatively high modification cost
 * but a relatively low search cost.
 */
template <class T>
class AvlTree : public DynamicBst<T>, public ansa::NoCopy {
public:
  typedef DynamicBst<T> super;
  typedef AvlNode<T> Node;
  
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
   * Returns the "mutable" root node which lacks depth information.
   */
  virtual typename super::Node * GetRoot() {
    return static_cast<typename super::Node *>(root);
  }
  
  /**
   * Returns an "immutable" root node which includes depth information.
   */
  const Node * GetAvlRoot() const {
    return root;
  }
  
  /**
   * Add a [value] to the tree. This runs in O(log(n)) time.
   */
  virtual bool Add(const T & value) {
    Node * node = AllocNode(value);
    if (!node) return false;
    
    // Trivial insertion case: the tree was empty
    if (!root) {
      root = node;
    } else {
      root = AddTo(root, node);
    }
    return true;
  }
  
  /**
   * Remove a [value] from the tree.
   */
  virtual bool Remove(const T & value) {
    bool found;
    root = RemoveFrom(root, value, found);
    return found;
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
    return root ? root->depth + 1 : 0;
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
  
  /**
   * Add a [leaf] somewhere underneath a [parent].
   *
   * Returns a balanced node to put in the place of [parent].
   */
  Node * AddTo(Node * parent, Node * leaf) {
    if (leaf->GetValue() < parent->GetValue()) {
      Node * subnode = parent->left;
      if (!subnode) {
        parent->left = leaf;
        parent->RecomputeDepth();
        return parent;
      } else {
        parent->left = AddTo(subnode, leaf);
        parent->RecomputeDepth();
        return parent->Rebalance();
      }
    } else {
      Node * subnode = parent->right;
      if (!subnode) {
        parent->right = leaf;
        parent->RecomputeDepth();
        return parent;
      } else {
        parent->right = AddTo(subnode, leaf);
        parent->RecomputeDepth();
        return parent->Rebalance();
      }
    }
  }
  
  /**
   * Remove a leaf with a given [value] from a [parent] node. The parent node
   * itself might have the value in question.
   *
   * The returned node should take the place of [parent] in the tree.
   */
  Node * RemoveFrom(Node * parent, const T & value, bool & found) {
    if (!parent) {
      found = false;
      return nullptr;
    }
    if (parent->GetValue() > value) {
      // TODO: see if an extra check makes this faster
      parent->left = RemoveFrom(parent->left, value, found);
      parent->RecomputeDepth();
      return parent->Rebalance();
    } else if (parent->GetValue() < value) {
      // TODO: see if an extra check makes this faster
      parent->right = RemoveFrom(parent->right, value, found);
      parent->RecomputeDepth();
      return parent->Rebalance();
    } else {
      found = true;
      Node * result = RemoveSubroutine(parent);
      DeallocNode(parent);
      return result;
    }
  }
  
  /**
   * "Remove" a node without deallocating it.
   *
   * Returns the new node which should take [node]'s place.
   */
  Node * RemoveSubroutine(Node * node) {
    if (!node->left) {
      // Replace node with its right child
      return node->right;
    } else if (!node->right) {
      // Replace node with its left child
      return node->left;
    } else {
      // Replace node with its in-order predecessor
      Node * predecessor;
      Node * result = RemoveInOrderPredecessor(node->left, predecessor);
      predecessor->left = result;
      predecessor->right = node->right;
      predecessor->RecomputeDepth();
      return predecessor->Rebalance();
    }
  }
  
  Node * RemoveInOrderPredecessor(Node * parent, Node *& predecessor) {
    if (!parent->right) {
      predecessor = parent;
      return parent->left;
    } else {
      parent->right = RemoveInOrderPredecessor(parent->right, predecessor);
      parent->RecomputeDepth();
      return parent->Rebalance();
    }
  }
};

}

#endif
