#include "../src/bbtree.hpp"
#include "tree-tester.hpp"

using namespace ANAlloc;

int main() {
  assert(BBTree::MemorySize(1) == 1);
  assert(BBTree::MemorySize(2) == 1);
  assert(BBTree::MemorySize(3) == 2);
  assert(BBTree::MemorySize(4) == 3);
  assert(BBTree::MemorySize(5) == 7);
  
  size_t size = BBTree::MemorySize(10);
  assert(size == 209);
  
  uint8_t * memory = new uint8_t[size];
  BBTree tree(10, memory);
  
  TreeTester tester(tree, "BBTree");
  tester.TestAll();
  
  delete memory;
  return 0;
}
