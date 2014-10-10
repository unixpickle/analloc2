#ifndef __ANALLOC2_DYNAMIC_BINARY_TREE_HPP__
#define __ANALLOC2_DYNAMIC_BINARY_TREE_HPP__

#include "dynamic-tree.hpp"

namespace analloc {

template <class T>
class DynamicBinaryTree : public DynamicTree<T> {
public:
  using typename DynamicTree<T>::Query;
  
  class Node {
  public:
    Node(const T & _value) : value(_value) {}
    
    virtual Node * GetLeft() = 0;
    virtual Node * GetRight() = 0;
    virtual const Node * GetLeft() const = 0;
    virtual const Node * GetRight() const = 0;
    
    inline const T & GetValue() const {
      return value;
    }
    
  protected:
    T value;
  };
  
  /**
   * Return the root node, or `nullptr` if there is no root node.
   */
  virtual Node * GetRoot() = 0;
  
  virtual bool Search(T * result, const Query & query, bool remove = false) {
    Node * node = GetRoot();
    while (node) {
      Query::Direction direction = query.Next(node->GetValue());
      if (direction == Query::DirectionFound) {
        if (result) {
          (*result) = node->GetValue();
        }
        if (remove) {
          Remove(node->GetValue());
        }
        return true;
      } else if (direction == Query::DirectionLeft) {
        node = node->GetLeft();
      } else {
        node = node->GetRight();
      }
    }
    return false;
  }
  
  virtual bool SearchBest(T * result, const BestQuery & query,
                          bool remove = false) {
    if (remove) {
      T theValue;
      if (!SearchBest(GetRoot(), &theValue, query)) {
        return false;
      }
      if (result) {
        (*result) = theValue;
      }
      Remove(theValue);
    } else {
      return SearchBestFrom(GetRoot(), result, query);
    }
  }
  
protected:
  virtual bool SearchBestFrom(Node * node, T * result,
                              const BestQuery & query) {
    if (!node) {
      return false;
    }
    // Check which direction to branch off in for the search
    Query::Direction direction = query.Next(node->GetValue());
    if (direction == Query::DirectionFound) {
      // If the node was found, no more branching necessary.
      if (node) {
        (*node) = node->GetValue();
      }
      return true;
    } else {
      // Figure out the next node to visit for the search
      Node * next;
      if (direction == Query::DirectionLeft) {
        next = node->GetLeft();
      } else {
        next = node->GetRight();
      }
      // If [node] contains an acceptable value, we might end up using it
      if (query.Accepts(node->GetValue())) {
        if (!result) {
          // If they were just trying to see if the tree contains an acceptable
          // value, there's no need to search more.
          return true;
        }
        if (SearchBestFrom(next, result, query)) {
          (*result) = query.Better(*result, node->GetValue());
        } else {
          (*result) = node->GetValue();
        }
        return true;
      } else {
        // Use whatever best value is found in the subnode
        return SearchBestFrom(next, result, query);
      }
    }
  }
};

}

#endif
