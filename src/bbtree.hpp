#ifndef __ANALLOC2_BBTREE_H__
#define __ANALLOC2_BBTREE_H__

#include "tree.hpp"
#include "bitmap.hpp"

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
public:
  static size_t MemorySize(int depth);
  
  BBTree(); // for placement-new placeholder only
  BBTree(int depth, uint8_t * bmMemory);
  BBTree(const BBTree & tree);
  BBTree & operator=(const BBTree & tree);
  
  virtual int GetDepth();
  virtual void SetType(Path path, NodeType type);
  virtual NodeType GetType(Path path);
  virtual bool FindFree(int depth, Path & path);
  
protected:
  Bitmap bitmap;
  int depth;
#ifndef ANALLOC_BBTREE_DONT_CACHE_PREFIXES
  uint64_t prefixSizes[0x40];
#endif
  
  static uint64_t TreeSizeAtDepth(int depth);
  int FieldSizeAtDepth(int depth);
  uint64_t CalculatePrefixSize(int depth);
  uint64_t GetPrefixSize(int depth);
  
  uint64_t NodeOffset(Path p);
  int ReadNode(Path p);
  void WriteNode(Path p, int value);
};

}

#endif
