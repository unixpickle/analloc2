#include "../src/malloc.hpp"
#include "../src/tree/bbtree.hpp"
#include "../src/tree/btree.hpp"
#include "scoped-pass.hpp"

using namespace ANAlloc;

template <class T>
void TestWrapRegion(const char * name);

int main() {
  TestWrapRegion<BBTree>("BBTree");
  TestWrapRegion<BTree>("BTree");
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
