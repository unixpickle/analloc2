#include "../src/bbtree.hpp"
#include "tree_test.hpp"

int main() {
  assert(ANAlloc::BBTree::MemorySize(1) == 1);
  assert(ANAlloc::BBTree::MemorySize(2) == 1);
  assert(ANAlloc::BBTree::MemorySize(3) == 2);
  assert(ANAlloc::BBTree::MemorySize(4) == 3);
  assert(ANAlloc::BBTree::MemorySize(5) == 7);
  
  size_t size = ANAlloc::BBTree::MemorySize(3);
  //assert(size == 209);
  
  uint8_t * memory = new uint8_t[size];
  ANAlloc::BBTree tree(3, memory);
  
  TestTreeSetGet(tree, "BBTree");
  TestTreeStrictTypes(tree, "BTree");
  TestTreeFindFree(tree, "BBTree");
  
  delete memory;
  return 0;
}
