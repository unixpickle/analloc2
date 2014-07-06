#include "../src/btree.hpp"
#include "tree-tester.hpp"

using namespace ANAlloc;

int main() {
  assert(BTree::MemorySize(1) == 1);
  assert(BTree::MemorySize(2) == 1);
  assert(BTree::MemorySize(3) == 1);
  assert(BTree::MemorySize(4) == 2);
  assert(BTree::MemorySize(5) == 4);
  assert(BTree::MemorySize(6) == 8);
  
  size_t size = BTree::MemorySize(10);
  assert(size == (1 << 7));
  
  uint8_t * memory = new uint8_t[size];
  BTree tree(10, memory);
  
  TreeTester tester(tree, "BTree");
  tester.TestAllocAll();
  tester.TestSetGet();
  tester.TestFindFree();
  
  delete memory;
  return 0;
}
