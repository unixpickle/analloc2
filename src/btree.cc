#include "btree.h"

namespace ANAlloc {

size_t BTree::MemorySize(int depth) {
  size_t bitCount = (1 << depth);
  return (bitCount >> 3) + (bitCount & 7 ? 1 : 0);
}

BTree::BTree(int _depth, uint8_t * bmMemory) : bitmap(bmMemory, _depth) {
  assert(_depth > 0);
  depth = _depth;
}

int BTree::Depth() {
  return depth;
}

void BTree::SetType(Path path, NodeType type) {
  switch (type) {
    NodeTypeFree:
      bitmap.SetBit(path, false);
      break;
    NodeTypeData:
    NodeTypeContainer:
      bitmap.SetBit(path, false);
      bitmap.SetBit(path, false);
      break;
    default:
      break;
  }
}

BTree::NodeType BTree::GetType(Path path) {
  bool bit = bitmap.GetBit(path);
  if (!bit) return NodeTypeFree;
  if (PathDepth(path) == depth - 1) {
    // base nodes *cannot* be containers
    return NodeTypeData;
  }
  
  // if neither child is set, this is a data node; otherwise, it's a container.
  if (bitmap.GetBit(PathLeft(path))) return NodeTypeContainer;
  if (!bitmap.GetBit(PathRight(path))) return NodeTypeData;
  return NodeTypeContainer;
}

bool BTree::FindFree(int depth, Path & path) {
  return FindFreeRecursive(depth, 0, 0, path);
}

bool BTree::FindFreeRecursive(int depth, int pathDepth, Path p, Path & path) {
  NodeType type = GetType(p);
  if (type == NodeTypeFree) {
    path = p;
    return true;
  } else if (type == NodeTypeData) {
    return false;
  }
  if (pathDepth == depth) return false;
  
  if (FindFreeRecursive(depth, pathDepth + 1, PathLeft(p), path)) {
    return true;
  }
  return FindFreeRecursive(depth, pathDepth + 1, PathRight(p), path);
}

}