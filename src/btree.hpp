#ifndef __ANALLOC2_BTREE_H__
#define __ANALLOC2_BTREE_H__

#include "tree.hpp"
#include "bitmap.hpp"

namespace ANAlloc {

/**
 * This is a simple binary tree. Finding an empty node could take up to O(n)
 * where n is 2^depth. Freeing is O(log(n)). Nicely, the number of bits
 * consumed is 2*n.
 */
class BTree : public Tree {
public:
  static size_t MemorySize(int depth);
  
  BTree(); // for placement-new placeholder only
  BTree(int depth, uint8_t * bmMemory);
  
  BTree(const BTree & t);
  BTree & operator=(const BTree & aTree);
  
  virtual int GetDepth();
  virtual void SetType(Path path, NodeType type);
  virtual NodeType GetType(Path path);
  virtual bool FindFree(int depth, Path & path);

private:
  Bitmap bitmap;
  int depth;

  bool FindFreeRecursive(int depth, Path p, Path & path);
};

}

#endif
