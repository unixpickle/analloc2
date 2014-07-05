#include "btree.hpp"

namespace ANAlloc {

size_t BTree::MemorySize(int depth) {
  if (depth <= 3) return 1;
  return (1 << (depth - 3));
}

BTree::BTree() {
}

BTree::BTree(int _depth, uint8_t * bmMemory)
  : bitmap(bmMemory, (1 << _depth) - 1), depth(_depth) {
  assert(depth > 0);
}

BTree::BTree(const BTree & t) {
  *this = t;
}

BTree & BTree::operator=(const BTree & aTree) {
  bitmap = aTree.bitmap;
  depth = aTree.depth;
  return *this;
}

int BTree::GetDepth() {
  return depth;
}

void BTree::SetType(Path path, NodeType type) {
  switch ((int)type) {
    case NodeTypeFree:
      bitmap.SetBit(path.TreeIndex(), false);
      break;
    case NodeTypeData:
      if (path.GetDepth() != depth - 1) {
        Path right = PathRight(path);
        uint64_t right = path.Right().TreeIndex();
        bitmap.SetBit(right, false);
        bitmap.SetBit(right - 1, false);
      }
    case NodeTypeContainer:
      bitmap.SetBit(path, true);
      break;
    default:
      break;
  }
}

NodeType BTree::GetType(Path path) {
  bool bit = bitmap.GetBit(path.TreeIndex());
  if (!bit) return NodeTypeFree;
  if (path.GetDepth() == depth - 1) {
    // base nodes *cannot* be containers
    return NodeTypeData;
  }
  
  // if neither child is set, this is a data node; otherwise, it's a container.
  uint64_t leftIdx = path.Left().TreeIndex();
  if (bitmap.GetBit(leftIdx)) return NodeTypeContainer;
  if (bitmap.GetBit(leftIdx + 1)) return NodeTypeContainer;
  return NodeTypeData;
}

bool BTree::FindFree(int depth, Path & path) {
  return FindFreeRecursive(depth, Path::RootPath(), path);
}

bool BTree::FindFreeRecursive(int depth, Path p, Path & path) {
  NodeType type = GetType(p);
  if (type == NodeTypeFree) {
    path = p;
    return true;
  } else if (type == NodeTypeData) {
    return false;
  }
  if (pathDepth == depth) return false;
  
  if (FindFreeRecursive(depth, p.Left(), path)) {
    return true;
  }
  return FindFreeRecursive(depth, p.Right(), path);
}

}
