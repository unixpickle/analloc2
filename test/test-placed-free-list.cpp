/**
 * Throughout these tests, you might see some notation indicating the state of
 * a placement allocator. General things to know:
 *
 * - R = active region structure in memory
 * - S = inactive region structure in the stack
 * - space = a free piece of memory
 *
 * I denote disjoint regions using brackets ([ and ]). I separate cells of
 * memory using vertical bars (|).
 */

#include "scoped-pass.hpp"
#include <analloc2/free-list>

using namespace analloc;
using namespace ansa;

template <size_t Capacity>
void TestAll();

template <size_t Capacity>
void TestSimplePlace();

template <size_t Capacity>
void TestOffsetPlace();

template <size_t Capacity>
void TestNormalAllocOverflow();

template <size_t Capacity>
void TestDoubleAllocOverflow();

template <size_t Capacity>
size_t ComputePreambleSize();

template <size_t Capacity>
size_t ComputeRegionSize();

int main() {
  TestAll<3>();
  TestAll<4>();
  TestAll<5>();
  TestAll<6>();
  TestAll<0x10>();
  return 0;
}

template <size_t Capacity>
void TestAll() {
  TestSimplePlace<Capacity>();
  TestOffsetPlace<Capacity>();
  TestNormalAllocOverflow<Capacity>();
  TestDoubleAllocOverflow<Capacity>();
}

template <size_t Capacity>
void TestSimplePlace() {
  ScopedPass pass("PlacedFreeList<", Capacity, ">::Place() [simple]");
  
  typedef PlacedFreeList<Capacity> Pfl;
  
  size_t regionSize = ComputeRegionSize<Capacity>();
  size_t preamble = ComputePreambleSize<Capacity>();
  
  // Create a buffer that gives everything the proper alignment.
  void * buffer;
  assert(!posix_memalign(&buffer, regionSize, preamble + regionSize * 3));
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
void TestOffsetPlace() {
  ScopedPass pass("PlacedFreeList<", Capacity, ">::Place() [offset]");
    
  typedef PlacedFreeList<Capacity> Pfl;
  
  size_t objectAlign = sizeof(void *);
  size_t regionSize = (size_t)1 << Log2Ceil(sizeof(typename Pfl::FreeRegion));
  regionSize = Align2(regionSize, objectAlign);
  size_t instanceSize = Align2(sizeof(Pfl), objectAlign);
  size_t stackSize = Align2(sizeof(typename Pfl::StackType), objectAlign);
  size_t preamble = instanceSize + stackSize;
  size_t misalignment = (preamble + objectAlign) % regionSize;
  if (misalignment) {
    preamble += regionSize - misalignment;
  }
  
  // Create a buffer that gives everything the proper alignment.
  void * buffer;
  assert(!posix_memalign(&buffer, regionSize, objectAlign + preamble +
                         regionSize * 3));
  uintptr_t start = (uintptr_t)buffer + objectAlign;
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
void TestNormalAllocOverflow() {
  
}

template <size_t Capacity>
void TestDoubleAllocOverflow() {
  // NOTE: most of the visual examples in this test make use of a large
  // Capacity to give a good depiction of what's going on. This test works for
  // Capacity >= 3, but the visual examples don't.
  
  ScopedPass pass("PlacedFreeList<", Capacity, ">::Alloc() [double overflow]");
  
  typedef PlacedFreeList<Capacity> Pfl;
  
  uintptr_t addr;
  
  size_t preambleSize = ComputePreambleSize<Capacity>();
  size_t regionSize = ComputeRegionSize<Capacity>();
  
  // Let R=regionSize, N=Capacity
  // We will have:
  // - one region of size R*(N+2)
  // - N-1 regions of size R*(N+3)
  // - Every region will be separated by R bytes
  // Total size needed = R*(N+2) + (N-1)*R*(N+3) + R*(N - 1)
  
  size_t headingSize = regionSize * (Capacity + 2);
  size_t blockSize = regionSize * (Capacity + 3);
  
  size_t totalSize = preambleSize + headingSize + blockSize * (Capacity - 1) + 
      regionSize * (Capacity - 1);
  
  void * buffer;
  assert(!posix_memalign(&buffer, regionSize, totalSize));
  
  Pfl * pfl = Pfl::Place((uintptr_t)buffer, preambleSize + headingSize);
  assert(pfl != nullptr);
  assert(pfl->GetStackCount() == 1);
  
  uintptr_t start = (uintptr_t)buffer + preambleSize;
  
  // Currently, the allocator looks like this: [S|R| |...]
  
  // Add a bunch of blocks
  for (size_t i = 1; i < Capacity; ++i) {
    size_t offset = headingSize + i * regionSize + (i - 1) * blockSize;
    pfl->Dealloc(start + offset, blockSize);
    assert(pfl->GetStackCount() == 1);
  }
  
  // Currently, the allocator looks like this:
  // [R|R|...|S| ] [ |...] ... [ |...]
  
  // Each time we Alloc() another block, it moves a region structure to the
  // stack as follows:
  // [S|R|...|S| ]
  // [S|R|S|R...|S| ] 
  // [S|R|S|S|R...|S| ]
  // Thus, after Capacity - 2 Alloc()s, the stack will contain Capacity - 1 
  // items:
  // [S|R|S|S|S|...|R|S| ]
  
  for (size_t i = 1; i < Capacity - 1; ++i) {
    assert(pfl->GetStackCount() == i);
    size_t offset = headingSize + i * regionSize + (i - 1) * blockSize;
    assert(pfl->Alloc(addr, blockSize));
    assert(addr == start + offset);
    assert(pfl->GetStackCount() == i + 1);
  }
  
  // When we allocate the next block, the heading will at first look like this:
  // [S|R|S|S|...|S|S|S| ]
  // To address this, the stack will remove the last element from the stack:
  // [S|R|S|S|...|S|X|S| ]
  // Then, it will need a new region, so it will remove the next element from
  // the stack as well:
  // [S|R|S|S|...|R| |S| ]
  
  assert(pfl->Alloc(addr, blockSize));
  assert(addr == start + headingSize + regionSize * (Capacity - 1) +
                 blockSize * (Capacity - 2));
  assert(pfl->GetStackCount() == Capacity - 2);
  
  // The first free region is now located near the end of the heading.
  assert(pfl->Alloc(addr, regionSize));
  assert(addr == start + headingSize - (regionSize * 3));
}

template <size_t Capacity>
size_t ComputePreambleSize() {
  typedef PlacedFreeList<Capacity> Pfl;
  
  size_t objectAlign = sizeof(void *);
  size_t regionSize = ComputeRegionSize<Capacity>();
  size_t instanceSize = Align2(sizeof(Pfl), objectAlign);
  size_t stackSize = Align2(sizeof(typename Pfl::StackType), objectAlign);
  return Align2(instanceSize + stackSize, regionSize);
}

template <size_t Capacity>
size_t ComputeRegionSize() {
  typedef PlacedFreeList<Capacity> Pfl;
  size_t objectAlign = sizeof(void *);
  size_t regionSize = (size_t)1 << Log2Ceil(sizeof(typename Pfl::FreeRegion));
  return Align2(regionSize, objectAlign);
}
