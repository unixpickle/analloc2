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
   * An abstract search comparator for a binary search tree.
   */
  class SearchFunction {
  public:
    /**
     * Returns 1 to indicate that the search should explore the left of the
     * node of a certain [value]. Returns -1 to indicate that the search should
     * explore the right of the node. Returns 0 to indicate that [value] is a
     * valid search result and that the search is complete.
     */
    virtual int DirectionFromNode(const T & value) const = 0;
  };
  
  /**
   * Create a new [DynamicTree] which will use a specified [allocator] to
   * allocate nodes.
   */
  DynamicTree(VirtualAllocator & allocator) : allocator(allocator) {}
  
  /**
   * Subclasses may use their destructors to deallocate the internal tree
   * structure.
   */
  virtual ~DynamicTree() {}
  
  /**
   * Find the lowest value in this tree which is greater than [value].
   *
   * If the tree does not contain such a value, `false` is returned. Otherwise,
   * `true` is returned and [result] is set to the value which was found.
   *
   * If [remove] is explicitly specified as `true`, the value will be removed
   * from the tree before it is returned.
   */
  virtual bool FindGreaterThan(T & result, const T & value,
                               bool remove = false) = 0;
  
  /**
   * Find the lowest value in this tree which is greater than or equal to
   * [value].
   *
   * If the tree does not contain such a value, `false` is returned. Otherwise,
   * `true` is returned and [result] is set to the value which was found.
   *
   * If [remove] is explicitly specified as `true`, the value will be removed
   * from the tree before it is returned.
   */
  virtual bool FindGreaterThanOrEqualTo(T & result, const T & value,
                                        bool remove = false) = 0;
  
  /**
   * Find the highest value in this tree which is less than [value].
   *
   * If the tree does not contain such a value, `false` is returned. Otherwise,
   * `true` is returned and [result] is set to the value which was found.
   *
   * If [remove] is explicitly specified as `true`, the value will be removed
   * from the tree before it is returned.
   */
  virtual bool FindLessThan(T & result, const T & value,
                            bool remove = false) = 0;
  
  /**
   * Find the highest value in this tree which is less than or equal to 
   * [value].
   *
   * If the tree does not contain such a value, `false` is returned. Otherwise,
   * `true` is returned and [result] is set to the value which was found.
   *
   * If [remove] is explicitly specified as `true`, the value will be removed
   * from the tree before it is returned.
   */
  virtual bool FindLessThanOrEqualTo(T & result, const T & value,
                                     bool remove = false) = 0;
  
  /**
   * Find a node in the tree using an arbitrary search [function].
   *
   * If no value can be found, `false` is returned. Otherwise, `true` is
   * returned. Upon success, [result] is set to the value which was found.
   *
   * If [remove] is specified as `true`, the node corresponding to the found
   * value is removed from the tree before it is returned.
   */
  virtual bool Search(T & result, const SearchFunction & function,
                      bool remove = false) = 0;
  
  /**
   * Check if this tree contains an exact value.
   *
   * If [value] is found in the tree, `true` is returned.
   */
  virtual bool Contains(const T & value) = 0;
  
  /**
   * Remove a [value] from the tree.
   *
   * If the value is found and removed, `true` is returned. Otherwise, `false`
   * is returned.
   */
  virtual bool Remove(const T & value) = 0;
  
  /**
   * Add a given [value] to the tree.
   *
   * The specified [value] will be copied with T(const T &). If the insertion
   * fails because a node cannot be allocated, this method will return `false`.
   * Otherwise, it will return `true`.
   */
  virtual bool Add(const T & value) = 0;
  
  /**
   * Remove all nodes from this tree.
   */
  virtual void Clear() = 0;
  
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
