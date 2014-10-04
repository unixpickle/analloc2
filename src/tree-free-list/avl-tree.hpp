#ifndef __ANALLOC2_AVL_TREE_HPP__
#define __ANALLOC2_AVL_TREE_HPP__

#include "dynamic-tree.hpp"
#include "avl-node.hpp"

namespace analloc {

/**
 * A self-balancing binary search tree with a relatively high modification cost
 * but a relatively low search cost.
 */
template <class T>
class AvlTree : public DynamicTree<T> {
public:
  typedef DynamicTree<T> super;
  
  /**
   * Create a new, empty AVL tree.
   */
  AvlTree(VirtualAllocator & allocator) : super(allocator) {}
  
  /**
   * Deallocate the AVL tree and all of its nodes.
   */
  virtual ~AvlTree() {
    RecursivelyDeallocNode(root);
  }
  
  virtual bool FindGreaterThan(T & result, const T & value,
                               bool remove = false) {
    return InternalFind(RecursivelySearchAbove(root, value, false),
                        result, remove);
  }
  
  virtual bool FindGreaterThanOrEqualTo(T & result, const T & value,
                                        bool remove = false) {
    return InternalFind(RecursivelySearchAbove(root, value, true),
                        result, remove);
  }
  
  virtual bool FindLessThan(T & result, const T & value,
                            bool remove = false) {
    return InternalFind(RecursivelySearchBelow(root, value, false),
                        result, remove);
  }
  
  virtual bool FindLessThanOrEqualTo(T & result, const T & value,
                                     bool remove = false) {
    return InternalFind(RecursivelySearchBelow(root, value, true),
                        result, remove);
  }
  
  /**
   * Returns `true` if the tree contains a given [value]. This runs in
   * O(log(n)) time.
   */
  virtual bool Contains(const T & value) {
    return FindEqual(value) != NULL;
  }
  
  /**
   * Remove a value from the tree. This runs in O(log(n)) time.
   */
  virtual bool Remove(const T & value) {
    AvlNode * node = FindEqual(value);
    if (!node) {
      return false;
    } else {
      RemoveNode(node);
      return true;
    }
  }
  
  /**
   * Add a value to the tree. This runs in O(log(n)) time.
   */
  virtual bool Add(const T & value) {
    AvlNode * node = AllocNode();
    if (!node) return false;
    
    // Trivial insertion case: the tree was empty
    if (!root) {
      root = node;
      return true;
    }
    
    AvlNode * current = root;
    while (true) {
      if (value > current->value) {
        if (current->right) {
          current = current->right;
        } else {
          current->right = node;
          node->parent = current;
          Rebalance(current);
          return true;
        }
      } else {
        if (current->left) {
          current = current->left;
        } else {
          current->left = node;
          node->parent = current;
          Rebalance(current);
          return true;
        }
      }
    }
  }
  
  /**
   * Returns the root node of the tree, or `NULL` if the tree is empty.
   */
  inline const AvlNode * GetRoot() {
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
  
protected:
  /**
   * The root node in the tree.
   */
  AvlNode * root = NULL;
  
  /**
   * Return a node's memory to the tree's allocator.
   */
  void DeallocNode(AvlNode * node) {
    GetAllocator().Dealloc((uintptr_t)node, sizeof(AvlNode));
  }
  
  /**
   * Allocate a new node from the tree's allocator.
   */
  AvlNode * AllocNode() {
    uintptr_t ptr;
    if (!GetAllocator().Alloc(ptr, sizeof(AvlNode))) {
      return NULL;
    }
    return (AvlNode *)ptr;
  }
  
  /**
   * Used by the destructor to deallocate a node and its descendants.
   */
  void RecursivelyDeallocNode(AvlNode * node) {
    if (!node) return;
    RecursivelyDeallocNode(node->left);
    RecursivelyDeallocNode(node->right);
    DeallocNode(node);
  }
  
  /**
   * Recursively find a value which is greater than (or equal to, if [equal] is
   * `true`) a given [value].
   */
  AvlNode * RecursivelySearchAbove(AvlNode * current, const T & value,
                                   bool equal) {
    if (!current) return NULL;
    if (current->value < value) {
      return RecursivelySearchAbove(current->right, value, equal);
    } else if (equal && current->value == value) {
      return current;
    } else {
      AvlNode * res = RecursivelySearchAbove(current->left, value, equal);
      if (res) {
        return res;
      } else {
        return current;
      }
    }
  }
  
  /**
   * Recursively find a value which is less than (or equal to, if [equal] is
   * `true`) a given [value].
   */
  AvlNode * RecursivelySearchBelow(AvlNode * current, const T & value,
                                   bool equal) {
    if (!current) return NULL;
    if (current->value > value) {
      return RecursivelySearchBelow(current->left, value, equal);
    } else if (equal && current->value == value) {
      return current;
    } else {
      AvlNode * res = RecursivelySearchBelow(current->right, value, equal);
      if (res) {
        return res;
      } else {
        return current;
      }
    }
  }
  
  /**
   * The internal mechanism behind all of the find methods. This is used to
   * save a few lines of code in each of the four methods.
   */
  inline bool InternalFind(AvlNode * node, T & output, bool remove) {
    if (!node) {
      return false;
    } else {
      output = node->value;
      if (remove) {
        RemoveNode(node);
      }
      return true;
    }
  }
  
  /**
   * Find a node in the tree which contains a given [value].
   */
  AvlNode * FindEqual(const T & value) {
    AvlNode * node = root;
    while (node) {
      if (node->value == value) {
        return node;
      } else if (node->value < value) {
        node = node->right;
      } else {
        node = node->left;
      }
    }
    return NULL;
  }
  
  /**
   * Remove a [node] from the tree and deallocate it.
   */
  void RemoveNode(AvlNode * node) {
    assert(node != NULL);
    AvlNode ** parentSlot = NodeParentSlot(node);
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
      // Find the rightmost subnode of the left node.
      AvlNode * rightmost = node;
      while (rightmost->right) {
        rightmost = rightmost->right;
      }
      // Replace the rightmost node with it's left child (which might not exist
      // if the rightmost node is a leaf).
      (*NodeParentSlot(rightmost)) = rightmost->left;
      if (rightmost->left) {
        rightmost->left->parent = rightmost->parent;
      }
      // We will balance upwards from the parent of the rightmost node.
      AvlNode * balanceStart = rightmost->parent;
      // Replace the node to delete with the rightmost node.
      (*parentSlot) = rightmost;
      rightmost->parent = node->parent;
      rightmost->left = node->left;
      rightmost->right = node->right;
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
  void Rebalance(AvlNode * node) {
    while (node) {
      if (!node->RecomputeDepth()) {
        return;
      }
      AvlNode ** parentSlot = NodeParentSlot(node);
      AvlNode * parent = node->parent;
      (*parentSlot) = node->Rebalance();
      node = parent;
    }
  }
  
  /**
   * Returns a pointer to the field which points to a given [node].
   *
   * Doing `(*NodeParentSlot(node)) = someNode` essentially replaces [node]
   * with a new node (in this case called [someNode]).
   */
  AvlNode ** NodeParentSlot(AvlNode * node) {
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
