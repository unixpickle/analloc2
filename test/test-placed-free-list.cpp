#include "scoped-pass.hpp"
#include <analloc2/free-list>

using namespace analloc;
using namespace ansa;

template <size_t Capacity>
void TestAll();

template <size_t Capacity>
void TestSimplePlace();

template <size_t Capacity>
void TestPaddedPlace();

template <size_t Capacity>
void TestStackOverflows();

template <size_t Capacity>
void TestStackUnderflows();

int main() {
  TestAll<4>();
  TestAll<5>();
  TestAll<6>();
  TestAll<0x10>();
  return 0;
}

template <size_t Capacity>
void TestAll() {
  TestSimplePlace<Capacity>();
  TestPaddedPlace<Capacity>();
  TestStackOverflows<Capacity>();
  TestStackUnderflows<Capacity>();
}

template <size_t Capacity>
void TestSimplePlace() {
  ScopedPass pass("PlacedFreeList<", Capacity, ">::Place() [simple]");
  
  typedef PlacedFreeList<Capacity> Pfl;
  
  size_t objectAlign = sizeof(void *);
  size_t regionSize = (size_t)1 << Log2Ceil(sizeof(typename Pfl::FreeRegion));
  regionSize = Align2(regionSize, objectAlign);
  size_t instanceSize = Align2(sizeof(Pfl), objectAlign);
  size_t stackSize = Align2(sizeof(typename Pfl::StackType), objectAlign);
  size_t preamble = Align2(instanceSize + stackSize, regionSize);
  
  // Create a buffer that gives everything the proper alignment.
  void * buffer;
  if (posix_memalign(&buffer, regionSize, preamble + regionSize * 3)) {
    std::cerr << "failed to align memory" << std::endl;
    abort();
  }
  uintptr_t start = (uintptr_t)buffer;
  uintptr_t addr;
  assert(!Pfl::Place(start, preamble));
  assert(!Pfl::Place(start, preamble + regionSize - 1));
  Pfl * pfl = Pfl::Place(start, preamble + regionSize);
  assert(!pfl->Alloc(addr, 1));
  assert(!pfl->Alloc(addr, 0));
  pfl = Pfl::Place(start, preamble + regionSize * 2);
  assert(!pfl->Alloc(addr, 1));
  assert(!pfl->Alloc(addr, 0));
  pfl = Pfl::Place(start, preamble + regionSize * 3);
  assert(!pfl->Alloc(addr, regionSize + 1));
  assert(pfl->Alloc(addr, regionSize));
  assert(addr == start + preamble + regionSize * 2);
  assert(!pfl->Alloc(addr, 1));
  
  free(buffer);
}

template <size_t Capacity>
void TestPaddedPlace() {
  ScopedPass pass("PlacedFreeList<", Capacity, ">::Place() [padded]");
  assert(false);
}

template <size_t Capacity>
void TestStackOverflows() {
  ScopedPass pass("PlacedFreeList<", Capacity, "> [overflows]");
  assert(false);
}

template <size_t Capacity>
void TestStackUnderflows() {
  ScopedPass pass("PlacedFreeList<", Capacity, "> [underflows]");
  assert(false);
}
