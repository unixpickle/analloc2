#ifndef __ANALLOC2_BBTREE_H__
#define __ANALLOC2_BBTREE_H__

#include "tree.h"
#include "bitmap.h"

namespace ANAlloc {

/**
 * This is a binary tree in which each node has a bitmap. The bitmap tells
 * which sized sub-blocks are available. Thus, for base nodes, the bitmap only
 * contains one bit, and the number of bits increases at each level.
 * 
 * This additional structure makes allocation O(log(n)), but the memory usage
 * is more taxing. Another downside is that allocations and frees take slightly
 * longer because their parent nodes have to have their bitmaps modified.
 */
class BBTree : public Tree {
private:
  Bitmap bitmap;
  int depth;

  static uintptr_t BitCount(int depth);

  int ReadNode(Path p);
  void WriteNode(Path p, int i);
  
  void FreePath(Path p);
  void AllocPathData(Path p);
  void AllocPathContainer(Path p);
  void UpdateParents(Path p);
  
  bool FindFreeRecursive(int depth, int curDepth, Path p, Path & path);

public:
  typedef Tree::NodeType NodeType;
  
  static size_t MemorySize(int depth);
  
  BBTree(int depth, uint8_t * bmMemory);
  
  int Depth();
  void SetType(Path path, NodeType type);
  NodeType GetType(Path path);
  bool FindFree(int depth, Path & path);
};

}

#endif
