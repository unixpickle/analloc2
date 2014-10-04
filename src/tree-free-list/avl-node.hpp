#ifndef __ANALLOC2_AVL_NODE_HPP__
#define __ANALLOC2_AVL_NODE_HPP__

#include <ansa/math>
#include <cstddef>

namespace analloc {

template <class T>
class AvlTree;

/**
 * A node which is appropriate for use in an AVL tree.
 */
template <class T>
struct AvlNode {
  /**
   * The parent of this node.
   */
  AvlNode * parent = nullptr;
  
  /**
   * The left child of this node.
   */
  AvlNode * left = nullptr;
  
  /**
   * The right child of this node.
   */
  AvlNode * right = nullptr;
  
  /**
   * The depth of this node. A depth of 0 indicates a node with no children.
   */
  int depth = 0;
  
  /**
   * Create a new [AvlNode] with a given value [val].
   */
  AvlNode(const T & val) : value(val) {}
  
  /**
   * Get the read-only value contained by this node.
   */
  inline const T & GetValue() const {
    return value;
  }
 
  /**
   * One more than the depth of the left subnode of this node, or zero if no
   * left subnode exists.
   */
  inline int GetLeftDepth() const {
    if (!left) return 0;
    return left->depth + 1;
  }
 
  /**
   * One more than the depth of the right subnode of this node, or zero if no
   * right subnode exists.
   */
  inline int GetRightDepth() const {
    if (!right) return 0;
    return right->depth + 1;
  }
 
  /**
   * Returns `true` if the right child's depth is greater than the left child's
   * depth.
   */
  inline bool IsRightHeavy() const {
    return GetLeftDepth() < GetRightDepth();
  }
  
  /**
   * Recompute the depth of this node using its two children. Returns `true` if
   * the depth changed.
   */
  inline bool RecomputeDepth() {
    int oldDepth = depth;
    depth = ansa::Max(GetLeftDepth(), GetRightDepth());
    return oldDepth != depth;
  }
  
  /**
   * Rebalance this node and return the node that should take its place.
   *
   * If the node is too imbalanced to balance with a single operation, this
   * method returns `nullptr`.
   *
   * Upon success, the returned node may have a different depth than this node
   * originally had. This node's parent will not be affected in any way, so it
   * is the caller's responsibility to set the returned [AvlNode] in the
   * parent's corresponding field and to recalculate the parent's depth.
   */
  AvlNode * Rebalance() {
    int imbalance = GetLeftDepth() - GetRightDepth();
    if (imbalance == -2) {
      // Right side is too heavy
      if (!right->IsRightHeavy()) {
        // Reduce right-left case to right-right case
        right = RotateRight(right, right->left);
        assert(!RecomputeDepth());
      }
      return RotateLeft(this, right);
    } else if (imbalance == 2) {
      if (left->IsRightHeavy()) {
        // Reduce left-right case to left-left case
        left = RotateLeft(left, left->right);
        assert(!RecomputeDepth());
      }
      return RotateRight(this, left);
    } else if (imbalance > -2 && imbalance < 2) {
      return this;
    } else {
      return nullptr;
    }
  }
  
protected:
  static AvlNode * RotateLeft(AvlNode * parent, AvlNode * child) {
    assert(parent->right == child);
    child->parent = parent->parent;
    parent->parent = child;
    parent->right = child->left;
    if (parent->right) {
      parent->right->parent = parent;
    }
    child->left = parent;
    parent->RecomputeDepth();
    child->RecomputeDepth();
    return child;
  }
  
  static AvlNode * RotateRight(AvlNode * parent, AvlNode * child) {
    assert(parent->left == child);
    child->parent = parent->parent;
    parent->parent = child;
    parent->left = child->right;
    if (parent->left) {
      parent->left->parent = parent;
    }
    child->right = parent;
    parent->RecomputeDepth();
    child->RecomputeDepth();
    return child;
  }
  
private:
  T value;
};

}

#endif
