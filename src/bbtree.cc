#include "bbtree.h"

namespace ANAlloc {

static int _Log2Floor(int n) {
  for (int i = 0; i < n; i++) {
    if ((1 << (i + 1)) >= n) {
      return i;
    }
  }
  return -1;
}

static uintptr_t _PathBitOff(Path p, int depthCount) {
  int depth = PathDepth(p);
  uintptr_t result = 0;
  for (int i = 0; i < depth; i++) {
    int numBits = _Log2Floor(depthCount - i) + 1;
    result += (uintptr_t)numBits << i;
  }
  
  uintptr_t index = PathIndex(p);
  int perNode = _Log2Floor(depthCount - depth) + 1;
  return result + (1L << depth) * (uintptr_t)perNode * index;
}

size_t BBTree::MemorySize(int depth) {
  uintptr_t bitCount = _PathBitOff((1L << depth) - 1L, depth) + 1;
  return (bitCount >> 3) + (bitCount & 7 ? 1 : 0);
}

BBTree::BBTree(int _depth, uint8_t * bmMemory)
  : bitmap(bmMemory, MemorySize(_depth)) {
  assert(_depth > 0);
  depth = _depth;
}

int BBTree::Depth() {
  return depth;
}

void BBTree::SetType(Path path, NodeType type) {
  // TODO: this
  switch ((int)type) {
    case NodeTypeFree:
      break;
    case NodeTypeData:
      break;
    case NodeTypeContainer:
      break;
    default:
      break;
  }
}

BBTree::NodeType BBTree::GetType(Path path) {
  // TODO: this
  return 0;
}

bool BBTree::FindFree(int depth, Path & path) {
  // TODO: this
  return false;
}

}