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

  // NOTE: since double cascades are *very* rare, this acts as a sanity check
  // more than an extensive all-covering test.
  
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
    {1, 2, 13, 1}, {1, 15, 44, 1}, {1, 59, 153, 1}, {1, 212, 20, 1},
    {1, 232, 21, 1}, {1, 253, 1, 1}, {1, 254, 1, 1}, {1, 255, 1, 2},
    {-1, 74, 103, 1}, {1, 74, 20, 1}, {1, 94, 37, 1}, {1, 131, 21, 1},
    {1, 152, 10, 1}, {-1, 185, 69, 1}, {-1, 20, 124, 1}, {1, 21, 48, 1},
    {-1, 15, 1, 2}, {1, 69, 3, 2}, {1, 72, 42, 2}, {1, 114, 13, 2},
    {1, 185, 62, 2}, {-1, 224, 6, 1}, {1, 127, 12, 1}, {-1, 19, 1, 2},
    {1, 163, 13, 2}, {1, 247, 7, 3}, {-1, 18, 1, 2}, {-1, 12, 2, 1},
    {-1, 194, 30, 1}, {-1, 14, 1, 1}, {1, 194, 12, 1}, {1, 206, 16, 1},
    {-1, 147, 11, 1}, {1, 147, 8, 1}, {1, 139, 5, 2}, {-1, 231, 18, 1},
    {-1, 160, 2, 1}, {-1, 10, 1, 2}, {-1, 251, 5, 1}, {1, 222, 5, 1},
    {-1, 11, 1, 2}, {1, 231, 8, 2}, {-1, 118, 5, 1}, {1, 239, 8, 1},
    {-1, 25, 64, 2}, {1, 25, 19, 2}, {-1, 134, 9, 1}, {1, 44, 20, 1},
    {1, 64, 20, 1}, {-1, 7, 3, 1}, {-1, 132, 1, 1}, {-1, 243, 1, 2},
    {-1, 221, 1, 1}, {-1, 127, 1, 2}, {1, 84, 1, 2}, {1, 85, 2, 2},
    {-1, 143, 12, 3}, {-1, 230, 2, 3}, {1, 134, 11, 3}, {-1, 159, 1, 3},
    {-1, 139, 3, 2}, {-1, 3, 3, 1}, {1, 145, 10, 1}, {1, 3, 2, 1},
    {1, 5, 1, 2}, {1, 118, 3, 2}, {1, 87, 2, 3}, {1, 139, 3, 2},
    {-1, 175, 1, 2}, {-1, 3, 1, 1}, {-1, 158, 1, 2}, {1, 155, 5, 2},
    {1, 227, 5, 3}, {-1, 246, 1, 3}, {1, 251, 4, 3}, {1, 121, 2, 2},
    {-1, 141, 12, 1}, {1, 141, 8, 1}, {-1, 197, 23, 2}, {1, 11, 1, 3},
    {1, 197, 21, 3}, {1, 149, 3, 3}, {-1, 5, 2, 2}, {1, 5, 2, 3},
    {1, 160, 2, 2}, {-1, 198, 15, 1}, {1, 198, 3, 1}, {1, 201, 6, 1},
    {1, 207, 4, 1}, {1, 14, 1, 2}, {1, 246, 3, 3}, {1, 20, 1, 2},
    {1, 175, 2, 3}, {1, 13, 1, 2}, {-1, 6, 1, 1}, {1, 211, 2, 2}, {1, 6, 1, 3},
    {-1, 11, 1, 2}, {1, 11, 1, 3}, {1, 218, 2, 2}, {1, 0, 1, 3},
    {1, 127, 1, 2}, {-1, 0, 1, 1}, {1, 0, 1, 2}, {-1, 237, 5, 1},
    {-1, 0, 1, 2}, {1, 237, 4, 2}, {1, 19, 1, 3}, {1, 132, 1, 2},
    {-1, 99, 40, 1}, {-1, 254, 1, 1}, {1, 99, 29, 1}, {1, 128, 9, 1},
    {-1, 14, 1, 2}, {-1, 154, 2, 1}, {-1, 248, 4, 2}, {-1, 191, 23, 1},
    {1, 191, 13, 1}, {-1, 6, 1, 2}, {-1, 35, 62, 1}, {1, 35, 30, 1},
    {1, 65, 23, 1}, {-1, 17, 1, 2}, {-1, 178, 25, 1}, {-1, 215, 2, 1},
    {-1, 223, 9, 1}, {1, 178, 22, 1}, {-1, 87, 1, 2}, {1, 90, 3, 2},
    {1, 93, 4, 3}, {-1, 214, 1, 2}, {-1, 220, 1, 2}, {-1, 222, 1, 3},
    {1, 200, 3, 3}, {1, 204, 13, 2}, {-1, 148, 1, 1}, {-1, 242, 1, 2},
    {-1, 232, 3, 2}, {-1, 126, 10, 1}, {-1, 245, 2, 2}, {-1, 5, 1, 1},
    {-1, 136, 1, 2}, {1, 126, 10, 2}, {-1, 70, 16, 1}, {1, 70, 11, 1},
    {-1, 16, 1, 1}, {-1, 153, 1, 2}, {-1, 4, 1, 2}, {1, 4, 1, 2},
    {-1, 183, 5, 1}, {1, 220, 10, 1}, {-1, 244, 1, 2}, {-1, 123, 10, 1},
    {-1, 4, 1, 1}, {-1, 29, 27, 1}, {-1, 150, 2, 1}, {-1, 2, 1, 2},
    {-1, 197, 22, 1}, {-1, 253, 1, 1}, {-1, 118, 2, 2}, {1, 29, 21, 2},
    {-1, 149, 1, 3}, {-1, 237, 2, 2}, {1, 50, 5, 2}, {1, 123, 10, 3},
    {1, 197, 19, 3}, {-1, 170, 10, 2}, {1, 148, 7, 2}, {1, 170, 10, 3},
    {-1, 57, 3, 2}, {-1, 76, 1, 1}, {-1, 11, 1, 2}, {1, 81, 4, 2},
    {-1, 252, 1, 3}, {-1, 179, 4, 3}, {-1, 235, 1, 3}, {-1, 13, 1, 2},
    {-1, 126, 5, 1}, {1, 179, 8, 1}, {-1, 92, 4, 2}, {-1, 154, 1, 2},
    {1, 248, 7, 2}, {-1, 227, 1, 1}, {1, 16, 2, 1}, {1, 92, 4, 2},
    {1, 18, 1, 3}, {1, 57, 2, 3}, {-1, 236, 1, 2}, {1, 12, 1, 3},
    {1, 230, 9, 2}, {1, 126, 4, 2}, {-1, 146, 4, 1}, {-1, 25, 25, 2},
    {1, 25, 14, 2}, {-1, 141, 2, 1}, {-1, 12, 1, 2}, {1, 39, 5, 2},
    {-1, 53, 2, 2}, {1, 44, 6, 3}, {-1, 151, 2, 2}, {-1, 28, 7, 1},
    {-1, 153, 1, 2}, {1, 28, 3, 2}, {-1, 210, 5, 1}, {-1, 68, 3, 1},
    {1, 146, 4, 2}, {1, 151, 5, 3}, {1, 241, 6, 2}, {-1, 48, 1, 1},
    {1, 210, 5, 2}, {-1, 74, 2, 2}, {-1, 251, 3, 1}, {-1, 140, 1, 1},
    {1, 32, 3, 2}, {-1, 183, 3, 1}, {1, 53, 3, 2}, {-1, 254, 1, 3},
    {1, 251, 4, 3}, {1, 3, 1, 2}, {1, 68, 3, 3}, {1, 74, 3, 2},
    {-1, 144, 4, 1}, {-1, 58, 1, 1}, {1, 5, 1, 2}, {-1, 159, 1, 1},
    {1, 58, 2, 2}, {1, 118, 2, 3}, {-1, 86, 1, 3}, {1, 85, 2, 2},
    {-1, 157, 2, 2}, {-1, 122, 2, 1}, {1, 122, 2, 2}, {1, 136, 3, 3},
    {-1, 143, 1, 2}, {1, 140, 4, 2}, {-1, 224, 2, 1}, {-1, 174, 5, 2},
    {1, 174, 5, 3}, {1, 144, 2, 3}, {1, 10, 1, 2}, {1, 146, 2, 3},
    {1, 15, 1, 2}, {-1, 160, 1, 2}, {1, 31, 1, 3}, {1, 157, 4, 2},
    {-1, 226, 1, 3}, {-1, 5, 1, 3}, {-1, 202, 4, 2}, {1, 5, 2, 3},
    {-1, 215, 1, 3}, {1, 183, 3, 2}, {-1, 169, 17, 1}, {1, 169, 12, 1},
    {1, 181, 2, 1}, {-1, 10, 1, 2}, {-1, 212, 1, 1}, {-1, 186, 1, 2},
    {1, 183, 3, 2}, {1, 202, 4, 3}, {-1, 214, 1, 3}, {1, 214, 5, 2},
    {-1, 55, 26, 1}, {1, 55, 12, 1}, {-1, 5, 2, 2}, {1, 67, 8, 2},
    {1, 75, 6, 3}, {-1, 246, 7, 2}, {-1, 253, 1, 2}, {1, 5, 1, 2},
    {1, 246, 8, 3}, {-1, 244, 1, 2}, {-1, 216, 4, 1}, {-1, 74, 5, 2},
    {1, 74, 4, 2}, {1, 216, 4, 3}, {-1, 183, 3, 3}, {1, 183, 5, 2},
    {1, 224, 2, 2}, {-1, 135, 12, 1}, {-1, 132, 2, 2}, {-1, 115, 5, 1},
    {-1, 229, 14, 2}, {1, 229, 13, 2}, {-1, 241, 1, 2}, {1, 135, 12, 3},
    {-1, 131, 1, 2}, {1, 115, 5, 3}, {-1, 3, 1, 3}, {1, 130, 4, 2},
    {-1, 5, 1, 2}, {-1, 243, 1, 3}, {1, 3, 3, 2}, {-1, 254, 1, 2},
    {-1, 5, 1, 2}, {1, 241, 3, 2}, {-1, 195, 11, 1}, {-1, 137, 5, 1},
    {-1, 182, 11, 2}, {-1, 61, 7, 1}, {-1, 172, 9, 2}, {-1, 80, 2, 1},
    {-1, 181, 1, 2}, {1, 172, 8, 2}, {-1, 219, 6, 1}, {1, 180, 10, 1},
    {1, 61, 3, 1}, {-1, 211, 1, 1}, {-1, 50, 7, 2}, {1, 50, 2, 2},
    {-1, 61, 1, 1}, {-1, 225, 1, 2}, {-1, 82, 1, 2}, {-1, 183, 5, 1},
    {-1, 146, 3, 1}, {1, 195, 7, 1}, {-1, 19, 27, 1}, {1, 20, 10, 1},
    {-1, 63, 1, 1}, {-1, 135, 2, 1}, {-1, 86, 1, 1}, {1, 31, 9, 1},
    {-1, 208, 3, 1}, {-1, 49, 3, 1}, {1, 41, 2, 1}, {1, 135, 6, 1},
    {-1, 20, 2, 1}, {1, 219, 9, 2}, {-1, 153, 1, 1}, {-1, 142, 2, 1},
    {-1, 79, 1, 2}, {1, 63, 5, 3}, {1, 21, 1, 2}, {-1, 206, 2, 3},
    {1, 202, 8, 3}, {-1, 252, 1, 2}, {1, 43, 3, 3}, {-1, 144, 2, 2},
    {-1, 168, 14, 1}, {1, 53, 4, 2}, {1, 78, 4, 2}, {-1, 166, 1, 1},
    {1, 168, 14, 2}, {-1, 83, 3, 3}, {1, 14, 1, 3}, {1, 82, 5, 2},
    {1, 49, 3, 2}, {-1, 68, 11, 1}, {-1, 102, 36, 2}, {1, 68, 5, 2},
    {-1, 32, 8, 2}, {1, 102, 21, 2}, {1, 32, 7, 2}, {1, 123, 9, 2},
    {-1, 36, 3, 2}, {-1, 253, 1, 3}, {1, 36, 4, 3}, {1, 73, 6, 2},
    {-1, 38, 2, 2}, {1, 11, 1, 3}, {-1, 23, 5, 2}, {1, 141, 7, 2},
    {-1, 28, 1, 2}, {-1, 42, 2, 1}, {-1, 49, 1, 1}, {-1, 206, 1, 1},
    {-1, 11, 1, 2}, {1, 25, 4, 3}, {-1, 79, 7, 2}, {1, 38, 2, 2},
    {-1, 77, 1, 1}, {1, 79, 5, 1}, {1, 132, 6, 2}, {-1, 26, 3, 1},
    {-1, 188, 1, 1}, {1, 183, 5, 1}, {1, 26, 2, 1}, {1, 252, 4, 2},
    {-1, 207, 2, 2}, {1, 28, 1, 3}, {-1, 164, 1, 2}, {1, 40, 1, 3},
    {-1, 165, 1, 2}, {-1, 31, 10, 1}, {-1, 54, 1, 2}, {1, 31, 8, 2},
    {-1, 189, 1, 3}, {1, 39, 2, 2}, {1, 164, 3, 3}, {1, 10, 1, 2},
    {-1, 220, 8, 1}, {-1, 4, 1, 2}, {-1, 174, 13, 1}, {1, 174, 3, 1},
    {-1, 250, 3, 2}, {-1, 25, 5, 1}, {-1, 254, 1, 1}, {1, 177, 7, 1},
    {1, 26, 1, 1}, {1, 188, 4, 1}, {-1, 150, 1, 1}, {-1, 209, 1, 2},
    {-1, 201, 2, 1}, {-1, 38, 3, 1}, {1, 220, 8, 2}, {-1, 22, 1, 1},
    {1, 38, 2, 1}, {-1, 187, 4, 1}, {1, 22, 1, 2}, {-1, 10, 1, 1},
    {-1, 183, 1, 1}, {-1, 51, 1, 1}, {-1, 86, 1, 1}, {-1, 26, 1, 2},
    {-1, 219, 12, 1}, {-1, 21, 1, 2}, {-1, 152, 1, 2}, {1, 219, 12, 3},
    {1, 183, 5, 3}, {1, 206, 5, 3}, {-1, 255, 1, 3}, {1, 26, 1, 3},
    {-1, 26, 1, 2}, {-1, 3, 1, 1}, {-1, 199, 1, 2}, {1, 84, 3, 3},
    {1, 188, 3, 2}, {1, 28, 2, 3}, {-1, 53, 1, 2}, {1, 42, 2, 3}, {1, 4, 1, 3},
    {-1, 4, 1, 2}, {-1, 223, 2, 1}, {1, 51, 2, 1}, {1, 250, 3, 2},
    {-1, 151, 1, 3}, {1, 150, 3, 3}, {-1, 22, 1, 2}, {-1, 200, 1, 3},
    {-1, 51, 2, 3}, {-1, 66, 10, 2}, {1, 51, 2, 2}, {1, 66, 9, 2},
    {-1, 213, 8, 2}, {1, 53, 2, 3}, {-1, 151, 1, 2}, {-1, 172, 18, 1},
    {-1, 78, 7, 1}, {1, 172, 11, 1}, {1, 211, 9, 1}, {1, 77, 4, 1},
    {-1, 191, 1, 1}, {-1, 150, 1, 1}, {1, 81, 4, 2}, {1, 183, 5, 2},
    {1, 4, 1, 3}, {1, 22, 1, 2}, {1, 199, 4, 3}, {1, 5, 1, 2}, {1, 13, 1, 3},
    {-1, 86, 1, 3}, {-1, 34, 5, 2}, {-1, 41, 4, 2}, {-1, 45, 2, 2},
    {-1, 47, 1, 2}, {1, 34, 4, 2}, {1, 40, 8, 3}, {-1, 149, 1, 2},
    {-1, 154, 6, 2}, {1, 153, 7, 3}, {1, 86, 3, 2}, {1, 148, 2, 2},
    {1, 8, 1, 3}, {1, 24, 1, 2}, {-1, 190, 1, 3}, {-1, 196, 2, 2},
    {1, 7, 1, 3}, {-1, 71, 3, 2}, {-1, 156, 6, 2}, {1, 156, 6, 2},
    {-1, 194, 1, 1}, {1, 71, 3, 2}, {-1, 221, 1, 2}, {1, 188, 4, 2},
    {1, 26, 1, 3}, {-1, 14, 3, 2}, {-1, 7, 1, 1}, {-1, 195, 1, 2},
    {-1, 252, 1, 1}, {1, 14, 3, 2}, {-1, 15, 4, 1}, {1, 15, 4, 2},
    {1, 7, 1, 3}, {-1, 5, 1, 2}, {1, 150, 2, 3}, {1, 5, 1, 2}, {1, 194, 2, 2},
    {1, 25, 1, 3}, {-1, 54, 3, 2}, {1, 54, 2, 2}, {1, 196, 2, 3},
    {1, 30, 1, 2}, {1, 220, 2, 3}, {-1, 22, 1, 2}, {1, 223, 2, 3},
    {1, 22, 1, 2}, {-1, 84, 5, 1}, {1, 23, 1, 2}, {-1, 189, 3, 2},
    {-1, 58, 1, 1}, {-1, 47, 1, 2}, {1, 84, 2, 2}, {1, 189, 4, 3},
    {1, 38, 1, 2}, {-1, 4, 2, 1}, {1, 86, 3, 2}, {1, 4, 2, 3}, {-1, 253, 1, 2},
    {1, 0, 1, 3}, {1, 252, 2, 3}, {1, 20, 1, 2}, {-1, 141, 1, 1},
    {-1, 57, 1, 2}, {-1, 42, 5, 2}, {1, 42, 4, 2}, {-1, 148, 14, 2},
    {1, 148, 14, 2}, {1, 56, 3, 3}, {-1, 8, 1, 2}, {-1, 167, 26, 1},
    {-1, 148, 8, 2}, {1, 148, 3, 2}, {-1, 105, 8, 1}, {1, 167, 12, 1},
    {-1, 52, 7, 2}, {-1, 145, 3, 1}, {1, 179, 11, 1}, {-1, 144, 1, 1},
    {-1, 29, 10, 1}, {1, 30, 3, 1}, {-1, 227, 17, 1}, {-1, 143, 1, 1},
    {1, 227, 10, 1}, {1, 33, 3, 1}, {1, 52, 7, 2}, {1, 105, 8, 3},
    {-1, 247, 2, 2}, {1, 143, 5, 3}, {1, 36, 2, 3}, {-1, 20, 1, 2},
    {-1, 100, 1, 1}, {1, 237, 6, 1}, {1, 46, 2, 2}, {1, 151, 2, 2},
    {1, 20, 1, 3}, {-1, 241, 2, 3}, {1, 153, 3, 2}, {-1, 0, 1, 1},
    {-1, 28, 1, 2}, {1, 241, 4, 3}, {-1, 249, 1, 3}, {1, 11, 1, 3},
    {-1, 20, 1, 2}, {-1, 209, 19, 1}, {1, 209, 19, 2}, {-1, 167, 1, 1},
    {-1, 16, 1, 2}, {1, 27, 2, 3}, {1, 20, 1, 2}, {-1, 20, 1, 1}, {1, 2, 1, 2},
    {-1, 56, 2, 1}, {1, 190, 3, 2}, {-1, 15, 1, 1}, {-1, 114, 20, 2},
    {1, 114, 4, 2}, {-1, 14, 1, 1}, {-1, 253, 1, 1}, {-1, 181, 16, 2},
    {-1, 159, 3, 2}, {1, 118, 8, 2}, {-1, 2, 1, 1}, {1, 56, 2, 2},
    {-1, 13, 1, 1}, {-1, 251, 2, 1}, {-1, 18, 1, 2}, {-1, 97, 2, 1},
    {1, 181, 9, 1}, {-1, 116, 3, 2}, {-1, 154, 3, 1}, {1, 126, 8, 2},
    {1, 18, 1, 3}, {1, 190, 7, 2}, {-1, 146, 7, 1}, {1, 146, 5, 1},
    {-1, 111, 4, 2}, {1, 251, 5, 3}, {1, 20, 1, 2}, {-1, 80, 3, 1},
    {1, 80, 3, 2}, {1, 97, 2, 3}, {-1, 253, 3, 2}, {1, 111, 2, 2},
    {-1, 74, 1, 2}, {-1, 30, 8, 2}, {-1, 145, 4, 1}, {-1, 55, 3, 2},
    {-1, 5, 1, 1}, {-1, 115, 1, 2}, {1, 5, 1, 3}, {1, 30, 9, 2},
    {1, 113, 6, 3}, {1, 55, 2, 3}, {1, 145, 3, 3}, {-1, 55, 2, 3},
    {1, 6, 1, 2}, {1, 55, 2, 2}, {-1, 189, 17, 1}, {-1, 153, 1, 2},
    {1, 151, 4, 2}, {-1, 56, 1, 2}, {1, 159, 3, 2}, {-1, 252, 1, 2},
    {1, 189, 14, 2}, {-1, 11, 1, 1}, {-1, 98, 1, 2}, {1, 56, 2, 3},
    {-1, 69, 1, 2}, {1, 203, 3, 3}, {1, 74, 2, 2}, {1, 252, 4, 3},
    {1, 247, 3, 2}, {1, 155, 2, 3}, {1, 8, 1, 2}, {1, 2, 1, 3},
    {-1, 107, 5, 2}, {1, 107, 4, 2}, {1, 12, 1, 3}, {1, 16, 1, 2},
    {-1, 2, 1, 1}, {-1, 12, 1, 2}, {-1, 166, 1, 2}, {-1, 146, 2, 2},
    {-1, 74, 13, 1}, {-1, 6, 1, 2}, {-1, 99, 1, 3}, {1, 74, 3, 3},
    {-1, 75, 1, 2}, {-1, 88, 1, 1}, {1, 77, 10, 2}, {-1, 143, 3, 2},
    {-1, 57, 2, 1}, {-1, 36, 3, 2}, {1, 29, 1, 3}, {1, 143, 6, 2},
    {1, 36, 3, 3}, {-1, 16, 1, 2}, {1, 3, 1, 3}, {-1, 59, 1, 3}, {-1, 4, 1, 2},
    {-1, 3, 1, 2}, {1, 57, 3, 3}, {1, 3, 2, 2}, {1, 14, 1, 3}, {-1, 125, 9, 2},
    {-1, 18, 1, 1}, {1, 16, 1, 2}, {1, 125, 4, 2}, {-1, 7, 2, 1},
    {-1, 27, 13, 1}, {1, 27, 12, 1}, {-1, 158, 1, 2}, {1, 98, 2, 2},
    {-1, 76, 5, 2}, {1, 75, 6, 3}, {1, 129, 2, 3}, {-1, 34, 3, 2},
    {1, 34, 3, 3}, {1, 18, 1, 2}, {1, 131, 2, 2}, {1, 12, 1, 3},
    {1, 166, 2, 2}, {-1, 225, 22, 1}, {1, 225, 5, 1}, {1, 230, 3, 1},
    {1, 233, 3, 1}, {-1, 68, 1, 1}, {-1, 161, 1, 1}, {-1, 27, 6, 2},
    {-1, 92, 4, 1}, {1, 27, 4, 1}, {1, 236, 11, 2}, {1, 92, 3, 2},
    {1, 31, 2, 3}, {-1, 20, 1, 2}, {-1, 5, 1, 1}, {-1, 132, 1, 1},
    {-1, 16, 2, 2}, {1, 16, 1, 2}, {-1, 159, 1, 2}, {-1, 176, 34, 1},
    {-1, 73, 6, 2}, {-1, 97, 2, 1}, {1, 176, 21, 1}, {-1, 96, 1, 2},
    {-1, 245, 1, 1}, {-1, 151, 4, 2}, {1, 73, 5, 2}, {-1, 99, 1, 3},
    {1, 197, 7, 3}, {1, 95, 5, 3}, {-1, 42, 6, 2}, {1, 39, 1, 3},
    {-1, 248, 5, 2}, {-1, 246, 1, 2}, {-1, 143, 1, 1}, {1, 42, 4, 1},
    {-1, 247, 1, 2}, {-1, 242, 2, 1}, {-1, 104, 6, 1}, {1, 104, 6, 2},
    {-1, 4, 1, 1}, {1, 245, 8, 2}, {1, 151, 3, 2}, {-1, 145, 9, 2},
    {1, 145, 9, 2}, {1, 204, 5, 2}, {1, 68, 2, 3}, {-1, 12, 1, 2},
    {1, 4, 1, 3}, {1, 132, 2, 2}, {-1, 62, 11, 2}, {-1, 4, 1, 1},
    {-1, 236, 5, 2}, {-1, 105, 4, 1}, {-1, 160, 1, 2}, {-1, 97, 2, 1},
    {-1, 109, 1, 1}, {-1, 91, 3, 2}, {-1, 116, 21, 1}, {1, 61, 9, 1},
    {1, 70, 2, 1}, {-1, 172, 20, 2}, {-1, 139, 2, 2}, {1, 116, 16, 2},
    {1, 91, 2, 2}, {-1, 90, 3, 2}, {1, 105, 5, 3}, {1, 172, 7, 3},
    {-1, 150, 3, 2}, {1, 179, 7, 2}, {1, 47, 1, 3}, {1, 132, 5, 2},
    {1, 90, 4, 3}, {1, 139, 3, 2}, {1, 4, 1, 3}, {-1, 205, 1, 2},
    {-1, 53, 1, 1}, {1, 158, 4, 1}, {-1, 23, 16, 2}, {-1, 170, 14, 1},
    {1, 23, 7, 1}, {1, 30, 1, 1}, {-1, 96, 1, 1}, {-1, 207, 1, 1},
    {1, 170, 9, 1}, {1, 32, 7, 2}, {-1, 4, 1, 1}, {-1, 169, 2, 2},
    {-1, 3, 1, 1}, {-1, 23, 2, 2}, {-1, 159, 1, 1}, {1, 23, 1, 1},
    {1, 179, 4, 1}, {-1, 206, 1, 2}, {1, 24, 1, 3}, {1, 186, 6, 2},
    {-1, 55, 13, 1}, {1, 55, 12, 1}, {-1, 14, 1, 2}, {1, 236, 4, 2},
    {1, 17, 1, 3}, {1, 96, 3, 2}, {-1, 126, 7, 1}, {1, 126, 2, 1},
    {-1, 71, 1, 1}, {-1, 36, 3, 2}, {-1, 236, 1, 1}, {-1, 153, 1, 2},
    {1, 128, 5, 3}, {-1, 166, 1, 2}, {-1, 101, 10, 3}, {-1, 220, 1, 2},
    {1, 100, 6, 2}, {1, 36, 3, 3}, {-1, 156, 2, 2}, {1, 71, 2, 3},
    {-1, 158, 1, 2}, {-1, 50, 3, 3}, {-1, 71, 3, 2}, {1, 49, 3, 2},
    {1, 3, 1, 3}, {1, 106, 6, 2}, {-1, 212, 4, 1}, {-1, 50, 1, 2},
    {-1, 30, 1, 1}, {-1, 208, 1, 2}, {-1, 167, 2, 3}, {1, 71, 3, 2},
    {-1, 18, 1, 1}, {-1, 49, 1, 1}, {1, 150, 3, 1}, {-1, 182, 1, 1},
    {1, 156, 4, 2}, {-1, 151, 1, 1}, {-1, 90, 30, 2}, {-1, 152, 1, 3},
    {1, 90, 21, 3}, {1, 111, 4, 3}, {1, 18, 1, 2}, {1, 49, 2, 3},
    {-1, 211, 1, 3}, {1, 5, 1, 2}, {-1, 200, 4, 1}, {-1, 95, 11, 2},
    {1, 95, 11, 3}, {-1, 238, 2, 3}, {1, 115, 4, 3}, {-1, 237, 1, 2},
    {1, 151, 4, 3}, {1, 166, 4, 3}, {-1, 47, 1, 3}, {-1, 167, 1, 2},
    {-1, 63, 4, 2}, {-1, 192, 7, 1}, {1, 192, 7, 2}, {1, 63, 5, 3},
    {-1, 164, 2, 2}, {-1, 210, 1, 3}, {1, 200, 3, 3}, {-1, 50, 2, 3},
    {-1, 219, 1, 3}, {-1, 199, 1, 2}, {-1, 200, 3, 3}, {1, 30, 1, 2},
    {1, 205, 10, 2}, {1, 50, 4, 3}, {1, 0, 1, 2}, {-1, 178, 3, 1},
    {-1, 136, 2, 2}, {-1, 126, 3, 1}, {1, 46, 1, 1}, {1, 47, 1, 2},
    {1, 199, 4, 2}, {1, 236, 4, 2}, {-1, 206, 8, 1}, {-1, 150, 5, 2},
    {-1, 214, 1, 3}, {-1, 163, 1, 2}, {-1, 216, 3, 3}, {1, 206, 6, 3},
    {1, 212, 8, 3}, {1, 126, 3, 3}, {1, 150, 3, 3}, {-1, 175, 2, 2},
    {-1, 166, 1, 3}, {1, 162, 4, 3}, {-1, 79, 2, 2}, {1, 11, 2, 3},
    {1, 79, 2, 2}, {1, 178, 3, 3}, {1, 136, 2, 2}, {-1, 5, 1, 2},
    {-1, 148, 2, 1}, {1, 4, 2, 2}, {1, 148, 2, 3}, {1, 153, 2, 2},
    {-1, 0, 1, 1}, {1, 166, 2, 2}, {1, 175, 2, 3}, {-1, 11, 2, 2},
    {1, 0, 1, 3}, {-1, 214, 2, 2}, {1, 11, 2, 3}, {-1, 217, 1, 2},
    {-1, 94, 19, 1}, {1, 94, 13, 1}, {-1, 159, 1, 2}, {-1, 27, 4, 1},
    {1, 27, 2, 1}, {1, 107, 3, 1}, {1, 19, 1, 2}, {1, 110, 3, 3},
    {1, 29, 2, 2}, {1, 15, 1, 3}, {1, 182, 2, 2}, {-1, 11, 1, 2},
    {-1, 189, 4, 1}, {-1, 154, 3, 1}, {-1, 241, 1, 2}, {-1, 200, 3, 2},
    {-1, 81, 6, 1}, {-1, 132, 5, 2}, {-1, 15, 3, 1}, {-1, 0, 1, 2},
    {-1, 163, 6, 1}, {1, 15, 3, 2}, {1, 81, 2, 2}, {-1, 247, 8, 1},
    {-1, 246, 1, 1}, {1, 163, 6, 2}, {-1, 15, 3, 1}, {-1, 146, 4, 1},
    {-1, 204, 8, 1}, {1, 200, 9, 1}, {1, 16, 1, 1}, {-1, 255, 1, 1},
    {1, 246, 8, 1}, {1, 17, 1, 2}, {-1, 87, 1, 3}, {1, 83, 3, 3},
    {-1, 216, 1, 2}, {1, 86, 3, 3}, {1, 132, 5, 2}, {1, 146, 3, 2},
    {-1, 39, 2, 1}, {1, 39, 2, 2}, {1, 154, 2, 2}, {1, 189, 2, 2},
    {-1, 145, 3, 1}, {1, 145, 3, 2}, {-1, 141, 2, 2}, {1, 10, 1, 3},
    {-1, 148, 1, 3}, {-1, 29, 1, 2}, {-1, 3, 3, 1}, {1, 3, 2, 1},
    {-1, 227, 8, 2}, {1, 227, 8, 3}, {1, 141, 2, 3}, {1, 209, 3, 2},
    {1, 9, 1, 3}, {1, 21, 1, 2}, {1, 214, 4, 3}, {1, 240, 3, 3},
    {-1, 147, 1, 3}, {-1, 154, 2, 3}, {1, 6, 1, 2}, {1, 147, 2, 2},
    {-1, 253, 1, 2}, {-1, 206, 3, 1}, {1, 154, 2, 1}, {-1, 30, 1, 1},
    {1, 15, 1, 2}, {1, 206, 3, 3}, {1, 29, 2, 2}, {1, 191, 2, 3},
    {-1, 204, 8, 2}, {1, 204, 8, 3}, {1, 253, 3, 2}, {-1, 146, 1, 1},
    {-1, 4, 1, 2}, {-1, 147, 2, 3}, {-1, 141, 1, 2}, {1, 146, 2, 2},
    {-1, 12, 1, 3}, {1, 11, 1, 3}, {1, 12, 1, 3}, {1, 148, 2, 2},
    {1, 13, 1, 3}, {-1, 6, 1, 2}, {1, 6, 1, 3}, {1, 89, 1, 2}, {1, 5, 1, 3},
    {-1, 139, 1, 2}, {1, 119, 1, 3}, {-1, 5, 1, 2}, {-1, 11, 2, 1},
    {-1, 10, 1, 1}, {1, 10, 2, 1}, {1, 5, 1, 2}, {1, 12, 1, 3}, {-1, 13, 1, 2},
    {1, 13, 1, 3}, {-1, 172, 25, 2}, {1, 172, 14, 2}, {-1, 140, 1, 3},
    {1, 186, 11, 2}, {1, 4, 1, 3}, {-1, 165, 5, 3}, {1, 139, 3, 2},
    {1, 165, 4, 2}, {1, 78, 1, 3}, {-1, 157, 2, 2}, {1, 8, 1, 3},
    {-1, 6, 1, 2}, {-1, 44, 3, 1}, {1, 44, 2, 1}, {1, 6, 1, 2},
    {-1, 140, 2, 1}, {1, 156, 3, 1}, {1, 46, 1, 2}, {-1, 151, 5, 1},
    {1, 140, 2, 2}, {-1, 162, 1, 1}, {1, 151, 3, 1}, {-1, 221, 19, 1},
    {-1, 24, 6, 1}, {1, 220, 8, 1}, {-1, 21, 1, 2}, {1, 228, 7, 2},
    {1, 25, 3, 2}, {1, 235, 3, 2}, {1, 28, 2, 3}, {1, 154, 2, 2},
    {1, 48, 1, 3}, {1, 169, 2, 2}, {1, 238, 2, 3}, {-1, 122, 7, 2},
    {-1, 13, 1, 2}, {1, 122, 6, 2}, {1, 13, 2, 3}, {-1, 3, 1, 2}, {1, 3, 1, 3},
    {1, 128, 1, 2}
  };
  
  for (size_t i = 0; i < sizeof(operations) / sizeof(operations[0]); ++i) {
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
