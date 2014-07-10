#include "tree.hpp"
#include <cassert>

namespace ANAlloc {

bool Tree::FindFree(int depth, Path & pathOut) const {
  return FindAligned(depth, depth, pathOut);
}

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
    SetType(pathOut, NodeTypeContainer);
    SetType(pathOut.Right(), NodeTypeFree);
    pathOut = pathOut.Left();
  }
  SetType(pathOut, NodeTypeData);
  return true;
}

bool Tree::Align(int depth, int align, Path & pathOut) {
  assert(depth < GetDepth());
  if (!FindAligned(depth, align, pathOut)) return false;
  while (pathOut.GetDepth() < depth) {
    SetType(pathOut, NodeTypeContainer);
    SetType(pathOut.Right(), NodeTypeFree);
    pathOut = pathOut.Left();
  }
  SetType(pathOut, NodeTypeData);
  return true;
}

void Tree::Dealloc(Path p) {
  assert(GetType(p) == NodeTypeData);
  Path path = p;
  SetType(path, NodeTypeFree);
  while (path.GetDepth()) {
    if (GetType(path.Sibling()) != NodeTypeFree) {
      break;
    }
    path = path.Parent();
    SetType(path, NodeTypeFree);
  }
}

bool Tree::FindByShadow(UInt baseIndex, Path & path) const {
  UInt shadow = 0;
  UInt shadowSize = Path::DepthCount(GetDepth() - 1);
  
  path = Path::Root();
  
  while (1) {
    NodeType type = GetType(path);
    if (type == NodeTypeFree) return false;
    else if (type == NodeTypeData) return true;
    shadowSize >>= 1;
    if (shadow + shadowSize <= baseIndex) {
      shadow += shadowSize;
      path = path.Right();
    } else {
      path = path.Left();
    }
  }
}

void Tree::Carve(Path p, UInt baseStart, UInt baseCount) {
  // At the moment, I have no reason to highly optimize this method because it
  // is not used profusely by my Operating System.
  UInt shadowSize = 1UL << (GetDepth() - p.GetDepth() - 1);
  UInt shadowStart = shadowSize * p.GetIndex();
  
  UInt shadowEnd = shadowSize + shadowStart;
  UInt baseEnd = baseStart + baseCount;
  
  if (shadowStart >= baseEnd || baseStart >= shadowEnd) {
    SetType(p, NodeTypeFree);
  } else if (shadowStart >= baseStart && shadowEnd <= baseEnd) {
    SetType(p, NodeTypeData);
  } else {
    SetType(p, NodeTypeContainer);
    Carve(p.Left(), baseStart, baseCount);
    Carve(p.Right(), baseStart, baseCount);
  }
}

int Tree::AllocHeuristic() {
  return 0;
}

}
