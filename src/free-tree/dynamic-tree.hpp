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
   * An abstract "comparator" used for querying elements in the tree.
   */
  class Query {
  public:
    enum Direction {
      DirectionFound,
      DirectionLeft,
      DirectionRight
    }
    
    /**
     * A method which the tree calls for each node in the tree it searches.
     *
     * If this method returns [DirectionFound], the tree will stop the search
     * and yield [value] as the result.
     *
     * If this method returns [DirectionLeft], the tree will continue its
     * search with the left child of the current node. Conversely, if it
     * returns [DirectionRight], the tree will search to the right of the node.
     *
     * It should be noted that nodes on the "left" are lower and nodes on the
     * "right" are higher.
     */
    virtual Direction Next(const T & value) const = 0;
  };
  
  /**
   * An abstract subclass of [Query] which adds the abstraction of "acceptable"
   * values, so that the "best" result can be found out of several
   * possibilities.
   */
  class BestQuery : public Query {
  public:
    /**
     * Return `true` if and only if [value] is an acceptable result.
     */
    virtual bool Accepts(const T & value) const = 0;
    
    /**
     * Return the "better" of the two values [value1] and [value2].
     */
    virtual const T & Better(const T & value1, const T & value2) const = 0;
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
   * Find a node in the tree using an arbitrary search [query].
   *
   * If no value can be found, `false` is returned. Otherwise, `true` is
   * returned. Upon success, [result] is set to the value which was found. If
   * [result] is `nullptr`, the actual value which was found will be ignored.
   *
   * If [remove] is specified as `true`, the node corresponding to the found
   * value is removed from the tree before it is returned.
   */
  virtual bool Search(T * result, const Query & query,
                      bool remove = false) = 0;
  
  /**
   * Like [Search], but finds the best available result using a [BestQuery].
   */
  virtual bool SearchBest(T * result, const BestQuery & query,
                          bool remove = false) = 0;
  
  /**
   * Searches for the lowest value in this tree which is greater than [value].
   */
  virtual bool SearchGT(T * result, const T & value, bool remove = false) {
    SearchBest(result, ComparatorBestQuery(value, true, false), remove);
  }

  /**
   * Searches for the lowest value in this tree which is greater than or equal
   * to [value].
   */
  virtual bool SearchGE(T * result, const T & value, bool remove = false) {
    SearchBest(result, ComparatorBestQuery(value, true, true), remove);
  }

  /**
   * Searches for the greatest value in this tree which is less than [value].
   */
  virtual bool SearchLT(T * result, const T & value, bool remove = false) {
    SearchBest(result, ComparatorBestQuery(value, false, false), remove);
  }

  /**
   * Searches for the greatest value in this tree which is less than or equal
   * to [value].
   */
  virtual bool SearchLE(T * result, const T & value, bool remove = false) {
    SearchBest(result, ComparatorBestQuery(value, false, true), remove);
  }
  
  /**
   * Check if this tree contains an exact value.
   *
   * If [value] is found in the tree, `true` is returned.
   */
  virtual bool Contains(const T & value) {
    return Search(nullptr, ValueQuery(value));
  }
  
  /**
   * Remove a [value] from the tree.
   *
   * If the value is found and removed, `true` is returned. Otherwise, `false`
   * is returned.
   */
  virtual bool Remove(const T & value) {
    return Search(nullptr, ValueQuery(value), true);
  }
  
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
  
  /**
   * A concrete [Query] which matches a specified value.
   */
  class ValueQuery : public Query {
  public:
    ValueQuery(const T & _match) : match(_match) {}
    
    virtual Direction Next(const T & value) const {
      if (value == match) return DirectionFound;
      else if (match > value) return DirectionRight;
      else return DirectionLeft;
    }
    
    T match;
  };
  
  /**
   * A concrete [BestQuery] which matches values close to a specified value.
   */
  class ComparatorBestQuery : public Query {
  public:
    /**
     * Create a comparator which matches either above or below a given value.
     * 
     * If [_above] is `true`, all values above [_match] will be acceptable.
     * Otherwise, all values below [_match] will be acceptable.
     *
     * If and only if [_equal] is `true`, [_match] will be acceptable.
     */
    ComparatorBestQuery(const T & _match, bool _above, bool _equal)
      : match(_match), above(_above), equal(_equal) {}
    
    virtual bool Accepts(const T & value) const {
      if (equal && (value == match)) return true;
      if (above) {
        return value > match;
      } else {
        return value < match;
      }
    }
    
    virtual const T & Better(const T & value1, const T & value2) const {
      assert(Accepts(value1));
      assert(Accepts(value2));
      if (above) {
        return value1 < value2 ? value1 : value2;
      } else {
        return value1 > value2 ? value1 : value2;
      }
    }
    
    virtual Direction Next(const T & value) const {
      if (value == match) {
        if (equal) {
          return DirectionFound;
        } else if (above) {
          return DirectionRight;
        } else {
          return DirectionLeft;
        }
      } else if (match > value) {
        return DirectionRight;
      } else {
        return DirectionLeft;
      }
    }
    
    T match;
    bool above;
    bool equal;
  };
  
private:
  VirtualAllocator & allocator;
};

}

#endif
