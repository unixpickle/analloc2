#ifndef __ANALLOC2_AVL_TREE_HPP__
#define __ANALLOC2_AVL_TREE_HPP__

#include "dynamic-tree.hpp"
#include "avl-node.hpp"
#include <ansa/nocopy>

namespace analloc {

/**
 * A self-balancing binary search tree with a relatively high modification cost
 * but a relatively low search cost.
 */
template <class T>
class AvlTree : public DynamicTree<T>, public ansa::NoCopy {
public:
  typedef DynamicTree<T> super;
  typedef AvlNode<T> Node;
  using typename super::Query;
  using typename super::EnumerateCallback;
  
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
  
  virtual bool FindGT(T & result, const T & value, bool remove = false) {
    return InternalFind(RecursivelySearchAbove(root, value, false),
                        result, remove);
  }
  
  virtual bool FindGE(T & result, const T & value, bool remove = false) {
    return InternalFind(RecursivelySearchAbove(root, value, true),
                        result, remove);
  }
  
  virtual bool FindLT(T & result, const T & value, bool remove = false) {
    return InternalFind(RecursivelySearchBelow(root, value, false),
                        result, remove);
  }
  
  virtual bool FindLE(T & result, const T & value, bool remove = false) {
    return InternalFind(RecursivelySearchBelow(root, value, true),
                        result, remove);
  }
  
  virtual bool Search(T & result, const Query & function,
                      bool remove = false) {
    Node * node = root;
    while (node) {
      int comparison = function.DirectionFromNode(node->GetValue());
      if (comparison == 0) {
        result = node->GetValue();
        if (remove) {
          RemoveNode(node);
        }
        return true;
      } else if (comparison == -1) {
        node = node->left;
      } else {
        node = node->right;
      }
    }
    return false;
  }
  
  /**
   * Returns `true` if the tree contains a given [value]. This runs in
   * O(log(n)) time.
   */
  virtual bool Contains(const T & value) {
    return FindEqual(value) != nullptr;
  }
  
  /**
   * Remove a value from the tree. This runs in O(log(n)) time.
   */
  virtual bool Remove(const T & value) {
    Node * node = FindEqual(value);
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
    Node * node = AllocNode(value);
    if (!node) return false;
    
    // Trivial insertion case: the tree was empty
    if (!root) {
      root = node;
      return true;
    }
    
    // Find a leaf node which is as close to this node as possible
    Node * current = root;
    while (true) {
      if (value > current->GetValue()) {
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
   * Recursively free all the nodes in this tree. This performs O(n)
   * deallocations and runs in O(n) time (assuming constant-time deallocation).
   */
  virtual void Clear() {
    RecursivelyDeallocNode(root);
    root = nullptr;
  }
  
  /**
   * Enumerate over the elements in the tree from least to greatest order.
   */
  virtual void Enumerate(EnumerateCallback & callback) {
    EnumerateFromNode(root, callback);
  }
  
  /**
   * Returns the root node of the tree, or `nullptr` if the tree is empty.
   */
  inline const Node * GetRoot() {
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
   * Recursively find a value which is greater than (or equal to, if
   * [allowEqual] is `true`) a given [value].
   */
  Node * RecursivelySearchAbove(Node * current, const T & value,
                                bool allowEqual) {
    if (!current) return nullptr;
    bool areEqual = (current->GetValue() == value);
    if (current->GetValue() < value || (!allowEqual && areEqual)) {
      return RecursivelySearchAbove(current->right, value, allowEqual);
    } else if (allowEqual && areEqual) {
      return current;
    } else {
      Node * res = RecursivelySearchAbove(current->left, value, allowEqual);
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
  Node * RecursivelySearchBelow(Node * current, const T & value,
                                   bool allowEqual) {
    if (!current) return nullptr;
    bool areEqual = (current->GetValue() == value);
    if (current->GetValue() > value || (!allowEqual && areEqual)) {
      return RecursivelySearchBelow(current->left, value, allowEqual);
    } else if (allowEqual && areEqual) {
      return current;
    } else {
      Node * res = RecursivelySearchBelow(current->right, value, allowEqual);
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
  inline bool InternalFind(Node * node, T & output, bool remove) {
    if (!node) {
      return false;
    } else {
      output = node->GetValue();
      if (remove) {
        RemoveNode(node);
      }
      return true;
    }
  }
  
  /**
   * Find a node in the tree which contains a given [value].
   */
  Node * FindEqual(const T & value) {
    Node * node = root;
    while (node) {
      if (node->GetValue() == value) {
        return node;
      } else if (node->GetValue() < value) {
        node = node->right;
      } else {
        node = node->left;
      }
    }
    return nullptr;
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
  
  bool EnumerateFromNode(Node * node, EnumerateCallback & callback) {
    if (!node) return true;
    if (!EnumerateFromNode(node->left, callback)) return false;
    if (!callback.Yield(node->GetValue())) return false;
    return EnumerateFromNode(node->right, callback);
  }
};

}

#endif
