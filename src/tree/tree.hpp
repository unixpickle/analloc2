#ifndef __ANALLOC2_TREE_HPP__
#define __ANALLOC2_TREE_HPP__

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
  virtual int GetDepth() const = 0;
  
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
  virtual NodeType GetType(Path p) const = 0;
  
  /**
   * Find a free node which is at most a certain depth. By default, this uses
   * FindFreeAlign(depth, depth, pathOut).
   * @return false if no desired nodes are available, true otherwise.
   */
  virtual bool FindFree(int depth, Path & pathOut) const;
  
  /**
   * Find a free node which is at most a given depth. The resultant node must
   * be aligned by `align`, meaning that it must start at the same shadow index
   * as a node at `align` depth or higher.
   */
  virtual bool FindAligned(int depth, int align, Path & pathOut) const = 0;
  
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
  
  /**
   * Allocate a node at a ceratin depth. This will recursively set the type of
   * the node's parents if needed to create a node of exactly the right size.
   */
  virtual bool Alloc(int depth, Path & pathOut);
  
  /**
   * Allocate a node at a certain depth that only branches to the left starting
   * at another depth `depthAlign`.
   */
  virtual bool Align(int depth, int align, Path & pathOut);
  
  /**
   * Free a data node. If the node's sibling is also free, the node's parent
   * will be free'd, etc.
   */
  virtual void Dealloc(Path p);
  
  /**
   * Find a data node given its shadow on the base row. If no data node exists,
   * false will be returned.
   */
  virtual bool FindByShadow(UInt baseIndex, Path & path) const;
  
  /**
   * Carve a shadow on the base row by recursively splitting a data node and
   * freeing unneeded residual.
   */
  virtual void Carve(Path p, UInt baseStart, UInt baseCount);
  
  /**
   * Returns the highest depth that could possibly contain a free node. For
   * example, if this returns 1, you will not be able to allocate the root
   * node. If it returns 2, you will only be able to allocate nodes of depth 2
   * or more.
   *
   * The default implementation of this method returns 0.
   */
  virtual int AllocHeuristic();
};

}

#endif
