#include "bbtree.h"

namespace ANAlloc {

static int _Log2Ceil(int n) {
  assert(n > 0);
  for (int i = 0; i < n; i++) {
    if ((1 << i) >= n) {
      return i;
    }
  }
  return -1;
}

static uintptr_t _PathBitOff(Path p, int depthCount, int * _numBits) {
  int depth = PathDepth(p);
  uintptr_t result = 0;
  for (int i = 0; i < depth; i++) {
    int numBits = _Log2Ceil(depthCount - i + 1);
    result += (uintptr_t)numBits << i;
  }
  
  uintptr_t index = PathIndex(p);
  int perNode = _Log2Ceil(depthCount - depth + 1);
  if (_numBits) *_numBits = perNode;
  return result + (uintptr_t)perNode * index;
}

uintptr_t BBTree::BitCount(int depth) {
  return _PathBitOff((1L << depth) - 1L, depth, NULL);
}

size_t BBTree::MemorySize(int depth) {
  uintptr_t bitCount = BitCount(depth);
  return (bitCount >> 3) + (bitCount & 7 ? 1 : 0);
}

BBTree::BBTree(int _depth, uint8_t * bmMemory)
  : bitmap(bmMemory, BitCount(_depth)) {
  assert(_depth > 0);
  depth = _depth;
  WriteNode(0, depth);
}

int BBTree::Depth() {
  return depth;
}

void BBTree::SetType(Path path, NodeType type) {
  switch ((int)type) {
    case NodeTypeFree:
      FreePath(path);
      break;
    case NodeTypeData:
      AllocPathData(path);
      break;
    case NodeTypeContainer:
      AllocPathContainer(path);
      break;
    default:
      break;
  }
}

BBTree::NodeType BBTree::GetType(Path path) {
  int value = ReadNode(path);
  if (!value) return NodeTypeData;
  if (value == Depth() - PathDepth(path)) return NodeTypeFree;
  return NodeTypeContainer;
}

bool BBTree::FindFree(int depth, Path & path) {
  return FindFreeRecursive(depth, 0, 0, path);
}

/** PRIVATE **/

int BBTree::ReadNode(Path p) {
  int numBits;
  uintptr_t bitIndex = _PathBitOff(p, Depth(), &numBits);
  return bitmap.GetMultibit(bitIndex, numBits);
}

void BBTree::WriteNode(Path p, int i) {
  int numBits;
  uintptr_t bitIndex = _PathBitOff(p, Depth(), &numBits);
  bitmap.SetMultibit(bitIndex, numBits, i);
}

void BBTree::FreePath(Path path) {
  int pathDepth = PathDepth(path);
  int biggestValue = Depth() - pathDepth;
  WriteNode(path, biggestValue);
  
  // iterate up and make sure the sizes are at least this
  Path p = path;
  for (int i = 0; i < pathDepth; i++) {
    p = PathParent(p);
    if (ReadNode(p) < biggestValue) {
      WriteNode(p, biggestValue);
    }
  }
}

void BBTree::AllocPathData(Path path) {
  Path p = path;
  
  // the path is now closed for business
  WriteNode(p, 0);
  
  // update parents' sizes
  UpdateParents(p);
}

void BBTree::AllocPathContainer(Path path) {
  Path p = path;
  
  int aDepth = PathDepth(p);
  assert(aDepth + 1 != Depth());
  
  int curSize = Depth() - (aDepth + 1);
  WriteNode(p, curSize);
  
  // update parents' sizes
  UpdateParents(p);
}

void BBTree::UpdateParents(Path p) {
  // update parents' sizes
  int depth = PathDepth(p);
  for (int i = 0; i < depth; i++) {
    int val1 = ReadNode(PathSibling(p));
    int val2 = ReadNode(p);
    p = PathParent(p);
    WriteNode(p, val1 > val2 ? val1 : val2);
  }
}

bool BBTree::FindFreeRecursive(int depth,
                               int curDepth,
                               Path p,
                               Path & path) {
  if (curDepth == depth) {
    if (GetType(p) != NodeTypeFree) return false;
    path = p;
    return true;
  }
  
  int available = ReadNode(p);
  int minSize = Depth() - depth;
  if (available < minSize) return false;
  
  int nodeSize = Depth() - curDepth;
  if (nodeSize == available) {
    // we found a free node
    path = p;
    return true;
  }
  
  // find a subnode that is suitable
  Path left = PathLeft(p);
  if (FindFreeRecursive(depth, curDepth + 1, left, path)) {
    return true;
  }
  return FindFreeRecursive(depth, curDepth + 1, left + 1, path);
}

}