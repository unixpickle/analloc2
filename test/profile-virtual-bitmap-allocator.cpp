#include "nanotime.hpp"
#include "scoped-buffer.hpp"
#include <analloc2/bitmap>

using namespace analloc;

template <typename Unit>
void ProfileAll();

template <typename Unit>
uint64_t ProfileTrivialAlloc();

template <typename Unit>
uint64_t ProfileLongAlloc(size_t amountUsed);

int main() {
  ProfileAll<unsigned char>();
  ProfileAll<unsigned short>();
  ProfileAll<unsigned int>();
  ProfileAll<unsigned long>();
  ProfileAll<unsigned long long>();
  return 0;
}

template <typename Unit>
void ProfileAll() {
  std::cout << "VirtualBitmapAllocator<" << ansa::NumericInfo<Unit>::name <<
    ">::Alloc() [trivial] ... " << std::flush << ProfileTrivialAlloc<Unit>() <<
    std::endl;
  for (int i = 0; i < 5; ++i) {
    size_t size = 0x1000 << i;
    std::cout << "VirtualBitmapAllocator<" << ansa::NumericInfo<Unit>::name <<
      ">::Alloc() [long, " << size << "] ... " << std::flush <<
      ProfileLongAlloc<Unit>(size) << std::endl;
  }
}

template <typename Unit>
uint64_t ProfileTrivialAlloc() {
  ScopedBuffer data(0x20);
  Unit bitmap[1];
  
  VirtualBitmapAllocator<Unit> allocator(8, (uintptr_t)data, bitmap, 0x20);
  const size_t iterations = 5000000;
  uintptr_t addr;
  uint64_t start = Nanotime();
  for (size_t i = 0; i < iterations; ++i) {
    bool res = allocator.Alloc(addr, 0);
    assert(res);
    (void)res;
    allocator.Free(addr);
  }
  return (Nanotime() - start) / iterations;
}

template <typename Unit>
uint64_t ProfileLongAlloc(size_t amountUsed) {
  assert(ansa::IsAligned<size_t>(amountUsed, 2));
  
  const size_t headerSize = sizeof(size_t);
  const size_t totalSize = amountUsed + (headerSize * 2) + 1;
  
  ScopedBuffer data(totalSize);
  
  size_t unitCount = ansa::RoundUpDiv<size_t>(totalSize,
      ansa::NumericInfo<Unit>::bitCount);
  Unit bitmap[unitCount];
  
  VirtualBitmapAllocator<Unit> allocator(1, (uintptr_t)data, bitmap,
                                         totalSize);
  uintptr_t addr;
  bool res = allocator.Alloc(addr, amountUsed);
  assert(res);
  (void)res;
  
  const size_t iterations = 0x3000000 / (amountUsed);
  uint64_t start = Nanotime();
  for (size_t i = 0; i < iterations; ++i) {
    bool res = allocator.Alloc(addr, 1);
    assert(res);
    assert(addr == (uintptr_t)data + (headerSize * 2) + amountUsed);
    (void)res;
    allocator.Free(addr);
  }
  
  return (Nanotime() - start) / iterations;
}
