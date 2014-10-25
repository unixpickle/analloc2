#include "nanotime.hpp"
#include "scoped-buffer.hpp"
#include <analloc2/bitmap>

using namespace analloc;

template <typename Unit>
void ProfileAll();

template <typename Unit>
uint64_t ProfileTrivialAlign();

template <typename Unit>
uint64_t ProfileFirstAlign();

template <typename Unit>
uint64_t ProfileLongAlign(size_t amountUsed);

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
  std::cout << "VirtualBitmapAligner<" << ansa::NumericInfo<Unit>::name <<
    ">::Align() [trivial] ... " << ProfileTrivialAlign<Unit>() << std::endl;
  std::cout << "VirtualBitmapAligner<" << ansa::NumericInfo<Unit>::name <<
    ">::Align() [first] ... " << ProfileFirstAlign<Unit>() << std::endl;
  for (int i = 0; i < 5; ++i) {
    size_t size = 0x40 << i;
    std::cout << "VirtualBitmapAligner<" << ansa::NumericInfo<Unit>::name <<
      ">::Align() [long, " << size << "] ... " <<
      ProfileLongAlign<Unit>(size) << std::endl;
  }
}

template <typename Unit>
uint64_t ProfileTrivialAlign() {
  ScopedBuffer<uint8_t *> data(0x20, 0x20);
  Unit bitmap[1];
  
  VirtualBitmapAligner<Unit> aligner(8, (uintptr_t)data, bitmap, 0x20);
  const size_t iterations = 100000;
  uintptr_t addr;
  uint64_t start = Nanotime();
  for (size_t i = 0; i < iterations; ++i) {
    bool res = aligner.Align(addr, 1, 0);
    assert(res);
    (void)res;
    aligner.Dealloc(addr, 0);
  }
  return (Nanotime() - start) / iterations;
}

template <typename Unit>
uint64_t ProfileFirstAlign() {
  ScopedBuffer<uint8_t *> data(0x20, 0x20);
  Unit bitmap[1];
  
  VirtualBitmapAligner<Unit> aligner(8, (uintptr_t)data, bitmap, 0x20);
  const size_t iterations = 100000;
  uintptr_t addr;
  uint64_t start = Nanotime();
  for (size_t i = 0; i < iterations; ++i) {
    bool res = aligner.Align(addr, 0x10, 0x10);
    assert(res);
    (void)res;
    aligner.Dealloc(addr, 0x10);
  }
  return (Nanotime() - start) / iterations;
}

template <typename Unit>
uint64_t ProfileLongAlign(size_t amountUsed) {
  assert(ansa::IsAligned<size_t>(amountUsed, 2));
  
  const size_t headerSize = sizeof(size_t);
  const size_t totalSize = amountUsed + (headerSize * 2) + 1;
  
  ScopedBuffer<uint8_t *> data(totalSize, 0x10);
  
  size_t unitCount = ansa::RoundUpDiv<size_t>(totalSize,
      ansa::NumericInfo<Unit>::bitCount);
  Unit bitmap[unitCount];
  
  VirtualBitmapAligner<Unit> aligner(1, (uintptr_t)data, bitmap, totalSize);
  uintptr_t addr;
  assert(aligner.Alloc(addr, amountUsed));
  
  const size_t iterations = 100000;
  uint64_t start = Nanotime();
  for (size_t i = 0; i < iterations; ++i) {
    bool res = aligner.Align(addr, 2, 1);
    assert(res);
    assert(addr == (uintptr_t)data + (headerSize * 2) + amountUsed);
    (void)res;
    aligner.Dealloc(addr, 1);
  }
  
  return (Nanotime() - start) / iterations;
}
