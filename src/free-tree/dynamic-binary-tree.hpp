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
          RemoveNode(node);
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
    // TODO: respect `remove` argument
    return SearchBestFrom(GetRoot(), result, query, remove);
  }
  
protected:
  virtual bool SearchBestFrom(Node * node, T * result,
                              const BestQuery & query, bool remove = false) {
    if (!node) {
      return false;
    }
    Query::Direction direction = query.Next(node->GetValue());
    if (direction == Query::DirectionFound) {
      if (node) {
        (*node) = node->GetValue();
      }
      return true;
    } else {
      Node * next;
      if (direction == Query::DirectionLeft) {
        next = node->GetLeft();
      } else {
        next = node->GetRight();
      }
      if (query.Accepts(node->GetValue())) {
        if (!result) {
          return true;
        }
        if (SearchBestFrom(result, query, remove, next)) {
          (*result) = query.Better(*result, node->GetValue());
        } else {
          (*result) = node->GetValue();
        }
        return true;
      } else {
        return SearchBestFrom(result, query, remove, next);
      }
    }
  }
};

}

#endif
