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
void TestDoubleDeallocOverflow();

void TestRandomOperations();

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
  TestRandomOperations();
  return 0;
}

template <size_t Capacity>
void TestAll() {
  TestSimplePlace<Capacity>();
  TestOffsetPlace<Capacity>();
  TestNormalAllocOverflow<Capacity>();
  TestDoubleAllocOverflow<Capacity>();
  TestDoubleDeallocOverflow<Capacity>();
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
  
  free(buffer);
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
  
  free(buffer);
}

template <size_t Capacity>
void TestDoubleDeallocOverflow() {
  ScopedPass pass("PlacedFreeList<", Capacity,
                  ">::Dealloc() [double overflow]");

  typedef PlacedFreeList<Capacity> Pfl;

  uintptr_t addr;

  size_t preambleSize = ComputePreambleSize<Capacity>();
  size_t regionSize = ComputeRegionSize<Capacity>();

  // Let R=regionSize, N=Capacity
  // We will have:
  // - one region of size R*(N+3)
  // - N regions of size R
  // - Every region will be separated by R bytes
  // Total size needed = R*(N+3) + 2*R*N
  
  size_t headingSize = regionSize * (Capacity + 3);
  size_t totalSize = preambleSize + headingSize + 2 * Capacity * regionSize;
  
  void * buffer;
  assert(!posix_memalign(&buffer, regionSize, totalSize));
  
  Pfl * pfl = Pfl::Place((uintptr_t)buffer, preambleSize + headingSize);
  assert(pfl != nullptr);
  assert(pfl->GetStackCount() == 1);
  
  uintptr_t start = (uintptr_t)buffer + preambleSize;
  
  // Currently, the allocator looks like this: [S|R| |...]
  
  // Perform N deallocations
  for (size_t i = 0; i < Capacity; ++i) {
    size_t offset = headingSize + (2 * i + 1) * regionSize;
    pfl->Dealloc(start + offset, regionSize);
    assert(pfl->GetStackCount() == 1);
  }
  
  // Now the allocator looks like this:
  // [R|R|...|S| ] [ ] ... [ ]
  
  // Join the N regions into a big region and one tiny one
  for (size_t i = 1; i < Capacity - 1; ++i) {
    size_t offset = headingSize + 2 * i * regionSize;
    pfl->Dealloc(start + offset, regionSize);
    assert(pfl->GetStackCount() == i + 1);
  }
  
  // Now the allocator looks like this:
  // [R|R|S|...|R|S| ] [ |...] [ ]
  
  // Perform the final join, causing the stack to exceed its maximum.
  pfl->Dealloc(start + headingSize + 2 * (Capacity - 1) * regionSize,
      regionSize);
  
  // Now the allocator looks like this:
  // [R|R|S|...|R| |S| ] [ |...]
  assert(pfl->GetStackCount() == Capacity - 2);
  
  // Ensure that the single space is available for allocation
  assert(pfl->Alloc(addr, regionSize));
  assert(addr == start + Capacity * regionSize);
  
  free(buffer);
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

void TestRandomOperations() {
  ScopedPass pass("PlacedFreeList<4>::[Alloc/Dealloc]() [random]");

  typedef PlacedFreeList<4> Pfl;

  uintptr_t addr;

  size_t preambleSize = ComputePreambleSize<4>();
  size_t regionSize = ComputeRegionSize<4>();

  // NOTE: the sequence of allocations below is a very special one. It will
  // trigger a Dealloc() cascade of depth 3. These are very rare, and I was far
  // to lazy to figure out how to generate one myself.
  
  size_t totalSize = preambleSize + regionSize * 0x100;
  
  void * buffer;
  assert(!posix_memalign(&buffer, regionSize, totalSize));
  
  Pfl * pfl = Pfl::Place((uintptr_t)buffer, preambleSize + regionSize * 0x100);
  assert(pfl != nullptr);
  assert(pfl->GetStackCount() == 1);
  
  uintptr_t start = (uintptr_t)buffer + preambleSize;
  
  /**
   * This list was generated by "resources/placed-free-list-gen.dart"
   */
  struct {
    int type;
    size_t address;
    size_t size;
    size_t stackCount;
  } operations[] = {
    {1, 2, 40, 1}, {-1, 7, 1, 2}, {1, 42, 60, 2}, {1, 102, 130, 2},
    {-1, 138, 85, 1}, {-1, 228, 2, 1}, {-1, 3, 2, 1}, {1, 139, 4, 1},
    {1, 143, 79, 1}, {1, 232, 6, 1}, {1, 238, 14, 1}, {1, 252, 4, 2},
    {1, 228, 2, 3}, {1, 4, 1, 2}, {1, 138, 1, 3}, {-1, 43, 169, 2},
    {-1, 221, 1, 2}, {-1, 216, 4, 1}, {-1, 32, 3, 1}, {-1, 214, 2, 1},
    {-1, 5, 1, 2}, {-1, 220, 1, 3}, {-1, 25, 4, 2}, {-1, 213, 1, 2},
    {1, 43, 142, 2}, {-1, 18, 2, 1}, {1, 185, 21, 1}, {1, 25, 4, 2},
    {1, 213, 8, 2}, {1, 206, 5, 2}, {-1, 6, 1, 1}, {-1, 4, 1, 2},
    {-1, 250, 2, 1}, {-1, 8, 9, 2}, {-1, 172, 19, 1}, {-1, 2, 1, 2},
    {1, 172, 14, 2}, {-1, 225, 1, 1}, {-1, 234, 4, 1}, {-1, 27, 2, 1},
    {1, 10, 2, 1}, {-1, 213, 2, 1}, {-1, 219, 1, 1}, {1, 186, 5, 2},
    {-1, 241, 4, 1}, {-1, 239, 1, 1}, {1, 15, 1, 1}, {-1, 22, 3, 2},
    {-1, 208, 2, 1}, {-1, 21, 1, 1}, {-1, 238, 1, 2}, {-1, 17, 1, 2},
    {1, 17, 2, 2}, {-1, 15, 1, 1}, {-1, 254, 2, 2}, {1, 234, 5, 2},
    {-1, 114, 52, 1}, {-1, 215, 4, 2}, {-1, 220, 1, 3}, {1, 114, 40, 3},
    {1, 19, 1, 2}, {-1, 240, 1, 3}, {1, 154, 7, 3}, {-1, 233, 6, 3},
    {1, 21, 3, 3}, {1, 161, 5, 2}, {-1, 25, 1, 2}, {-1, 10, 1, 1},
    {1, 213, 3, 1}, {1, 24, 2, 2}, {-1, 25, 1, 1}, {-1, 247, 1, 2},
    {-1, 19, 4, 1}, {1, 233, 11, 1}, {1, 216, 7, 2}, {1, 19, 4, 3},
    {1, 27, 2, 2}, {1, 8, 1, 3}, {1, 33, 2, 2}, {1, 208, 2, 3}, {1, 3, 1, 2},
    {1, 250, 2, 3}, {-1, 18, 6, 2}, {1, 18, 6, 3}, {1, 254, 2, 2},
    {-1, 245, 1, 2}, {1, 5, 1, 3}, {-1, 205, 5, 2}, {1, 10, 1, 3},
    {1, 205, 5, 3}, {1, 15, 1, 3}, {-1, 17, 4, 3}, {-1, 246, 1, 2},
    {1, 16, 3, 2}, {1, 13, 1, 3}, {-1, 27, 2, 2}, {-1, 8, 1, 1},
    {1, 244, 4, 2}, {-1, 10, 2, 1}, {1, 10, 2, 2}, {-1, 5, 1, 1},
    {-1, 215, 4, 2}, {-1, 31, 1, 2}, {1, 8, 1, 3}, {1, 215, 3, 3},
    {-1, 15, 1, 2}, {1, 19, 2, 3}, {1, 27, 2, 2}, {1, 12, 1, 3}, {1, 31, 2, 2},
    {-1, 165, 14, 1}, {-1, 181, 9, 2}, {-1, 133, 28, 1}, {1, 133, 5, 1},
    {-1, 3, 1, 2}, {1, 138, 3, 2}, {1, 141, 4, 2}, {1, 145, 16, 3},
    {1, 165, 12, 3}, {-1, 207, 1, 2}, {1, 181, 8, 2}, {-1, 228, 24, 1},
    {1, 228, 4, 1}, {-1, 213, 2, 2}, {1, 232, 20, 3}, {1, 25, 1, 2},
    {-1, 193, 1, 1}, {-1, 8, 1, 2}, {1, 14, 1, 3}, {1, 177, 1, 3},
    {-1, 245, 8, 2}, {-1, 216, 2, 2}, {1, 245, 5, 2}, {1, 178, 1, 3},
    {-1, 227, 21, 2}, {1, 227, 14, 2}, {-1, 191, 1, 1}, {1, 189, 1, 2},
    {-1, 215, 1, 3}, {1, 213, 5, 3}, {1, 241, 3, 3}, {-1, 206, 1, 3},
    {1, 244, 4, 2}, {-1, 255, 1, 1}, {1, 250, 3, 2}, {1, 206, 2, 3},
    {1, 5, 1, 2}, {-1, 5, 1, 1}, {1, 1, 1, 2}, {1, 5, 1, 3}, {-1, 5, 1, 2},
    {1, 5, 1, 3}, {1, 191, 1, 2}, {-1, 5, 1, 1}, {1, 5, 1, 2}, {1, 8, 1, 3},
    {-1, 8, 1, 2}, {1, 8, 1, 3}, {1, 193, 1, 2}, {-1, 8, 1, 1}, {1, 8, 1, 2},
    {1, 15, 1, 3}, {-1, 220, 4, 2}, {1, 220, 4, 3}, {-1, 219, 1, 3},
    {-1, 215, 2, 2}, {1, 215, 2, 3}, {-1, 213, 5, 3}, {1, 213, 2, 3},
    {-1, 238, 14, 2}, {1, 215, 2, 2}, {1, 238, 9, 2}, {-1, 212, 5, 3},
    {-1, 254, 1, 3}, {1, 211, 2, 3}, {-1, 252, 1, 3}, {-1, 1, 1, 2},
    {-1, 233, 2, 1}, {1, 213, 2, 1}, {1, 1, 1, 2}, {1, 215, 1, 2},
    {1, 247, 5, 2}, {-1, 230, 3, 2}, {-1, 221, 4, 2}, {-1, 220, 1, 3},
    {1, 216, 2, 3}, {1, 218, 1, 3}, {-1, 5, 1, 2}, {1, 219, 6, 2},
    {1, 230, 4, 2}, {-1, 228, 4, 1}, {-1, 232, 1, 1}, {1, 228, 5, 2},
    {1, 254, 2, 3}, {-1, 8, 1, 2}, {1, 5, 1, 3}, {1, 8, 1, 2}, {1, 7, 1, 3},
    {1, 225, 1, 2}, {1, 0, 1, 3}, {1, 234, 1, 2}, {-1, 163, 89, 2},
    {1, 163, 16, 2}, {-1, 177, 1, 1}, {-1, 8, 1, 2}, {-1, 165, 2, 1},
    {1, 179, 53, 1}, {1, 232, 8, 1}, {1, 240, 5, 1}, {-1, 0, 1, 2},
    {-1, 253, 1, 2}, {1, 245, 6, 2}, {1, 8, 1, 3}, {1, 165, 1, 3},
    {1, 166, 1, 2}, {1, 251, 3, 3}, {1, 4, 1, 2}, {1, 9, 1, 3}, {-1, 1, 1, 2},
    {1, 1, 1, 3}, {-1, 1, 1, 2}, {-1, 200, 15, 1}, {1, 200, 6, 1},
    {-1, 5, 1, 2}, {1, 206, 2, 2}, {1, 208, 5, 2}, {1, 213, 2, 3},
    {-1, 201, 15, 2}, {-1, 237, 12, 1}, {-1, 251, 5, 2}, {1, 201, 13, 2},
    {1, 237, 4, 2}, {1, 241, 4, 2}, {-1, 199, 15, 2}, {1, 199, 7, 2},
    {-1, 4, 1, 1}, {-1, 236, 8, 2}, {-1, 54, 88, 1}, {1, 54, 74, 1},
    {1, 128, 6, 1}, {1, 206, 9, 1}, {1, 134, 8, 2}, {-1, 244, 1, 3},
    {-1, 249, 1, 3}, {1, 236, 4, 3}, {1, 240, 9, 3}, {-1, 242, 1, 2},
    {-1, 223, 11, 1}, {1, 177, 1, 2}, {1, 223, 8, 2}, {1, 215, 1, 3},
    {1, 231, 2, 3}, {1, 251, 5, 2}, {1, 3, 1, 3}, {1, 233, 1, 2},
    {-1, 248, 1, 2}, {-1, 246, 2, 2}, {1, 246, 2, 2}, {-1, 251, 2, 1},
    {-1, 245, 2, 2}, {1, 242, 1, 3}, {1, 245, 1, 3}, {-1, 3, 1, 2},
    {-1, 253, 2, 2}, {-1, 34, 212, 2}, {-1, 247, 1, 3}, {-1, 250, 1, 2},
    {-1, 255, 1, 2}, {-1, 17, 4, 1}, {1, 34, 216, 1}, {-1, 11, 5, 2},
    {-1, 21, 144, 2}, {-1, 213, 6, 1}, {-1, 9, 1, 2}, {-1, 187, 19, 1},
    {-1, 10, 1, 2}, {-1, 16, 1, 3}, {-1, 178, 6, 2}, {1, 9, 89, 2},
    {1, 98, 13, 2}, {1, 111, 10, 2}, {1, 121, 10, 2}, {1, 131, 27, 2},
    {1, 158, 1, 2}, {1, 159, 6, 3}, {-1, 221, 11, 2}, {-1, 186, 1, 2},
    {-1, 220, 1, 2}, {1, 186, 13, 2}, {1, 178, 4, 2}, {-1, 169, 6, 1},
    {1, 220, 11, 1}, {-1, 211, 1, 1}, {-1, 192, 5, 1}, {1, 199, 7, 2},
    {-1, 181, 1, 2}, {-1, 197, 3, 2}, {1, 192, 5, 2}, {-1, 223, 1, 1},
    {1, 171, 3, 1}, {-1, 246, 4, 1}, {-1, 177, 1, 2}, {-1, 186, 2, 1},
    {-1, 212, 1, 2}, {-1, 191, 4, 1}, {1, 246, 10, 2}, {1, 177, 1, 3},
    {-1, 184, 1, 3}, {-1, 185, 1, 2}, {1, 181, 5, 2}, {1, 186, 2, 3},
    {-1, 171, 2, 2}, {-1, 228, 3, 2}, {-1, 254, 2, 1}, {1, 191, 3, 1},
    {1, 211, 8, 2}, {1, 228, 4, 3}, {-1, 179, 12, 2}, {-1, 227, 1, 1},
    {1, 179, 9, 1}, {-1, 126, 16, 1}, {-1, 144, 8, 1}, {1, 128, 8, 1},
    {1, 136, 5, 1}, {1, 144, 5, 1}, {-1, 195, 1, 1}, {1, 149, 3, 2},
    {-1, 191, 2, 2}, {1, 188, 5, 3}, {1, 197, 3, 2}, {-1, 151, 7, 1},
    {1, 151, 7, 2}, {-1, 40, 49, 1}, {-1, 118, 6, 1}, {-1, 134, 3, 1},
    {-1, 130, 2, 1}, {-1, 173, 1, 2}, {-1, 205, 9, 1}, {1, 43, 38, 1},
    {1, 81, 6, 1}, {1, 118, 3, 1}, {1, 87, 1, 1}, {1, 171, 4, 2},
    {-1, 225, 1, 1}, {-1, 138, 2, 2}, {-1, 132, 2, 3}, {1, 121, 2, 3},
    {1, 205, 9, 3}, {1, 130, 5, 3}, {1, 123, 1, 2}, {-1, 204, 10, 1},
    {1, 204, 6, 1}, {1, 210, 4, 2}, {-1, 231, 10, 1}, {-1, 140, 1, 2},
    {1, 231, 9, 2}, {-1, 56, 4, 1}, {1, 56, 1, 1}, {1, 57, 1, 1},
    {1, 58, 2, 2}, {-1, 137, 1, 3}, {-1, 46, 31, 2}, {-1, 205, 1, 1},
    {1, 46, 27, 1}, {1, 73, 3, 1}, {-1, 209, 12, 2}, {1, 209, 11, 2},
    {-1, 64, 4, 1}, {-1, 101, 10, 1}, {1, 101, 10, 2}, {-1, 245, 9, 2},
    {-1, 222, 1, 2}, {1, 245, 9, 2}, {1, 135, 7, 3}, {1, 65, 2, 3},
    {-1, 142, 14, 2}, {1, 142, 11, 2}, {1, 153, 3, 3}, {-1, 203, 1, 2},
    {-1, 232, 6, 1}, {1, 67, 1, 2}, {1, 169, 2, 3}, {-1, 83, 5, 2},
    {-1, 245, 4, 1}, {1, 83, 5, 2}, {1, 127, 1, 3}, {-1, 219, 1, 3},
    {-1, 36, 3, 2}, {1, 36, 1, 2}, {1, 37, 2, 3}, {-1, 73, 3, 2},
    {-1, 197, 2, 1}, {-1, 66, 4, 1}, {-1, 249, 3, 1}, {1, 245, 7, 2},
    {-1, 196, 1, 3}, {1, 232, 6, 2}, {-1, 204, 1, 3}, {-1, 217, 1, 2},
    {1, 194, 5, 3}, {1, 67, 3, 2}, {1, 73, 3, 3}, {-1, 91, 33, 2},
    {-1, 86, 1, 1}, {1, 91, 30, 1}, {-1, 98, 3, 2}, {-1, 75, 1, 1},
    {1, 98, 3, 2}, {-1, 169, 24, 1}, {1, 169, 18, 1}, {-1, 226, 1, 2},
    {1, 187, 5, 2}, {1, 121, 2, 2}, {-1, 74, 1, 2}, {-1, 248, 3, 1},
    {-1, 201, 1, 2}, {1, 74, 1, 2}, {1, 75, 1, 3}, {1, 203, 2, 3},
    {1, 219, 2, 2}, {1, 222, 2, 3}, {1, 86, 1, 2}, {-1, 235, 3, 1},
    {-1, 213, 1, 2}, {1, 123, 1, 3}, {1, 225, 3, 2}, {1, 4, 1, 3},
    {-1, 65, 1, 2}, {-1, 238, 1, 2}, {-1, 29, 5, 1}, {1, 29, 2, 1},
    {-1, 4, 1, 2}, {1, 31, 2, 2}, {-1, 211, 1, 1}, {1, 33, 1, 2},
    {-1, 194, 6, 1}, {1, 194, 5, 1}, {-1, 138, 31, 2}, {-1, 58, 6, 1},
    {1, 58, 5, 1}, {1, 138, 16, 1}, {-1, 193, 4, 1}, {1, 154, 8, 1},
    {-1, 208, 2, 2}, {-1, 142, 15, 1}, {-1, 224, 4, 2}, {1, 142, 2, 2},
    {-1, 140, 1, 1}, {-1, 212, 1, 2}, {1, 140, 1, 3}, {1, 144, 12, 3},
    {1, 156, 1, 2}, {1, 162, 3, 2}, {-1, 241, 5, 2}, {-1, 183, 9, 2},
    {1, 183, 13, 2}, {1, 165, 4, 3}, {1, 208, 2, 2}, {1, 240, 6, 3},
    {-1, 68, 6, 2}, {1, 68, 2, 2}, {-1, 202, 1, 2}, {1, 70, 4, 3},
    {-1, 72, 2, 2}, {-1, 85, 2, 1}, {-1, 209, 2, 1}, {-1, 77, 1, 2},
    {-1, 28, 4, 1}, {1, 28, 3, 1}, {-1, 87, 1, 1}, {1, 209, 4, 1},
    {-1, 74, 2, 1}, {1, 72, 2, 1}, {1, 31, 1, 2}, {1, 224, 4, 3},
    {-1, 246, 1, 2}, {1, 85, 3, 3}, {1, 235, 3, 3}, {-1, 162, 10, 2},
    {-1, 197, 2, 3}, {-1, 253, 1, 3}, {-1, 251, 1, 3}, {1, 162, 8, 3},
    {1, 63, 1, 2}, {-1, 68, 4, 1}, {-1, 67, 1, 1}, {1, 67, 4, 1},
    {1, 196, 3, 1}, {1, 74, 2, 2}, {1, 71, 1, 3}, {1, 248, 3, 3},
    {-1, 203, 2, 2}, {1, 201, 4, 2}, {1, 253, 3, 3}, {-1, 14, 23, 2},
    {1, 14, 17, 2}, {-1, 200, 1, 2}, {-1, 114, 5, 1}, {-1, 244, 2, 1},
    {1, 31, 2, 1}, {1, 33, 4, 2}, {-1, 204, 1, 2}, {1, 114, 4, 2},
    {-1, 25, 4, 1}, {1, 25, 1, 1}, {-1, 123, 2, 1}, {1, 27, 2, 2},
    {1, 41, 1, 3}, {-1, 249, 2, 3}, {-1, 219, 3, 2}, {-1, 212, 1, 2},
    {-1, 189, 3, 1}, {-1, 41, 1, 2}, {1, 123, 2, 3}, {-1, 173, 1, 2},
    {-1, 226, 5, 1}, {-1, 254, 1, 2}, {1, 170, 2, 3}, {-1, 16, 1, 2},
    {-1, 134, 8, 1}, {1, 134, 4, 1}, {-1, 115, 1, 2}, {1, 115, 1, 3},
    {-1, 241, 2, 2}, {1, 138, 4, 3}, {-1, 198, 1, 3}, {-1, 82, 1, 2},
    {1, 189, 3, 3}, {-1, 214, 1, 3}, {1, 82, 1, 2}, {-1, 215, 2, 3},
    {1, 212, 4, 3}, {1, 198, 2, 3}, {-1, 255, 1, 3}, {-1, 21, 5, 2},
    {-1, 243, 1, 3}, {1, 21, 2, 3}, {-1, 234, 1, 2}, {1, 226, 5, 3},
    {1, 23, 2, 3}, {1, 204, 2, 2}, {-1, 14, 2, 1}, {1, 219, 3, 2},
    {1, 241, 5, 2}, {-1, 186, 13, 1}, {-1, 253, 1, 1}, {1, 186, 13, 2},
    {-1, 106, 11, 1}, {-1, 87, 1, 2}, {1, 106, 11, 3}, {1, 249, 3, 2},
    {1, 253, 3, 3}, {1, 2, 1, 2}, {-1, 45, 2, 1}, {-1, 2, 1, 2},
    {-1, 244, 2, 2}, {-1, 68, 3, 1}, {1, 14, 2, 2}, {-1, 214, 2, 2},
    {1, 25, 1, 3}, {1, 68, 3, 2}, {-1, 204, 6, 1}, {1, 204, 3, 1},
    {1, 207, 3, 2}, {-1, 222, 1, 1}, {1, 214, 4, 2}, {-1, 13, 3, 1},
    {1, 13, 3, 2}, {1, 2, 1, 3}, {1, 244, 3, 2}, {1, 40, 1, 3}, {-1, 2, 1, 2},
    {-1, 231, 1, 1}, {1, 2, 1, 2}, {-1, 80, 4, 1}, {-1, 44, 1, 1},
    {1, 44, 2, 1}, {1, 80, 4, 2}, {1, 42, 1, 3}, {-1, 241, 15, 2},
    {-1, 35, 4, 1}, {-1, 2, 1, 2}, {-1, 31, 2, 1}, {-1, 39, 2, 1},
    {1, 241, 14, 1}, {1, 35, 4, 1}, {1, 31, 2, 2}, {-1, 106, 5, 1},
    {1, 106, 5, 2}, {-1, 52, 10, 1}, {1, 52, 4, 1}, {-1, 243, 5, 1},
    {1, 56, 2, 1}, {1, 58, 4, 2}, {-1, 122, 4, 2}, {1, 122, 2, 2},
    {-1, 29, 4, 1}, {-1, 206, 2, 1}, {-1, 42, 4, 1}, {-1, 84, 1, 1},
    {-1, 227, 1, 1}, {1, 42, 3, 1}, {-1, 235, 3, 2}, {-1, 233, 1, 2},
    {-1, 239, 1, 2}, {1, 233, 4, 2}, {1, 32, 1, 3}, {1, 243, 4, 3},
    {1, 45, 2, 2}, {-1, 86, 1, 2}, {1, 1, 1, 3}, {-1, 85, 1, 2}, {1, 84, 2, 2},
    {1, 3, 1, 3}, {-1, 197, 3, 3}, {-1, 120, 2, 2}, {1, 124, 3, 3},
    {1, 40, 1, 2}, {-1, 1, 1, 1}, {1, 1, 1, 2}, {-1, 224, 1, 1},
    {-1, 248, 2, 1}, {1, 26, 1, 2}, {-1, 34, 1, 1}, {-1, 161, 7, 2},
    {-1, 33, 1, 1}, {1, 161, 4, 1}, {-1, 124, 19, 2}, {1, 124, 5, 2},
    {-1, 40, 1, 1}, {-1, 32, 1, 2}, {1, 129, 8, 2}, {-1, 111, 5, 1},
    {1, 86, 2, 2}, {-1, 201, 3, 2}, {1, 111, 3, 2}, {1, 137, 5, 2},
    {-1, 176, 21, 2}, {1, 176, 17, 2}, {-1, 184, 5, 1}, {-1, 228, 2, 1},
    {-1, 254, 1, 1}, {1, 165, 3, 2}, {-1, 205, 1, 2}, {-1, 1, 1, 1},
    {-1, 149, 16, 2}, {1, 149, 3, 2}, {-1, 13, 3, 1}, {1, 152, 6, 1},
    {-1, 242, 3, 1}, {-1, 117, 1, 1}, {-1, 246, 1, 1}, {-1, 204, 1, 2},
    {-1, 182, 2, 2}, {-1, 212, 1, 1}, {1, 193, 14, 1}, {1, 158, 4, 1},
    {-1, 215, 5, 1}, {-1, 70, 3, 2}, {-1, 213, 1, 2}, {1, 182, 6, 2},
    {1, 215, 5, 3}, {-1, 82, 3, 2}, {1, 246, 4, 3}, {-1, 58, 4, 2},
    {-1, 186, 2, 2}, {-1, 69, 1, 2}, {-1, 68, 1, 2}, {-1, 49, 6, 1},
    {-1, 62, 1, 1}, {1, 49, 4, 1}, {1, 58, 3, 1}, {1, 53, 2, 2},
    {-1, 35, 4, 1}, {-1, 178, 3, 1}, {-1, 223, 1, 2}, {1, 36, 1, 2},
    {1, 68, 5, 3}, {-1, 230, 1, 2}, {-1, 194, 3, 1}, {-1, 63, 1, 1},
    {1, 227, 4, 1}, {1, 4, 1, 2}, {-1, 36, 1, 2}, {1, 36, 1, 2},
    {-1, 36, 1, 2}, {1, 36, 3, 3}, {1, 61, 3, 3}, {-1, 116, 1, 2},
    {1, 114, 5, 3}, {1, 40, 1, 3}, {1, 41, 1, 2}, {1, 82, 2, 2},
    {-1, 27, 2, 1}, {-1, 17, 6, 1}, {-1, 240, 2, 2}, {-1, 100, 11, 1},
    {-1, 25, 2, 1}, {-1, 176, 1, 1}, {-1, 168, 3, 1}, {1, 100, 9, 1},
    {-1, 235, 1, 1}, {-1, 36, 1, 1}, {1, 237, 8, 2}, {1, 25, 2, 2},
    {1, 22, 1, 3}, {-1, 241, 2, 2}, {1, 162, 3, 3}, {1, 27, 1, 3},
    {1, 28, 1, 2}, {1, 168, 3, 3}, {1, 178, 3, 2}, {1, 1, 1, 3},
    {-1, 250, 3, 2}, {-1, 230, 1, 2}, {-1, 189, 4, 2}, {1, 186, 3, 2},
    {-1, 23, 5, 1}, {1, 23, 3, 1}, {-1, 233, 2, 1}, {-1, 28, 1, 1},
    {1, 14, 1, 2}, {-1, 14, 1, 1}, {1, 26, 2, 1}, {-1, 67, 3, 2},
    {1, 67, 3, 3}, {1, 109, 2, 2}, {1, 28, 1, 3}, {-1, 159, 13, 2},
    {1, 159, 10, 2}, {-1, 25, 2, 1}, {-1, 193, 1, 2}, {1, 189, 5, 2},
    {-1, 115, 2, 1}, {1, 25, 1, 1}, {1, 169, 3, 2}, {-1, 228, 1, 1},
    {-1, 48, 5, 2}, {-1, 211, 1, 2}, {1, 48, 2, 2}, {1, 50, 3, 3},
    {1, 194, 3, 2}, {-1, 221, 1, 2}, {1, 211, 3, 3}, {1, 221, 3, 3},
    {-1, 174, 2, 2}, {-1, 4, 1, 1}, {1, 173, 4, 2}, {-1, 237, 1, 1},
    {1, 233, 3, 2}, {1, 4, 1, 3}, {-1, 83, 1, 3}, {-1, 157, 50, 3},
    {-1, 245, 4, 2}, {-1, 244, 1, 2}, {-1, 95, 20, 2}, {-1, 229, 1, 3},
    {-1, 52, 6, 2}, {-1, 253, 1, 3}, {-1, 37, 1, 3}, {-1, 4, 1, 2},
    {1, 157, 37, 2}, {1, 95, 14, 2}, {1, 52, 3, 2}, {1, 4, 1, 3},
    {1, 36, 2, 2}, {1, 194, 10, 2}, {-1, 235, 2, 2}, {1, 17, 1, 3},
    {-1, 249, 1, 2}, {1, 244, 11, 2}, {-1, 17, 1, 2}, {-1, 117, 3, 3},
    {1, 109, 10, 3}, {-1, 3, 1, 2}, {-1, 140, 2, 2}, {1, 17, 2, 3},
    {-1, 197, 3, 2}, {1, 3, 1, 3}, {1, 55, 2, 3}, {1, 119, 3, 2},
    {1, 204, 4, 3}, {1, 83, 2, 2}, {1, 140, 3, 3}, {-1, 251, 2, 2},
    {-1, 100, 11, 1}, {1, 100, 9, 1}, {1, 197, 3, 2}, {1, 109, 2, 3},
    {1, 228, 3, 3}, {1, 13, 1, 2}, {1, 235, 3, 3}, {-1, 62, 1, 2},
    {-1, 71, 5, 1}, {-1, 154, 8, 2}, {1, 71, 3, 2}, {-1, 18, 1, 1},
    {-1, 254, 1, 1}, {-1, 17, 1, 1}, {1, 17, 2, 2}, {1, 74, 2, 3},
    {-1, 18, 1, 2}, {1, 154, 5, 2}, {-1, 233, 6, 1}, {1, 233, 6, 2},
    {1, 159, 3, 3}, {-1, 240, 1, 3}, {-1, 1, 1, 2}, {-1, 61, 1, 2},
    {1, 1, 1, 3}, {-1, 17, 1, 3}, {-1, 44, 5, 2}, {-1, 13, 1, 1},
    {-1, 73, 2, 2}, {-1, 59, 1, 1}, {1, 44, 5, 2}, {-1, 82, 4, 1},
    {1, 82, 4, 2}, {-1, 182, 15, 1}, {-1, 37, 2, 1}, {-1, 253, 1, 2},
    {-1, 58, 1, 3}, {1, 18, 1, 3}, {1, 182, 8, 3}, {1, 37, 2, 3},
    {-1, 75, 1, 3}, {1, 57, 3, 2}, {1, 29, 3, 3}, {1, 190, 4, 3},
    {-1, 59, 1, 2}, {-1, 234, 2, 1}, {-1, 1, 1, 2}, {1, 61, 2, 3},
    {1, 73, 2, 3}, {-1, 3, 2, 2}, {1, 194, 3, 3}, {-1, 239, 1, 3},
    {1, 239, 3, 3}, {1, 251, 4, 3}, {1, 3, 1, 3}, {1, 4, 1, 2},
    {-1, 243, 3, 2}, {1, 242, 4, 3}, {1, 6, 1, 2}, {-1, 139, 41, 1},
    {1, 139, 40, 1}, {1, 14, 1, 2}, {1, 21, 1, 3}, {1, 234, 2, 2},
    {-1, 18, 1, 1}, {1, 18, 1, 2}, {1, 26, 1, 3}, {-1, 57, 1, 2},
    {1, 33, 1, 3}, {1, 39, 1, 2}, {-1, 30, 2, 1}, {-1, 42, 1, 2},
    {-1, 84, 1, 1}, {1, 30, 1, 1}, {1, 31, 1, 2}, {-1, 209, 10, 1},
    {-1, 48, 3, 2}, {1, 209, 4, 2}, {-1, 58, 1, 3}, {-1, 130, 8, 2},
    {1, 48, 3, 3}, {1, 130, 4, 3}, {1, 57, 2, 3}, {1, 213, 5, 3},
    {1, 134, 3, 3}, {1, 59, 1, 2}, {1, 13, 1, 3}, {1, 64, 1, 2}, {1, 2, 1, 3},
    {1, 66, 1, 2}, {1, 19, 1, 3}, {-1, 18, 1, 2}, {1, 18, 1, 3}, {1, 75, 1, 2},
    {1, 0, 1, 3}, {1, 77, 1, 2}, {1, 5, 1, 3}, {1, 84, 1, 2}, {-1, 225, 5, 2},
    {-1, 253, 1, 1}, {1, 224, 4, 1}, {1, 228, 2, 2}, {1, 20, 1, 3},
    {-1, 69, 7, 2}, {-1, 50, 8, 1}, {-1, 79, 3, 1}, {1, 51, 2, 1},
    {1, 53, 3, 1}, {1, 69, 6, 1}, {1, 56, 1, 1}, {-1, 47, 3, 1}, {1, 48, 2, 2},
    {-1, 52, 1, 1}, {-1, 91, 18, 2}, {-1, 63, 2, 1}, {1, 91, 16, 1},
    {-1, 45, 2, 1}, {-1, 38, 1, 2}, {-1, 11, 1, 1}, {-1, 49, 1, 2},
    {1, 63, 2, 3}, {-1, 37, 1, 2}, {-1, 101, 6, 2}, {-1, 221, 4, 1},
    {1, 79, 3, 2}, {-1, 170, 8, 1}, {1, 101, 2, 1}, {-1, 44, 1, 2},
    {1, 103, 4, 2}, {1, 170, 7, 2}, {1, 221, 3, 2}, {-1, 33, 1, 1},
    {1, 107, 2, 2}, {-1, 178, 1, 3}, {1, 177, 2, 3}, {-1, 36, 1, 2},
    {-1, 182, 13, 1}, {-1, 13, 1, 2}, {-1, 169, 6, 1}, {-1, 14, 1, 2},
    {-1, 239, 14, 2}, {-1, 175, 4, 3}, {-1, 210, 8, 3}, {-1, 0, 1, 2},
    {1, 169, 3, 2}, {1, 172, 7, 2}, {-1, 54, 3, 2}, {1, 182, 13, 3},
    {1, 54, 4, 2}, {-1, 81, 3, 1}, {-1, 12, 1, 2}, {-1, 48, 1, 2},
    {-1, 225, 5, 2}, {1, 239, 14, 2}, {1, 210, 5, 2}, {-1, 241, 8, 1},
    {1, 81, 3, 2}, {-1, 51, 1, 1}, {1, 12, 1, 2}, {-1, 68, 3, 1},
    {1, 68, 3, 2}, {1, 215, 4, 3}, {1, 16, 1, 2}, {-1, 209, 4, 1},
    {1, 14, 1, 2}, {-1, 233, 1, 1}, {1, 44, 3, 1}, {-1, 234, 2, 1},
    {1, 241, 8, 2}, {1, 47, 4, 2}, {-1, 16, 1, 1}, {-1, 14, 1, 2},
    {-1, 46, 5, 2}, {1, 16, 1, 3}, {1, 46, 6, 2}, {-1, 10, 1, 1},
    {1, 209, 4, 2}, {-1, 50, 2, 1}, {1, 50, 2, 2}, {1, 10, 1, 3},
    {-1, 112, 14, 2}, {1, 112, 14, 3}, {-1, 41, 1, 2}, {1, 224, 5, 2},
    {-1, 87, 1, 2}, {-1, 39, 2, 3}, {1, 33, 1, 2}, {1, 36, 2, 2},
    {-1, 16, 1, 1}, {-1, 230, 1, 2}, {1, 38, 2, 2}, {-1, 33, 1, 1},
    {1, 0, 1, 2}, {-1, 60, 2, 1}, {1, 16, 1, 2}, {-1, 28, 1, 1},
    {-1, 232, 1, 2}, {1, 229, 6, 2}, {-1, 45, 2, 1}, {1, 40, 2, 2},
    {1, 45, 2, 3}, {-1, 254, 1, 3}, {-1, 36, 6, 2}, {1, 36, 4, 2},
    {1, 28, 1, 3}, {-1, 12, 1, 2}, {-1, 245, 2, 1}, {1, 253, 3, 2},
    {-1, 46, 1, 1}, {1, 40, 2, 2}, {1, 12, 1, 3}, {1, 60, 2, 2},
    {-1, 16, 1, 1}, {1, 14, 1, 2}, {-1, 227, 2, 1}, {1, 16, 1, 2},
    {-1, 241, 1, 1}, {1, 75, 2, 2}, {1, 33, 1, 3}, {1, 46, 1, 2},
    {-1, 33, 1, 1}, {1, 87, 2, 2}, {1, 227, 2, 3}, {1, 245, 2, 2},
    {1, 1, 1, 3}, {1, 33, 1, 2}, {-1, 12, 1, 1}, {-1, 14, 1, 2},
    {-1, 38, 2, 1}, {-1, 244, 4, 2}, {-1, 36, 2, 2}, {-1, 158, 10, 1},
    {1, 158, 9, 1}, {1, 36, 3, 1}, {1, 244, 2, 1}, {-1, 29, 1, 2},
    {1, 32, 1, 3}, {1, 246, 2, 2}, {1, 15, 1, 3}
  };
  
  assert(sizeof(operations) / sizeof(operations[0]) == 0x400);
  
  for (size_t i = 0; i < 0x400; ++i) {
    if (operations[i].type == 1) {
      // Subtract 1 from the size just to make sure that the ChunkedFreeList
      // subclass is doing its job.
      assert(pfl->Alloc(addr, operations[i].size * regionSize - 1));
      assert(addr == start + operations[i].address * regionSize);
      assert(pfl->GetStackCount() == operations[i].stackCount);
    } else {
      // Subtract 1 from the size just like above
      pfl->Dealloc(start + operations[i].address * regionSize,
          operations[i].size * regionSize - 1);
      assert(pfl->GetStackCount() == operations[i].stackCount);
    }
  }
  
  free(buffer);
}
