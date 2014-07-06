#ifndef __ANALLOC2_TREE_H__
#define __ANALLOC2_TREE_H__

#include "path.hpp"

namespace ANAlloc {

typedef enum {
  NodeTypeFree,
  NodeTypeData,
  NodeTypeContainer
} NodeType;

/**
 * This is an interface for a binary tree. My goal with this class is to
 * provide a simple API for a binary tree that still allows for algorithmic
 * optimization.
 */
class Tree {
public:
  /**
   * Return the depth of this binary tree. A tree of depth 0 is invalid, so
   * this will probably range from 1 to 63.
   */
  virtual int GetDepth() = 0;
  
  /**
   * Set the node-type at a path. The node's parent must be a container before
   * you call this method (unless, of course, the node is the root node).
   *
   * If you set a node to be a container, the node's type is incomplete until
   * at least one of its children are not free. If you set one child's type but
   * not the other's, the other node will be assigned the free type.
   *
   * If you want to change a container node's type, you must first free both of
   * its children or use the Free() method.
   */
  virtual void SetType(Path p, NodeType type) = 0;
  
  /**
   * Get the type of a node at a path. The node's parent must be a container
   * or the node just like with SetType().
   */
  virtual NodeType GetType(Path p) = 0;
  
  /**
   * Find a free node which is at most a certain depth. This method may provide
   * optimizations that any given tree algorithm has to offer.
   * @return false if no desired nodes are available, true otherwise.
   */
  virtual bool FindFree(int depth, Path & pathOut) = 0;
  
  /**
   * Recursively free every path in the tree starting at a given path. This may
   * run slower than it needs to for certain tree representations, so I suggest
   * overriding it in your subclass.
   */
  virtual void Free(Path p);
  
  /**
   * Recursively free every path in the tree starting at the root.
   */
  virtual void FreeAll();
};

}

#endif
