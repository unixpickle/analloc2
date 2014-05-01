#ifndef __ANALLOC2_BTREE_H__
#define __ANALLOC2_BTREE_H__

#include "tree.h"
#include "bitmap.h"

namespace ANAlloc {

/**
 * This is a simple binary tree. Finding an empty node could take up to O(n)
 * where n is 2^depth. Freeing is O(log(n)). Nicely, the number of bits
 * consumed is 2*n.
 */
class BTree : public Tree {
private:
  Bitmap bitmap;
  int depth;

public:
  typedef typename Tree::NodeType NodeType;
  
  static MemorySize(int depth);
  
  BTree(int depth, uint8_t * bmMemory);
  
  int Depth();
  void SetType(Path path, NodeType type);
  NodeType GetType(Path path);
};

}

#endif
