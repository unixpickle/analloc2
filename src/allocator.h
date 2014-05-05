#ifndef __ANALLOC2_ALLOC_H__
#define __ANALLOC2_ALLOC_H__

#include "tree.h"
#include <cassert>

namespace ANAlloc {

/**
 * This class wraps a Tree subclass. It provides a basic mechanism for
 * allocating, freeing, finding, and splitting nodes.
 */
template <class T>
class Allocator {
protected:
  T * tree;

public:
  Allocator() {
    tree = NULL;
  }
  
  Allocator(const Allocator<T> & a) {
    tree = a.tree;
  }
  
  Allocator(T * aTree) {
    tree = aTree;
  }
  
  Allocator & operator=(const Allocator & alloc) {
    tree = alloc.tree;
    return *this;
  }
  
  /**
   * Allocate a path at a certain `depth`. If the allocation can be completed,
   * true is returned and `path` is set.
   */
  bool Alloc(int depth, Path & path) {
    int foundDepth = 0;
    if (!tree->FindFree(depth, path)) {
      return false;
    }
    foundDepth = ANAlloc::PathDepth(path);
    while (foundDepth < depth) {
      tree->SetType(path, T::NodeTypeContainer);
      
      ANAlloc::Path right = PathRight(path);
      tree->SetType(right, T::NodeTypeFree);
      path = right - 1;
      foundDepth++;
    }
    tree->SetType(path, T::NodeTypeData);
    return true;
  }
  
  /**
   * Frees a node allocated at a path. The behavior of this function is
   * undefined if no node is allocated at `path`.
   */
  void Free(Path path) {
    assert(tree->GetType(path) == T::NodeTypeData);
    tree->SetType(path, T::NodeTypeFree);
    
    int depth = PathDepth(path);
    for (int i = 0; i < depth; i++) {
      if (tree->GetType(PathSibling(path)) != T::NodeTypeFree) break;
      path = PathParent(path);
      tree->SetType(path, T::NodeTypeFree);
    }
  }
  
  /**
   * Splits the allocated node at `path`, resulting in the two children being
   * allocated separately. This may be used to shorten a chunk of allocated
   * memory.
   */
  void Split(Path path) {
    assert(tree->GetType(path) == T::NodeTypeData);
    tree->SetType(path, T::NodeTypeContainer);
    Path right = PathRight(path);
    tree->SetType(right, T::NodeTypeData);
    tree->SetType(right - 1, T::NodeTypeData);
  }
  
  /**
   * Let's say you get an address and you want to calculate what its size is.
   * This depends on the depth of the memory allocated. For instance, if the
   * pointer is 0x0, and that's where your memory region starts, then you might
   * have a buffer of size 1 block, or of size 2 blocks, or 4 blocks, etc.
   *
   * This function allows you to provide the node index as if a block of memory
   * were at the lowest depth in the tree, and it will figure out what greater
   * chunk of memory actually contains the allocated pointer.
   *
   * @param idx The index of a path in the lowest allowed depth. This will be a
   * number from 0 to 2^n-1 (inclusive) where n is the depth of the tree.
   * @param path On successful search, this will be set to the path found.
   * @return true if the index belongs to an allocated node; false otherwise.
   */
  bool Find(uintptr_t idx, Path & path) {
    Path p = 0;
    uintptr_t theIdx = 0;
    for (int d = 0; d < tree->Depth(); d++) {
      typename T::NodeType t = tree->GetType(p);
      if (t == T::NodeTypeFree) {
        return false;
      } else if (t == T::NodeTypeData) {
        path = p;
        return true;
      }
      
      if (d + 1 == tree->Depth()) break;
      
      // go right only if we don't pass it
      uintptr_t nodeSize = 1 << (tree->Depth() - d - 2); // size of lower node
      if (nodeSize + theIdx > idx) {
        p = PathLeft(p);
      } else {
        p = PathRight(p);
        theIdx += idx;
      }
    }
    return false;
  }
  
  /**
   * For a node allocated at a certain path, keep splitting and freeing it so
   * that we may keep the base nodes starting at `idx` going for `count` nodes.
   */
  void Reserve(Path path, uintptr_t idx, uintptr_t count) {
    int depth = PathDepth(path);
    uintptr_t index = PathIndex(path);
    uintptr_t baseCount = 1L << (tree->Depth() - depth - 1);
    
    // check if we are completely contained by the base range
    if (index * baseCount >= idx && (index + 1) * baseCount <= idx + count) {
      return;
    }
    
    // check if we are completely outside the base range
    if ((index + 1) * baseCount <= idx) {
      Free(path);
      return;
    } else if (index * baseCount >= count + idx) {
      Free(path);
      return;
    }
    
    // split and recurse
    assert(depth != tree->Depth() - 1);
    Split(path);
    Reserve(PathLeft(path), idx, count);
    Reserve(PathRight(path), idx, count);
  }
};

}

#endif
