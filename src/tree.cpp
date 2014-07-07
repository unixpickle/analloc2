#include "tree.hpp"
#include <cassert>

namespace ANAlloc {

void Tree::Free(Path p) {
  NodeType type = GetType(p);
  if (type != NodeTypeFree) {
    if (type == NodeTypeContainer) {
      Free(p.Left());
      Free(p.Right());
    }
    SetType(p, NodeTypeFree);
  }
}

void Tree::FreeAll() {
  Free(Path(0, 0));
}

bool Tree::Alloc(int depth, Path & pathOut) {
  assert(depth < GetDepth());
  if (!FindFree(depth, pathOut)) return false;
  while (pathOut.GetDepth() < depth) {
    Split(pathOut);
    Free(pathOut.Right());
    pathOut = pathOut.Left();
  }
  return true;
}

bool Tree::FindByShadow(uint64_t baseIndex, Path & path) {
  uint64_t shadow = 0;
  uint64_t shadowSize = Path::DepthCount(GetDepth() - 1);
  path = Path::RootPath();
  
  while (1) {
    NodeType type = GetType(path);
    if (type == NodeTypeFree) return false;
    else if (type == NodeTypeData) return true;
    shadowSize >>= 2;
    if (shadow + shadowSize <= baseIndex) {
      shadow += shadowSize;
      path = path.Right();
    } else {
      path = path.Left();
    }
  }
}

void Tree::Reserve(Path p, uint64_t baseStart, uint64_t baseCount) {
  // At the moment, I have no reason to highly optimize this method because it
  // is not used profusely by my Operating System.
  uint64_t shadowSize = 1UL << (GetDepth() - p.GetDepth() - 1);
  uint64_t shadowStart = shadowSize * p.GetIndex();
  
  uint64_t shadowEnd = shadowSize + shadowStart;
  uint64_t baseEnd = baseStart + baseCount;
  
  if (shadowStart >= baseEnd || baseStart >= shadowEnd) {
    SetType(p, NodeTypeFree);
  } else if (shadowStart >= baseStart && shadowEnd <= baseEnd) {
    SetType(p, NodeTypeData);
  } else {
    SetType(p, NodeTypeContainer);
    Reserve(p.Left(), baseStart, baseCount);
    Reserve(p.Right(), baseStart, baseCount);
  }
}

}
