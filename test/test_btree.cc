#include "../src/btree.hpp"
#include "tree_test.hpp"

int main() {
  assert(ANAlloc::BTree::MemorySize(1) == 1);
  assert(ANAlloc::BTree::MemorySize(2) == 1);
  assert(ANAlloc::BTree::MemorySize(3) == 1);
  assert(ANAlloc::BTree::MemorySize(4) == 2);
  assert(ANAlloc::BTree::MemorySize(5) == 4);
  assert(ANAlloc::BTree::MemorySize(6) == 8);
  
  size_t size = ANAlloc::BTree::MemorySize(10);
  assert(size == (1 << 7));
  
  uint8_t * memory = new uint8_t[size];
  ANAlloc::BTree tree(10, memory);
  
  TestTreeSetGet(tree, "BTree");
  TestTreeStrictTypes(tree, "BTree");
  TestTreeFindFree(tree, "BTree");
  
  delete memory;
  return 0;
}
