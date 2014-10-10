#ifndef __ANALLOC2_AVL_NODE_HPP__
#define __ANALLOC2_AVL_NODE_HPP__

#include "dynamic-bst.hpp"
#include <ansa/math>
#include <cstddef>

namespace analloc {

/**
 * A node which tracks its depth as well as its children.
 */
template <class T>
struct AvlNode : public DynamicBst<T>::Node {
  typedef typename DynamicBst<T>::Node super;
  
  AvlNode * left = nullptr;
  AvlNode * right = nullptr;
  int depth = 0;
  
  AvlNode(const T & val) : super(val) {}
  
  virtual super * GetLeft() {
    return static_cast<super *>(left);
  }
  
  virtual super * GetRight() {
    return static_cast<super *>(right);
  }
  
  virtual const super * GetLeft() const {
    return static_cast<const super *>(left);
  }
  
  virtual const super * GetRight() const {
    return static_cast<const super *>(right);
  }
  
  /**
   * Returns 0 if there is no left child. Otherwise, returns one more than the
   * depth of the left child.
   */
  inline int GetLeftDepth() const {
    return left ? left->depth + 1 : 0;
  }
  
  /**
   * Returns 0 if there is no right child. Otherwise, returns one more than the
   * depth of the right child.
   */
  inline int GetRightDepth() const {
    return right ? right->depth + 1 : 0;
  }
  
  /**
   * Returns `true` if and only if the left depth is less than the right one.
   */
  inline bool IsRightHeavy() const {
    return GetLeftDepth() < GetRightDepth();
  }
  
  /**
   * Returns `true` if and only if the right depth is less than the left one.
   */
  inline bool IsLeftHeavy() const {
    return GetRightDepth() < GetLeftDepth();
  }
  
  /**
   * Reset this node's [depth] based on [GetLeftDepth] and [GetRightDepth].
   */
  inline void RecomputeDepth() {
    depth = ansa::Max(GetLeftDepth(), GetRightDepth());
  }
  
  /**
   * Perform a single AVL rebalance operation on this node and return the node
   * which ought to take its place.
   *
   * This operation automatically recomputes the depths of every node involved.
   */
  AvlNode * Rebalance() {
    int imbalance = GetLeftDepth() - GetRightDepth();
    assert(imbalance >= -2 && imbalance <= 2);
    if (imbalance == -2) {
      // Right-X case
      if (right->IsLeftHeavy()) {
        // Reduce right-left case to right-right case
        right = RotateRight(right, right->left);
      }
      return RotateLeft(this, right);
    } else if (imbalance == 2) {
      // Left-X case
      if (left->IsRightHeavy()) {
        // Reduce left-right case to left-left case
        left = RotateLeft(left, left->right);
      }
      return RotateRight(this, left);
    } else {
      return this;
    }
  }
  
  /**
   * Changes a sub-tree that looks like this:
   *
   *     O
   *      \
   *       1
   *
   * To one that looks like this:
   *
   *     1
   *    /
   *   0
   *
   */
  static AvlNode * RotateLeft(AvlNode * parent, AvlNode * child) {
    assert(parent->right == child);
    parent->right = child->left;
    child->left = parent;
    parent->RecomputeDepth();
    child->RecomputeDepth();
    return child;
  }
  
  /**
   * Changes a sub-tree that looks like this:
   *
   *     1
   *    /
   *   0
   *
   * To one that looks like this:
   *
   *     O
   *      \
   *       1
   *
   */
  static AvlNode * RotateRight(AvlNode * parent, AvlNode * child) {
    assert(parent->left == child);
    parent->left = child->right;
    child->right = parent;
    parent->RecomputeDepth();
    child->RecomputeDepth();
    return child;
  }
};

}

#endif
