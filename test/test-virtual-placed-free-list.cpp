#include "scoped-pass.hpp"
#include "scoped-buffer.hpp"
#include "../src/free-list/virtual-placed-free-list.hpp"
//#include <analloc2/free-list>
#include <ansa/cstring>

using namespace analloc;

typedef VirtualPlacedFreeList<0x10> Vpfl;

void TestAlloc();
void TestRealloc();
void TestOffsetAlign();

int main() {
  TestAlloc();
  TestRealloc();
  TestOffsetAlign();
  return 0;
}

void TestAlloc() {
  ScopedPass pass("VirtualPlacedFreeList::[Alloc/Dealloc]()");
  ScopedBuffer buffer(0x1000, 0x1000);
  Vpfl * allocator = Vpfl::Place(buffer, 0x1000);
  uintptr_t addr;
  for (size_t i = 0; i < 0x1000; ++i) {
    assert(allocator->Alloc(addr, 1));
    assert(addr >= (uintptr_t)buffer && addr <= (uintptr_t)buffer + 0xfff);
    allocator->Free(addr);
  }
}

void TestRealloc() {
  ScopedPass pass("VirtualPlacedFreeList::Realloc()");
  ScopedBuffer buffer(0x1000, 0x1000);
  Vpfl * allocator = Vpfl::Place(buffer, 0x1000);
  uintptr_t addr;
  for (size_t i = 0; i < 0x1000; ++i) {
    assert(allocator->Alloc(addr, 3));
    ansa::Memcpy((void *)addr, "hey", 3);
    assert(allocator->Realloc(addr, 0x100));
    assert(ansa::Memcmp((void *)addr, "hey", 3) == 0);
    ansa::Memcpy((void *)addr, "sup??", 5);
    assert(allocator->Realloc(addr, 3));
    assert(ansa::Memcmp((void *)addr, "sup", 3) == 0);
    allocator->Free(addr);
  }
}

void TestOffsetAlign() {
  assert(false);
}
