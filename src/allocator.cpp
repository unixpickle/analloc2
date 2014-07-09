#include "malloc.hpp"

namespace ANAlloc {

Allocator::Allocator(UInt _start, Tree & _tree, int _psLog)
  : start(_start), length(1UL << (_tree.GetDepth() - 1 + _psLog)),
    tree(_tree), psLog(_psLog) {
}

bool Allocator::Alloc(UInt size, UInt & result) {
  int depth = DepthForSize(size);
  if (depth < 0) return false;
  
  Path p;
  bool res = tree.Alloc(depth, p);
  if (!res) return false;
  
  // compute the resultant address
  UInt nodeSize = 1UL << (psLog + (tree.GetDepth() - depth - 1));
  result = start + nodeSize * p.GetIndex();
  return true;
}

bool Allocator::Align(UInt size, UInt align, UInt & result) {
  if (start % align) {
    return false;
  }
  int sizeDepth = DepthForSize(size);
  int alignDepth = DepthForSize(align);
  if (sizeDepth < 0 || alignDepth < 0) {
    return false;
  }
  
  Path p;
  bool res = tree.Align(sizeDepth, alignDepth, p);
  if (!res) return false;
  
  // compute the resultant address
  UInt nodeSize = 1UL << (psLog + (tree.GetDepth() - sizeDepth - 1));
  result = start + nodeSize * p.GetIndex();
  return true;
}

void Allocator::Free(UInt addr) {
  UInt baseIndex = (addr - start) >> psLog;
  
  Path p;
  bool res = tree.FindByShadow(baseIndex, p);
  assert(res);
  tree.Dealloc(p);
}

bool Allocator::OwnsAddress(UInt addr) const {
  return addr >= start && addr < start + length;
}

const Tree & Allocator::GetTree() const {
  return tree;
}

int Allocator::GetPageSizeLog() const {
  return psLog;
}

// PROTECTED //

int Allocator::DepthForSize(UInt size) const {
  int power = Log2Ceil(size) - psLog;
  if (power < 0) power = 0;
  return tree.GetDepth() - power - 1;
}

}
