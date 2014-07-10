#ifndef __ANALLOC2_BBTREE_HPP__
#define __ANALLOC2_BBTREE_HPP__

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
  
  virtual int GetDepth() const;
  virtual void SetType(Path path, NodeType type);
  virtual NodeType GetType(Path path) const;
  virtual bool FindFree(int depth, Path & pathOut) const;
  virtual bool FindAligned(int depth, int align, Path & pathOut) const;
  virtual void Free(Path path);
  virtual int AllocHeuristic();
  
  bool IsFree(Path path) const;
  
protected:
  Bitmap bitmap;
  int depth;
#ifndef ANALLOC_BBTREE_DONT_CACHE_PREFIXES
  UInt prefixSizes[0x40];
#endif
  
  static UInt TreeSizeAtDepth(int depth);
  int FieldSizeAtDepth(int depth) const;
  UInt CalculatePrefixSize(int depth) const;
  UInt GetPrefixSize(int depth) const;
  
  int ReadNode(Path p) const;
  void WriteNode(Path p, int value);
  void UpdateParents(Path p, int pValue);
  
  bool RecursiveFindAligned(int depth, int align, Path path,
                            Path & pathOut) const;
};

}

#endif
