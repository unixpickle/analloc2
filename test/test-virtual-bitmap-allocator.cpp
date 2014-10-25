#include "scoped-pass.hpp"
#include <analloc2/bitmap>

using namespace analloc;

void TestConstructed(size_t pageSize, size_t pageCount, size_t headerSize);

template <typename Unit>
void TestNormalPlace(size_t pageSize, size_t pageCount, size_t headerSize);

template <typename Unit>
void TestEdgePlace();

template <typename Unit>
void TestOptimalPlace();

template <typename Unit>
void ValidateOptimalPlace(size_t pageSize, size_t totalSize);

int main() {
  for (int i = 0; i < 5; ++i) {
    size_t size = 1UL << i;
    size_t headerSize = ansa::Max<size_t>(sizeof(size_t), size);
    const size_t pageCount = 0x100;
    TestConstructed(size, pageCount, headerSize);
    TestNormalPlace<unsigned char>(size, pageCount, headerSize);
    TestNormalPlace<unsigned short>(size, pageCount, headerSize);
    TestNormalPlace<unsigned int>(size, pageCount, headerSize);
    TestNormalPlace<unsigned long>(size, pageCount, headerSize);
    TestNormalPlace<unsigned long long>(size, pageCount, headerSize);
  }
  TestEdgePlace<unsigned char>();
  TestEdgePlace<unsigned short>();
  TestEdgePlace<unsigned int>();
  TestEdgePlace<unsigned long>();
  TestEdgePlace<unsigned long long>();
  TestOptimalPlace<unsigned char>();
  TestOptimalPlace<unsigned short>();
  TestOptimalPlace<unsigned int>();
  TestOptimalPlace<unsigned long>();
  TestOptimalPlace<unsigned long long>();
  return 0;
}

void TestConstructed(size_t pageSize, size_t pageCount, size_t headerSize) {
  ScopedPass pass("VirtualBitmapAllocator [constructed, ", pageSize, ", ",
                  pageCount, ", ", headerSize, "]");
  
  assert(headerSize >= pageSize);
  assert(ansa::IsAligned(headerSize, pageSize));
  
  uint8_t bitmap[ansa::RoundUpDiv<size_t>(pageCount, 8)];
  uint8_t zeroBitmap[sizeof(bitmap)];
  uint8_t data[pageSize * pageCount];
  
  ansa::Bzero(zeroBitmap, sizeof(zeroBitmap));
  
  uintptr_t start = (uintptr_t)data;
  
  VirtualBitmapAllocator<uint8_t> allocator(pageSize, start, bitmap,
                                            sizeof(data));
  assert(allocator.GetOffset() == start);
  assert(allocator.GetScale() == pageSize);
  assert(allocator.GetBitCount() == pageCount);
  assert(allocator.GetTotalSize() == pageSize * pageCount);
  
  uintptr_t addr;
  assert(ansa::Memcmp(bitmap, zeroBitmap, sizeof(bitmap)) == 0);
  
  // Test large allocations
  assert(!allocator.Alloc(addr, sizeof(data) - headerSize + 1));
  // Large allocations with Dealloc()
  assert(allocator.Alloc(addr, sizeof(data) - headerSize));
  assert(addr == start + headerSize);
  allocator.Dealloc(addr, sizeof(data) - headerSize);
  assert(ansa::Memcmp(bitmap, zeroBitmap, sizeof(bitmap)) == 0);
  // Large allocations with Free()
  assert(allocator.Alloc(addr, sizeof(data) - headerSize));
  assert(addr == start + headerSize);
  allocator.Free(addr);
  assert(ansa::Memcmp(bitmap, zeroBitmap, sizeof(bitmap)) == 0);
  
  // Test simple allocation with Dealloc()
  assert(allocator.Alloc(addr, 1));
  assert(addr == start + headerSize);
  assert(ansa::Memcmp(bitmap, zeroBitmap, sizeof(bitmap)) != 0);
  allocator.Dealloc(addr, 1);
  assert(ansa::Memcmp(bitmap, zeroBitmap, sizeof(bitmap)) == 0);
  // Test simple allocation with Free()
  assert(allocator.Alloc(addr, 1));
  assert(addr == start + headerSize);
  assert(ansa::Memcmp(bitmap, zeroBitmap, sizeof(bitmap)) != 0);
  allocator.Free(addr);
  assert(ansa::Memcmp(bitmap, zeroBitmap, sizeof(bitmap)) == 0);
  
  // Test re-allocation
  size_t firstSize = ansa::Align<size_t>(3, pageSize);
  size_t nextSize = firstSize * 2;
  assert(allocator.Alloc(addr, firstSize));
  assert(ansa::Memcmp(bitmap, zeroBitmap, sizeof(bitmap)) != 0);
  assert(addr == start + headerSize);
  ansa::Memcpy((void *)addr, "hey", 3);
  assert(allocator.Realloc(addr, nextSize));
  assert(ansa::Memcmp(bitmap, zeroBitmap, sizeof(bitmap)) != 0);
  assert(addr == start + (headerSize * 2) + firstSize);
  assert(ansa::Memcmp((void *)addr, "hey", 3) == 0);
  allocator.Free(addr);
  assert(ansa::Memcmp(bitmap, zeroBitmap, sizeof(bitmap)) == 0);
}

