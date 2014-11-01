/**
 * Throughout these tests, you might see some notation indicating the state of
 * an allocator. General things to know:
 *
 * - R = active region structure in memory
 * - S = inactive region structure in the stack
 * - space = a free piece of memory
 *
 * I denote disjoint regions using brackets ([ and ]). I separate cells of
 * memory using vertical bars (|).
 *
 * Most of the time, I used this notation to figure out what a test should do.
 * A lot of the time, I assumed a large stack capacity so that I could see the
 * inductive nature of the test.
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
  ScopedPass pass("PlacedFreeList<", Capacity, ">::Alloc() [single overflow]");
  
  typedef PlacedFreeList<Capacity> Pfl;
  
  uintptr_t addr;
  
  size_t preambleSize = ComputePreambleSize<Capacity>();
  size_t regionSize = ComputeRegionSize<Capacity>();
  
  // Let R=regionSize, N=Capacity
  // We will have:
  // - one region of size R*N
  // - a region of size 5*R
  // - N-2 regions of size 2*R
  // - a region of size R
  // - a region of size 2*R
  // - Every region will be separated by R bytes
  
  size_t headingSize = Capacity * regionSize;
  size_t firstSize = 5 * regionSize;
  size_t blockSize = 2 * regionSize;
  size_t totalSize = preambleSize + headingSize + firstSize +
      blockSize * (Capacity - 1) + (Capacity + 2) * regionSize;
  
  void * buffer;
  assert(!posix_memalign(&buffer, regionSize, totalSize));
  
  Pfl * pfl = Pfl::Place((uintptr_t)buffer, preambleSize + headingSize);
  assert(pfl != nullptr);
  assert(pfl->GetStackCount() == 1);
  
  uintptr_t start = (uintptr_t)buffer + preambleSize;
  
  // Currently, the allocator looks like this: [S|R| |...]
  
  pfl->Dealloc(start + headingSize + regionSize * 3, regionSize * 3);
  
  // Now, the allocator looks like this:
  // [R|R|S| |...]    [ |...]
  
  // Now, deallocate enough more regions to saturate the heading
  for (size_t i = 0; i < Capacity - 3; ++i) {
    assert(pfl->GetStackCount() == 1);
    size_t offset = headingSize + regionSize * 6 + blockSize * i +
        regionSize * (i + 1);
    pfl->Dealloc(start + offset, blockSize);
  }
  // Ensure that the heading was saturated
  assert(pfl->GetStackCount() == 2);
  
  // The allocator now looks like this:
  // [R|S|R|...|S]    [ | | ] [ | ] ...
  
  // Now, we unsaturate the heading by deallocating another block
  size_t unsaturateOffset = headingSize + regionSize * 6 +
      blockSize * (Capacity - 3) + regionSize * (Capacity - 2);
  pfl->Dealloc(start + unsaturateOffset, blockSize);
  
  // The allocator now looks like this:
  // [R|R|R|...|S]    [ | | ] [ | ] ...
  
  // Add a small block to the allocator to fill up the heading altogether
  size_t smallBlockOffset = unsaturateOffset + regionSize * 3;
  pfl->Dealloc(start + smallBlockOffset, regionSize);
  
  // The allocator now looks like this:
  // [R|R|R|...|R]    [S| | ] [ | ] ... [ ]
  
  // Throw another block on the end.
  size_t lastBlockOffset = smallBlockOffset + regionSize * 2;
  assert(lastBlockOffset + regionSize * 2 == totalSize - preambleSize);
  pfl->Dealloc(start + lastBlockOffset, blockSize);
  
  // The allocator now looks like this:
  // [R|R|R|...|R]    [R|S| ] [ | ] ... [ ] [ | ]
  
  // Ninja the hell out of the allocator by adding a region below the first
  // region.
  pfl->Dealloc(start + headingSize + regionSize, 2 * regionSize);
  
  // The allocator now looks like this:
  // [R|R|R|...|R] [S| |R|R| ] [ | ] ... [ ] [ | ]
  
  // The next N-2 allocations will add to the stack
  for (size_t i = 0; i < Capacity - 2; ++i) {
    size_t offset = headingSize + regionSize * 6 + blockSize * i +
        regionSize * (i + 1);
    assert(pfl->Alloc(addr, blockSize));
    assert(addr == start + offset);
    assert(pfl->GetStackCount() == i + 2);
  }
  
  // The allocator now looks like this:
  // [R|S|...|S|R] [S| |R|R| ] [ ] [ | ]
  
  // Now, we'll deallocate the last block and go over the maximum stack size.
  assert(pfl->Alloc(addr, blockSize));
  assert(addr == start + lastBlockOffset);
  assert(pfl->GetStackCount() == Capacity - 1);
  
  // The allocator now looks like this:
  // [R|S|...|S|R] [S| | |R| ] [ ] [ | ]
  
  // We should now be able to allocate 2*R and find it in the first block
  assert(pfl->Alloc(addr, regionSize * 2));
  assert(addr == start + headingSize + regionSize * 2);
}

template <size_t Capacity>
void TestDoubleAllocOverflow() {
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
