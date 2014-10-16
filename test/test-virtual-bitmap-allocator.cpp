#include "scoped-pass.hpp"
#include <analloc2>

using namespace analloc;

void TestConstructed(size_t pageSize, size_t pageCount, size_t headerSize);

int main() {
  for (int i = 0; i < 5; ++i) {
    size_t size = 1UL << i;
    TestConstructed(size, 0x100, ansa::Max<size_t>(sizeof(size_t), size));
  }
  std::cerr << "TODO: test VirtualBitmapAllocator::Place() here" << std::endl;
  return 0;
}

void TestConstructed(size_t pageSize, size_t pageCount, size_t headerSize) {
  ScopedPass pass("VirtualBitmapAllocator [constructed, ", pageSize, ", ",
                  pageCount, ", ", headerSize, "]");
  
  assert(headerSize >= pageSize);
  assert(ansa::IsAligned(headerSize, pageSize));
  
  uint8_t bitmap[ansa::Align<size_t>(pageCount, 8) / 8];
  uint8_t zeroBitmap[sizeof(bitmap)];
  uint8_t data[pageSize * pageCount];
  
  ansa::Bzero(zeroBitmap, sizeof(zeroBitmap));
  
  uintptr_t start = (uintptr_t)data;
  
  VirtualBitmapAllocator<uint8_t> allocator(pageSize, start, bitmap,
                                            sizeof(data));
  assert(allocator.GetOffset() == start);
  assert(allocator.GetScale() == pageSize);
  
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
