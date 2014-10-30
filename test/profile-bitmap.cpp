#include <iostream>
#include <cstddef>
#include <analloc2/bitmap>
#include "nanotime.hpp"

using namespace analloc;

template <typename T>
void ProfileBoth();

template <typename T>
uint64_t ProfileAllocBest();

template <typename T>
uint64_t ProfileAllocLast(size_t iterations, size_t bitCount);

template <typename T>
uint64_t ProfileAllocFragmented(size_t iterations, size_t bitCount);

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
  std::cout << "Bitmap<" << ansa::NumericInfo<T>::name <<
    ">::Alloc() [best] ... " << std::flush << ProfileAllocBest<T>() <<
    std::endl;
  const size_t iters = 100000;
  for (int i = 0; i < 5; ++i) {
    size_t bc = 0x1000 << i;
    std::cout << "Bitmap<" << ansa::NumericInfo<T>::name <<
      ">::Alloc() [last, " << bc << "] ... " << std::flush <<
      ProfileAllocLast<T>(iters >> i, bc) << std::endl;
    std::cout << "Bitmap<" << ansa::NumericInfo<T>::name <<
      ">::Alloc() [fragmented, " << bc << "] ... " << std::flush <<
      ProfileAllocFragmented<T>(iters >> (i + 2), bc) << std::endl;
  }
}

template <typename T>
uint64_t ProfileAllocBest() {
  T bitmap[1];
  Bitmap<T, size_t> allocator(bitmap, 1);
  
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
uint64_t ProfileAllocLast(size_t iterations, size_t bitCount) {
  assert(bitCount % (sizeof(T) * 8) == 0);
  T * list = new T[bitCount / (sizeof(T) * 8)];
  Bitmap<T, size_t> allocator(list, bitCount);
  
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

template <typename T>
uint64_t ProfileAllocFragmented(size_t iterations, size_t bitCount) {
  assert(bitCount % (sizeof(T) * 8) == 0);
  T * list = new T[bitCount / (sizeof(T) * 8)];
  Bitmap<T, size_t> allocator(list, bitCount);
  
  uint64_t startTime = Nanotime();
  size_t result;
  for (size_t i = 0; i < (bitCount / 2) - 1; ++i) {
    allocator.Alloc(result, 2);
    assert(result == i * 2);
    allocator.Dealloc(result, 1);
  }
  for (size_t i = 0; i < iterations; ++i) {
    allocator.Alloc(result, 2);
    assert(result == bitCount - 2);
    allocator.Dealloc(result, 2);
  }
  uint64_t endTime = Nanotime();
  
  delete list;
  return (endTime - startTime) / iterations;
}
