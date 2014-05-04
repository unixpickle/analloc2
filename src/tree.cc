#include "tree.h"

namespace ANAlloc {

Path PathParent(Path path) {
  if (path == 0) return 0;
  
  int depth = PathDepth(path);
  
  uintptr_t localIdx = (path + 1) ^ (1 << depth);
  uintptr_t newIdx = localIdx >> 1;
  return (newIdx | (1L << (depth - 1))) - 1;
}

Path PathSibling(Path path) {
  if (path == 0) return 0;
  
  // odd global index = even local index = left node
  if (path & 1) return path + 1;
  return path - 1;
}

Path PathRight(Path path) {
  int depth = PathDepth(path);

  // get the index in this particular depth
  uintptr_t localIdx = (path + 1) ^ (1L << depth);

  // double the local index for the next depth
  uintptr_t nextIdx = localIdx << 1;
  return (nextIdx | (1L << (depth + 1))); // don't subtract 0 for right
}

Path PathLeft(Path path) {
  int depth = PathDepth(path);

  // get the index in this particular depth
  uintptr_t localIdx = (path + 1) ^ (1L << depth);

  // double the local index for the next depth
  uintptr_t nextIdx = localIdx << 1;
  return (nextIdx | (1L << (depth + 1))) - 1;
}

int PathDepth(Path path) {
  int depth;
  for (depth = 63; depth >= 0; depth--) {
    if ((path + 1) & (1L << depth)) break;
  }
  return depth;
}

uintptr_t PathIndex(Path path) {
  int depth = PathDepth(path);
  return (path + 1) ^ (1L << depth);
}

uintptr_t PathsForDepth(int d) {
  return (1L << d);
}

}