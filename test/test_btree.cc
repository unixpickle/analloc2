#include "../src/btree.h"
#include "tree_test.h"

int main() {
  assert(ANAlloc::BTree::MemorySize(0) == 1);
  assert(ANAlloc::BTree::MemorySize(1) == 1);
  assert(ANAlloc::BTree::MemorySize(2) == 1);
  assert(ANAlloc::BTree::MemorySize(3) == 2);
  assert(ANAlloc::BTree::MemorySize(4) == 4);
  
  size_t size = ANAlloc::BTree::MemorySize(10);
  assert(size == (1 << 8));
  
  uint8_t * memory = new uint8_t[size];
  ANAlloc::BTree tree(10, memory);
  
  TestTreeSetGet<ANAlloc::BTree>(tree, "BTree");
  
  return 0;
}