template <typename Unit>
void TestNormalPlace(size_t pageSize, size_t pageCount,
                     size_t headerSize) {
  ScopedPass pass("VirtualBitmapAllocator<", ansa::NumericInfo<Unit>::name,
                  ">::Place() [normal]");
  
  size_t align = ansa::Max(sizeof(Unit), pageSize);
  
  // Calculate a bunch of sizes
  size_t objectSize = ansa::Align(sizeof(VirtualBitmapAllocator<Unit>), align);
  size_t bitCount = ansa::Align<size_t>(pageCount, 8);
  size_t bitmapSize = ansa::Align<size_t>(bitCount / 8, align);
  size_t freeSize = pageSize * pageCount;
  size_t totalSize = objectSize + bitmapSize + freeSize;
  
  uint8_t buffer[totalSize];
  uintptr_t region = (uintptr_t)buffer;
  auto allocator = VirtualBitmapAllocator<Unit>::Place(region, totalSize,
                                                       pageSize);
  assert(allocator != nullptr);
  
  uintptr_t freeStart = region + objectSize + bitmapSize;
  assert(allocator->GetOffset() == freeStart);
  assert(allocator->GetScale() == pageSize);
  
  uintptr_t addr;
  
  // Test large allocations
  assert(!allocator->Alloc(addr, freeSize - headerSize + 1));
  // Large allocations with Dealloc()
  assert(allocator->Alloc(addr, freeSize - headerSize));
  assert(addr == freeStart + headerSize);
  allocator->Dealloc(addr, freeSize - headerSize);
  // Large allocations with Free()
  assert(allocator->Alloc(addr, freeSize - headerSize));
  assert(addr == freeStart + headerSize);
  allocator->Free(addr);
  // Ensure that the whole buffer is now free
  assert(allocator->Alloc(addr, freeSize - headerSize));
  allocator->Free(addr);
  
  // Test simple allocation with Dealloc()
  assert(allocator->Alloc(addr, 1));
  assert(addr == freeStart + headerSize);
  allocator->Dealloc(addr, 1);
  // Test simple allocation with Free()
  assert(allocator->Alloc(addr, 1));
  assert(addr == freeStart + headerSize);
  allocator->Free(addr);
  // Ensure that the whole buffer is now free
  assert(allocator->Alloc(addr, freeSize - headerSize));
  allocator->Free(addr);
  
  // Test re-allocation
  size_t firstSize = ansa::Align<size_t>(3, pageSize);
  size_t nextSize = firstSize * 2;
  assert(allocator->Alloc(addr, firstSize));
  assert(addr == freeStart + headerSize);
  ansa::Memcpy((void *)addr, "hey", 3);
  assert(allocator->Realloc(addr, nextSize));
  assert(addr == freeStart + (headerSize * 2) + firstSize);
  allocator->Free(addr);
  // Ensure that the whole buffer is now free
  assert(allocator->Alloc(addr, freeSize - headerSize));
  allocator->Free(addr);
}

