#include "../src/allocator.h"
#include "../src/btree.h"
#include "../src/bbtree.h"
#include <iostream>
#include "allocator_test.h"

void TestWithBTree();
void TestWithBBTree();

int main() {
  TestWithBTree();
  TestWithBBTree();
  return 0;
}

void TestWithBTree() {
  size_t size = ANAlloc::BTree::MemorySize(10);
  uint8_t * memory = new uint8_t[size];
  ANAlloc::BTree tree(10, memory);
  ANAlloc::Allocator<ANAlloc::BTree> alloc(&tree);
  TestBaseAlloc(alloc, tree, "BTree");
  TestFragAlloc(alloc, tree, "BTree");
  delete memory;
}

void TestWithBBTree() {
  size_t size = ANAlloc::BBTree::MemorySize(10);
  uint8_t * memory = new uint8_t[size];
  ANAlloc::BBTree tree(10, memory);
  ANAlloc::Allocator<ANAlloc::BBTree> alloc(&tree);
  TestBaseAlloc(alloc, tree, "BBTree");
  TestFragAlloc(alloc, tree, "BBTree");
  delete memory;
}
