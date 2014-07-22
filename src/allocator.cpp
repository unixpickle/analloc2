#include "malloc.hpp"

namespace ANAlloc {

Allocator::Allocator(UInt _start, Tree & _tree, int _psLog)
  : start(_start), length(1UL << (_tree.GetDepth() - 1 + _psLog)),
    tree(_tree), psLog(_psLog), freeSize(length) {
  assert(tree.GetType(Path::Root()) == NodeTypeFree);
}

void Allocator::Reserve(UInt resStart, UInt resSize) {
  assert(resSize + resStart <= length);
  
  UInt blockStart = resStart >> psLog;
  UInt blockEnd = (resStart + resSize) >> psLog;
  if (blockEnd << psLog != resStart + resSize) {
    ++blockEnd;
  }
  UInt blockCount = blockEnd - blockStart;
  
  assert(tree.GetType(Path::Root()) == NodeTypeFree);
  tree.SetType(Path::Root(), NodeTypeData);
  tree.Carve(Path::Root(), blockStart, blockCount);
  
  freeSize -= blockCount << psLog;
}

bool Allocator::Alloc(UInt size, UInt & result) {
  int depth = DepthForSize(size);
  if (depth < 0) return false;
  
  if (tree.AllocHeuristic() > depth) return false;
  
  Path p;
  bool res = tree.Alloc(depth, p);
  if (!res) return false;
  
  // compute the resultant address
  UInt nodeSize = 1UL << (psLog + (tree.GetDepth() - depth - 1));
  freeSize -= nodeSize;
  result = start + nodeSize * p.GetIndex();
  return true;
}

bool Allocator::Align(UInt size, UInt align, UInt & result) {
  if (start % align) {
    return false;
  }
  
  int sizeDepth = DepthForSize(size);
  int alignDepth = DepthForSize(align);
  
  // if they are trying to allocate this entire allocator and align it by
  // whatever the allocator is aligned to, why stop them?
  if (sizeDepth < 0) return false;
  if (alignDepth < 0) alignDepth = 0;
  
  if (tree.AllocHeuristic() > sizeDepth) return false;
  
  Path p;
  bool res = tree.Align(sizeDepth, alignDepth, p);
  if (!res) return false;
  
  // compute the resultant address
  UInt nodeSize = 1UL << (psLog + (tree.GetDepth() - sizeDepth - 1));
  freeSize -= nodeSize;
  result = start + nodeSize * p.GetIndex();
  return true;
}

void Allocator::Free(UInt addr) {
  UInt baseIndex = (addr - start) >> psLog;
  
  Path p;
  bool res = tree.FindByShadow(baseIndex, p);
  assert(res);
  tree.Dealloc(p);
  
  UInt nodeSize = 1UL << (psLog + (tree.GetDepth() - p.GetDepth() - 1));
  freeSize += nodeSize;
}

bool Allocator::OwnsAddress(UInt addr) const {
  return addr >= start && addr < start + length;
}

UInt Allocator::GetFreeSize() const {
  return freeSize;
}

UInt Allocator::GetTotalSize() const {
  return length;
}

UInt Allocator::GetStart() const {
  return start;
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
