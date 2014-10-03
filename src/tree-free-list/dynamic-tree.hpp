#ifndef __ANALLOC2_DYNAMIC_TREE_HPP__
#define __ANALLOC2_DYNAMIC_TREE_HPP__

#include "../abstract/virtual-allocator.hpp"

namespace analloc {

/**
 * A tree which grows and shrinks based on the number of nodes it contains.
 *
 * A [DynamicTree] can be used to store a sorted list of objects which can be
 * searched and modified in logarithmic time.
 */
template <class T>
class DynamicTree {
public:
  /**
   * A node which is contained within a tree.
   */
  class Node {
  public:
    /**
     * Returns a constant reference to the contents of this node. This object
     * will be destroyed when this node is destroyed, so you should not store
     * this reference indefinitely.
     */
    inline const T & GetContent() const {
      return content;
    }
    
  private:
    Node(const T & value) : content(value) {}
    
    T content;
  };
  
  /**
   * Create a new [DynamicTree] which will use a specified [allocator] to
   * allocate nodes.
   */
  DynamicTree(VirtualAllocator & allocator) : allocator(allocator) {}
  
  virtual ~DynamicTree() {}
  
  /**
   * Find the [Node] with the lowest value in this tree which is greater than
   * or equal to a given [value].
   *
   * If no such node exists, `NULL` will be returned.
   */
  virtual Node * FindAboveOrEqual(const T & value) = 0;
  
  /**
   * Find the first [Node] which has a [value].
   *
   * If no such node exists, `NULL` will be returned.
   */
  virtual Node * FindEqual(const T & value) = 0;
  
  /**
   * Remove a [node] from the tree.
   */
  virtual void Remove(Node * node) = 0;
  
  /**
   * Add a node to the tree with a given [value].
   *
   * The specified [value] will be copied with T(const T &). If the node cannot
   * be allocated because the tree's allocator fails, `NULL` will be returned.
   * Otherwise, a new node is returned which stores the added [value].
   */
  virtual Node * Add(const T & value) = 0;
  
  /**
   * Returns the allocator which this tree uses to allocate nodes.
   */
  inline VirtualAllocator & GetAllocator() {
    return allocator;
  }
  
private:
  VirtualAllocator & allocator;
};

}

#endif
