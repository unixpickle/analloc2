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
  
  virtual bool Contains(const T & value) {
    return FindEqual(value) != NULL;
  }
  
  virtual bool Remove(const T & value) {
    AvlNode * node = FindEqual(value);
    if (!node) {
      return false;
    } else {
      RemoveNode(node);
      return true;
    }
  }
  
  virtual bool Add(const T & value) {
    
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
  AvlNode * root = NULL;
  
  void DeallocNode(AvlNode * node) {
    GetAllocator().Dealloc((uintptr_t)node, sizeof(AvlNode));
  }
  
  AvlNode * AllocNode() {
    uintptr_t ptr;
    if (!GetAllocator().Alloc(ptr, sizeof(AvlNode))) {
      return NULL;
    }
    return (AvlNode *)ptr;
  }
  
  void RecursivelyDeallocNode(AvlNode * node) {
    if (!node) return;
    RecursivelyDeallocNode(node->left);
    RecursivelyDeallocNode(node->right);
    DeallocNode(node);
  }
  
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
  
  void RemoveNode(AvlNode * node) {
    // TODO: this
  }
};

}

#endif
