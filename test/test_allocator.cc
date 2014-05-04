#include "../src/allocator.h"
#include "../src/btree.h"
#include <iostream>

using namespace ANAlloc;

int main() {
  std::cout << "testing Allocator<BTree> ... ";
  size_t size = ANAlloc::BTree::MemorySize(10);
  uint8_t * memory = new uint8_t[size];
  BTree tree(10, memory);
  Allocator<BTree> alloc(&tree);
  
  Path p;
  alloc.Alloc(9, p);
  assert((1 << 9) - 1 == p);
  
  assert(tree.GetType(1) == BTree::NodeTypeContainer); // depth 1
  assert(tree.GetType(2) == BTree::NodeTypeFree);
  assert(tree.GetType(3) == BTree::NodeTypeContainer); // depth 2
  assert(tree.GetType(4) == BTree::NodeTypeFree);
  assert(tree.GetType(7) == BTree::NodeTypeContainer); // depth 3
  assert(tree.GetType(8) == BTree::NodeTypeFree);
  assert(tree.GetType(0xf) == BTree::NodeTypeContainer); // depth 4
  assert(tree.GetType(0x10) == BTree::NodeTypeFree);
  assert(tree.GetType(0x1f) == BTree::NodeTypeContainer); // depth 5
  assert(tree.GetType(0x20) == BTree::NodeTypeFree);
  assert(tree.GetType(0x3f) == BTree::NodeTypeContainer); // depth 6
  assert(tree.GetType(0x40) == BTree::NodeTypeFree);
  assert(tree.GetType(0x7f) == BTree::NodeTypeContainer); // depth 7
  assert(tree.GetType(0x80) == BTree::NodeTypeFree);
  assert(tree.GetType(0xff) == BTree::NodeTypeContainer); // depth 8
  assert(tree.GetType(0x100) == BTree::NodeTypeFree);
  assert(tree.GetType(0x1ff) == BTree::NodeTypeData); // depth 8
  assert(tree.GetType(0x200) == BTree::NodeTypeFree);
  
  std::cout << "passed!" << std::endl;
  
  delete memory;
  return 0;
}
