#include "../src/malloc.hpp"
#include "../src/tree/bbtree.hpp"
#include "../src/tree/btree.hpp"
#include "scoped-pass.hpp"
#include <cstdlib>

using namespace ANAlloc;

template <class T>
void TestWrapRegion(const char * name);

template <class T>
void TestAlign(const char * name);

template <class T>
void TestFree(const char * name);

int main() {
  TestWrapRegion<BBTree>("BBTree");
  TestWrapRegion<BTree>("BTree");
  TestAlign<BBTree>("BBTree");
  TestAlign<BTree>("BTree");
  TestFree<BBTree>("BBTree");
  TestFree<BTree>("BTree");
  // TODO: test with initUsed != 0
  return 0;
}

template <class T>
void TestWrapRegion(const char * name) {
  ScopedPass pass("Malloc::WrapRegion<", name, ">()");
  
  uint8_t * buf = new uint8_t[0x200000];
  // create a new Malloc with page size 64
  
  Malloc * m = Malloc::WrapRegion<T>(buf, 0x200000, 6);
  
  assert(m->GetTree().GetDepth() == 16);
  assert(m->GetPageSizeLog() == 6);
  
  uint64_t useSize = sizeof(Malloc) + sizeof(T) + T::MemorySize(16);
  uint64_t baseCount = useSize >> 6;
  if (baseCount << 6 < useSize) ++baseCount;
  
  // make sure it carved up space correctly
  for (uint64_t i = 0; i < Path::DepthCount(15); i++) {
    Path p;
    bool res = m->GetTree().FindByShadow(i, p);
    assert(res == (i < baseCount));
  }
  
  assert(m->OwnsPointer(buf));
  assert(m->OwnsPointer(buf + 0x1fffff));
  assert(!m->OwnsPointer(buf - 1));
  assert(!m->OwnsPointer(buf + 0x200000));
  
  // test a simple allocation
  uint8_t * buffer = (uint8_t *)m->Alloc(0x100000);
  assert(buffer == buf + 0x100000);
  assert(!m->Alloc(0x100000));
  m->Free((void *)buffer);
  assert(buffer == (uint8_t *)m->Alloc(0x100000));
  m->Free((void *)buffer);
    
  assert(!m->Alloc(0x100001));
  
  delete buf;
}

template <class T>
void TestAlign(const char * name) {
  ScopedPass pass("Malloc::Align() [with ", name, "]");
  uint8_t * treeBuffer = new uint8_t[T::MemorySize(21)];
  
  T tree(21, treeBuffer);
  uint8_t * buffer = NULL;
  assert(!posix_memalign((void **)&buffer, 0x200000, 0x200000));
  Malloc theMalloc(buffer, tree, 1);
  
  uint8_t * bigBuf = (uint8_t *)theMalloc.Align(0x100000, 0x200000);
  assert(bigBuf != NULL);
  assert(bigBuf == buffer);
  assert(!theMalloc.Align(0x100000, 0x200000));
  
  // use a size smaller than 0x80000 just to shake things up
  uint8_t * smallerBuf = (uint8_t *)theMalloc.Align(0x1, 0x100000);
  assert(smallerBuf == buffer + 0x100000);
  assert(!theMalloc.Align(1, 0x100000));
  
  delete treeBuffer;
  free(buffer);
}

template <class T>
void TestFree(const char * name) {
  ScopedPass pass("Malloc::Free() [with ", name, "]");
  
  uint8_t * treeBuffer = new uint8_t[T::MemorySize(21)];
  
  T tree(21, treeBuffer);
  uint8_t * buffer = NULL;
  assert(!posix_memalign((void **)&buffer, 0x200000, 0x200000));
  Malloc theMalloc(buffer, tree, 1);
  
  uint8_t * buffer1 = (uint8_t *)theMalloc.Alloc(0x100000);
  uint8_t * buffer2 = (uint8_t *)theMalloc.Alloc(0x80000);
  uint8_t * buffer3 = (uint8_t *)theMalloc.Alloc(0x80000);
  assert(buffer1 != NULL);
  assert(buffer2 != NULL);
  assert(buffer3 != NULL);
  
  assert(!theMalloc.Alloc(1));
  theMalloc.Free(buffer3);
  assert(buffer3 == theMalloc.Alloc(0x80000));
  assert(!theMalloc.Alloc(1));
  
  delete treeBuffer;
  free(buffer);
}
