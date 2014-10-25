#include <iostream>
#include <cstddef>
#include <analloc2/bitmap>
#include "nanotime.hpp"

using namespace analloc;

template <typename T>
void ProfileBoth();

template <typename T>
uint64_t ProfileBitmapBest();

template <typename T>
uint64_t ProfileBitmapWorst(size_t iterations, size_t bitCount);

int main() {
  ProfileBoth<unsigned char>();
  ProfileBoth<unsigned short>();
  ProfileBoth<unsigned int>();
  ProfileBoth<unsigned long>();
  ProfileBoth<unsigned long long>();
  return 0;
}

template <typename T>
void ProfileBoth() {
  std::cout << "BitmapAllocator<" << ansa::NumericInfo<T>::name <<
    "> [best] ... " << std::flush << ProfileBitmapBest<T>() << std::endl;
  const size_t iters = 100000;
  for (int i = 0; i < 5; ++i) {
    size_t bc = 0x1000 << i;
    std::cout << "BitmapAllocator<" << ansa::NumericInfo<T>::name <<
      "> [worst, " << bc << "] ... " << std::flush <<
      ProfileBitmapWorst<T>(iters >> i, bc) << std::endl;
  }
}

template <typename T>
uint64_t ProfileBitmapBest() {
  T bitmap[1];
  BitmapAllocator<T, size_t> allocator(bitmap, 1);
  
  uint64_t start = Nanotime();
  const size_t iterations = 10000000;
  size_t result;
  for (size_t i = 0; i < iterations; ++i) {
    allocator.Alloc(result, 1);
    assert(result == 0);
    allocator.Dealloc(result, 1);
  }
  return (Nanotime() - start) / iterations;
}

template <typename T>
uint64_t ProfileBitmapWorst(size_t iterations, size_t bitCount) {
  assert(bitCount % (sizeof(T) * 8) == 0);
  T * list = new T[bitCount / (sizeof(T) * 8)];
  BitmapAllocator<T, size_t> allocator(list, bitCount);
  
  uint64_t startTime = Nanotime();
  size_t result;
  assert(allocator.Alloc(result, bitCount - 1));
  assert(result == 0);
  for (size_t i = 0; i < iterations; ++i) {
    allocator.Alloc(result, 1);
    assert(result == bitCount - 1);
    allocator.Dealloc(result, 1);
  }
  uint64_t endTime = Nanotime();
  
  delete list;
  return (endTime - startTime) / iterations;
}
