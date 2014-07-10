#include "../src/allocator.hpp"
#include "../src/tree/bbtree.hpp"
#include "../src/tree/btree.hpp"
#include "scoped-pass.hpp"

using namespace ANAlloc;

template <class T>
void TestReserve(const char * treeName);

template <class T>
void TestFreeSize(const char * treeName);

int main() {
  TestReserve<BTree>("BTree");
  TestReserve<BBTree>("BBTree");
  TestFreeSize<BTree>("BTree");
  TestFreeSize<BBTree>("BBTree");
  return 0;
}

template <class T>
void TestReserve(const char * treeName) {
  ScopedPass pass("Allocator::Reserve() [", treeName, "]");
  
  uint8_t * buffer = new uint8_t[T::MemorySize(10)];
  T tree(10, buffer);
  
  // start = 0x100, tree = tree, page size = 2
  Allocator alloc(0x100, tree, 1);
  alloc.Reserve(1, 10);
  
  // should have ended up reserving 12 bytes
  assert(alloc.GetFreeSize() == alloc.GetTotalSize() - 0xc);
  assert(alloc.GetTotalSize() == 0x400);
  assert(tree.GetType(Path::Root()) == NodeTypeContainer);
  assert(tree.GetType(Path(1, 1)) == NodeTypeFree);
  
  // you may want to test the rest of the tree, but I trust it...
  
  delete[] buffer;
}

template <class T>
void TestFreeSize(const char * treeName) {
  ScopedPass pass("Allocator::GetFreeSize() [", treeName, "]");
  
  uint8_t * buffer = new uint8_t[T::MemorySize(10)];
  T tree(10, buffer);
  
  // start = 0x100, tree = tree, page size = 2
  Allocator alloc(0x100, tree, 1);
  alloc.Reserve(1, 10);
  
  assert(alloc.GetFreeSize() == 0x400 - 0xc);
  
  UInt addr;
  bool res = alloc.Align(0x10, 0x100, addr);
  assert(res);
  assert(addr == 0x100 || addr == 0x200 || addr == 0x300);
  
  assert(alloc.GetFreeSize() == 0x400 - 0x1c);
  
  UInt addr2;
  res = alloc.Alloc(0x40, addr2);
  assert(res);
  assert(alloc.GetFreeSize() == 0x400 - 0x5c);
  
  alloc.Free(addr);
  assert(alloc.GetFreeSize() == 0x400 - 0x4c);
  alloc.Free(addr2);
  assert(alloc.GetFreeSize() == 0x400 - 0xc);
  
  delete[] buffer;
}
