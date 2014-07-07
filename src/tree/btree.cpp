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
  bitmap.SetBit(0, false);
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
    case NodeTypeContainer:
      bitmap.SetBit(path.TreeIndex(), true);
      if (path.GetDepth() != depth - 1) {
        uint64_t right = path.Right().TreeIndex();
        bitmap.SetBit(right, false);
        bitmap.SetBit(right - 1, false);
      }
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

bool BTree::FindAligned(int depth, int align, Path & path) {
  return FindFreeRecursive(depth, align, Path::Root(), path);
}

bool BTree::FindFreeRecursive(int _depth, int align, Path p, Path & pathOut) {
  NodeType type = GetType(p);
  if (type == NodeTypeFree) {
    pathOut = p;
    return true;
  } else if (type == NodeTypeData) {
    return false;
  }
  if (_depth == p.GetDepth()) return false;
  
  if (FindFreeRecursive(_depth, align, p.Left(), pathOut)) {
    return true;
  } else if (p.GetDepth() < align) {
    return FindFreeRecursive(_depth, align, p.Right(), pathOut);
  } else {
    return false;
  }
}

}
