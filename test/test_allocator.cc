#include "../src/allocator.h"
#include "../src/btree.h"
#include "../src/bbtree.h"
#include <iostream>
#include "allocator_test.h"

template <class T>
void TestWithClass(std::string name);

int main() {
  TestWithClass<ANAlloc::BTree>("BTree");
  TestWithClass<ANAlloc::BBTree>("BBTree");
  return 0;
}

template <class T>
void TestWithClass(std::string name) {
  size_t size = T::MemorySize(10);
  uint8_t * memory = new uint8_t[size];
  T tree(10, memory);
  ANAlloc::Allocator<T> alloc(&tree);
  AllocTest<T> test(alloc, tree, name);
  test.TestBaseAlloc();
  test.TestFragAlloc();
  test.TestSplit();
  test.TestFind();
  test.TestReserve();
  delete memory;
}
