#include "malloc.hpp"

namespace ANAlloc {

Malloc::Malloc(uint8_t * _start, Tree & _tree, int _psLog)
  : start(_start), length(1UL << (_tree.GetDepth() - 1 + _psLog)),
    tree(_tree), psLog(_psLog) {
}

void * Malloc::Alloc(size_t size) {
  int depth = DepthForSize(size);
  if (depth < 0) return NULL;
  
  Path p;
  bool res = tree.Alloc(depth, p);
  if (!res) return NULL;
  
  // compute the resultant address
  UInt nodeSize = 1UL << (psLog + (tree.GetDepth() - depth - 1));
  return (void *)(start + nodeSize * p.GetIndex());
}

void * Malloc::Align(size_t size, size_t align) {
  if ((uintptr_t)start % align) {
    return NULL;
  }
  int sizeDepth = DepthForSize(size);
  int alignDepth = DepthForSize(align);
  if (sizeDepth < 0 || alignDepth < 0) {
    return NULL;
  }
  
  Path p;
  bool res = tree.Align(sizeDepth, alignDepth, p);
  if (!res) return NULL;
  
  // compute the resultant address
  UInt nodeSize = 1UL << (psLog + (tree.GetDepth() - sizeDepth - 1));
  return (void *)(start + nodeSize * p.GetIndex());
}

void Malloc::Free(void * buff) {
  UInt baseIndex = (UInt)(((uintptr_t)buff - (uintptr_t)start)
    >> psLog);
  
  Path p;
  bool res = tree.FindByShadow(baseIndex, p);
  assert(res);
  tree.Dealloc(p);
}

bool Malloc::OwnsPointer(void * ptr) const {
  if ((uintptr_t)ptr < (uintptr_t)start) return false;
  if ((uintptr_t)ptr >= (uintptr_t)start + length) return false;
  return true;
}

const Tree & Malloc::GetTree() const {
  return tree;
}

int Malloc::GetPageSizeLog() const {
  return psLog;
}

// PROTECTED //

int Malloc::DepthForSize(size_t size) const {
  int power = Log2Ceil(size) - psLog;
  if (power < 0) power = 0;
  return tree.GetDepth() - power - 1;
}

}