template <typename Unit>
void TestEdgePlace() {
  typedef VirtualBitmapAllocator<Unit> TheAllocator;
  
  ScopedPass pass("VirtualBitmapAllocator<", ansa::NumericInfo<Unit>::name,
                  ">::Place() [edge]");
  
  uint8_t dummyRegion[sizeof(TheAllocator) * 8];
  uintptr_t region = (uintptr_t)dummyRegion;
  
  size_t minimumSize = ansa::Align<size_t>(sizeof(TheAllocator), sizeof(Unit));
  
  // Fundamental sanity checks; make sure these *do* work (although the
  // returned allocator will be empty, of course).
  int sizeLog = ansa::Log2Floor(minimumSize);
  for (int i = 0; i <= sizeLog; ++i) {
    size_t pageSize = (size_t)1 << sizeLog;
    size_t headerSize = ansa::Align(minimumSize, pageSize);
    auto allocator = TheAllocator::Place(region, headerSize, pageSize);
    assert(allocator != nullptr);
    assert(allocator->GetOffset() == region + headerSize);
    assert(allocator->GetScale() == pageSize);
    
    // Prove that it's empty
    uintptr_t addr;
    assert(!allocator->Alloc(addr, 1));
  }
  
  // Less than 1 page in the buffer; can't work.
  assert(!TheAllocator::Place(region, sizeof(TheAllocator),
                              ((size_t)1 << (sizeLog + 1))));
  
  // Try giving it every size less than sizeof(TheAllocator).
  for (size_t i = 0; i < sizeof(TheAllocator); ++i) {
    // Loop through the page sizes we could use for this [i].
    for (int align = 0; align <= ansa::BitScanRight(i); ++align) {
      assert(!TheAllocator::Place(region, i, (size_t)1 << align));
    }
  }
  // Try the smallest possible allocator that will actually give us memory
  // using a bunch of page sizes
  for (int ps = 0; ps < sizeLog + 1; ++ps) {
    size_t pageSize = (size_t)1 << ps;
    size_t align = ansa::Align(sizeof(Unit), pageSize);
    size_t objectSize = ansa::Align(sizeof(TheAllocator), align);
    
    // This size may change according to the info stored in a memory header
    size_t headerSize = ansa::Align(sizeof(size_t), pageSize);
    
    size_t dataSize = headerSize + pageSize;
    size_t bitmapSize = ansa::Align(ansa::RoundUpDiv<size_t>(dataSize, 8 *
        pageSize), align);
    
    size_t minSize = objectSize + bitmapSize + headerSize + pageSize;
    assert(ansa::IsAligned(minSize, pageSize));
    uint8_t tempRegion[minSize];
    for (size_t i = 0; i <= minSize; ++i) {
      auto allocator = TheAllocator::Place((uintptr_t)tempRegion, i, pageSize);
      if (i < objectSize) {
        assert(!allocator);
      } else if (i < minSize) {
        assert(allocator != nullptr);
        assert(allocator->GetScale() == pageSize);
        uintptr_t address;
        assert(!allocator->Alloc(address, 1));
      } else {
        assert(allocator != nullptr);
        assert(allocator->GetOffset() == (uintptr_t)tempRegion + objectSize + 
               bitmapSize);
        assert(allocator->GetScale() == pageSize);
        uintptr_t address;
        assert(allocator->Alloc(address, 1));
        assert(address == allocator->GetOffset() + headerSize);
      }
    }
  }
}

template <typename T>
void TestOptimalPlace() {
  ScopedPass pass("VirtualBitmapAllocator<", ansa::NumericInfo<T>::name,
                  ">::Place() [opt]");
  for (int psLog = 0; psLog < 8; ++psLog) {
    size_t pageSize = ((size_t)1 << psLog);
    size_t prefixSize = ansa::Align(sizeof(VirtualBitmapAllocator<T>), 
                                    ansa::Align(pageSize, sizeof(T)));
    size_t maxSize = prefixSize + pageSize * 0x20;
    for (size_t size = prefixSize; size < maxSize; ++size) {
      ValidateOptimalPlace<T>(pageSize, size);
    }
  }
}

template <typename Unit>
void ValidateOptimalPlace(size_t pageSize, size_t totalSize) {
  typedef VirtualBitmapAllocator<Unit> TheAllocator;
  
  uint8_t regionBuffer[totalSize];
  uintptr_t region = (uintptr_t)regionBuffer;
  
  size_t align = ansa::Align(pageSize, sizeof(Unit));
  size_t objectSize = ansa::Align(sizeof(TheAllocator), align);
  assert(objectSize <= totalSize);
  size_t bitmapSize = 0;
  size_t freeSpace = 0;
  
  // Brute force the optimal bitmap size. Obviously, the VirtualBitmapAllocator
  // doesn't brute force this. The reason *I* do is that I specifically want to
  // test whatever magical O(1) way the VirtualBitmapAllocator does it.
  for (size_t i = 0; i <= (totalSize - objectSize) * 8; i += align * 8) {
    size_t bytes = i / 8;
    size_t remainingSize = ((totalSize - objectSize - bytes) / pageSize) * 
                           pageSize;
    size_t representable = pageSize * i;
    size_t max = ansa::Min(remainingSize, representable);
    if (max > freeSpace) {
      freeSpace = max;
      bitmapSize = bytes;
    }
  }
  
  auto allocator = TheAllocator::Place(region, totalSize, pageSize);
  assert(allocator != nullptr);
  assert(allocator->GetScale() == pageSize);
  assert(allocator->GetOffset() == region + objectSize + bitmapSize);
  
  // make sure we can allocate the right size
  uintptr_t addr;
  size_t headerSize = ansa::Align(sizeof(size_t), pageSize);
  if (freeSpace > headerSize) {
    assert(!allocator->Alloc(addr, freeSpace - headerSize + 1));
    assert(allocator->Alloc(addr, freeSpace - headerSize));
  } else {
    assert(!allocator->Alloc(addr, 1));
  }
}
