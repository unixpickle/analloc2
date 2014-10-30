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

template <typename T>
uint64_t ProfileAllocPartiallyFragmented(size_t iterations, size_t bitCount);

template <typename T>
uint64_t ProfileOffsetAlignLast(size_t iterations, size_t bitCount);

template <typename T>
uint64_t ProfileOffsetAlignFragmented(size_t iterations, size_t bitCount);

int main() {
  ProfileBoth<unsigned long long>();
  ProfileBoth<unsigned long>();
  ProfileBoth<unsigned int>();
  ProfileBoth<unsigned short>();
  ProfileBoth<unsigned char>();
  return 0;
}

template <typename T>
void ProfileBoth() {
  std::cout << "Bitmap<" << ansa::NumericInfo<T>::name <<
    ">::Alloc() [best] ... " << std::flush << ProfileAllocBest<T>() <<
    std::endl;
  const size_t iters = 0x20000;
  for (int i = 0; i < 5; ++i) {
    size_t bc = 0x1000 << i;
    std::cout << "Bitmap<" << ansa::NumericInfo<T>::name <<
      ">::Alloc() [last, " << bc << "] ... " << std::flush <<
      ProfileAllocLast<T>(iters >> i, bc) << std::endl;
    std::cout << "Bitmap<" << ansa::NumericInfo<T>::name <<
      ">::Alloc() [fragmented, " << bc << "] ... " << std::flush <<
      ProfileAllocFragmented<T>(iters >> (i + 2), bc) << std::endl;
    std::cout << "Bitmap<" << ansa::NumericInfo<T>::name <<
      ">::Alloc() [partial, " << bc << "] ... " << std::flush <<
      ProfileAllocPartiallyFragmented<T>(iters >> (i + 2), bc) << std::endl;
    std::cout << "Bitmap<" << ansa::NumericInfo<T>::name <<
      ">::OffsetAlign() [last, " << bc << "] ... " << std::flush <<
      ProfileOffsetAlignLast<T>(iters >> (i + 2), bc) << std::endl;
    std::cout << "Bitmap<" << ansa::NumericInfo<T>::name <<
      ">::OffsetAlign() [fragmented, " << bc << "] ... " << std::flush <<
      ProfileOffsetAlignFragmented<T>(iters >> (i + 2), bc) << std::endl;
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
  
  size_t result;
  assert(allocator.Alloc(result, bitCount - 1));
  assert(result == 0);
  uint64_t startTime = Nanotime();
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
  
  size_t result;
  for (size_t i = 0; i < (bitCount / 2) - 1; ++i) {
    allocator.Alloc(result, 2);
    assert(result == i * 2);
    allocator.Dealloc(result, 1);
  }
  uint64_t startTime = Nanotime();
  for (size_t i = 0; i < iterations; ++i) {
    allocator.Alloc(result, 2);
    assert(result == bitCount - 2);
    allocator.Dealloc(result, 2);
  }
  uint64_t endTime = Nanotime();
  
  delete list;
  return (endTime - startTime) / iterations;
}

template <typename T>
uint64_t ProfileAllocPartiallyFragmented(size_t iterations, size_t bitCount) {
  assert(bitCount % (sizeof(T) * 8) == 0);
  T * list = new T[bitCount / (sizeof(T) * 8)];
  Bitmap<T, size_t> allocator(list, bitCount);
  
  size_t result;
  for (size_t i = 0; i < (bitCount / 8) - 1; ++i) {
    allocator.Alloc(result, 8);
    assert(result == i * 8);
    allocator.Dealloc(result, 1);
  }
  uint64_t startTime = Nanotime();
  for (size_t i = 0; i < iterations; ++i) {
    allocator.Alloc(result, 8);
    assert(result == bitCount - 8);
    allocator.Dealloc(result, 8);
  }
  uint64_t endTime = Nanotime();
  
  delete list;
  return (endTime - startTime) / iterations;
}

template <typename T>
uint64_t ProfileOffsetAlignLast(size_t iterations, size_t bitCount) {
  assert(bitCount % (sizeof(T) * 8) == 0);
  T * list = new T[bitCount / (sizeof(T) * 8)];
  Bitmap<T, size_t> allocator(list, bitCount);
  
  size_t result;
  assert(allocator.Alloc(result, bitCount - 1));
  assert(result == 0);
  uint64_t startTime = Nanotime();
  for (size_t i = 0; i < iterations; ++i) {
    allocator.OffsetAlign(result, 2, 1, 1);
    assert(result == bitCount - 1);
    allocator.Dealloc(result, 1);
  }
  uint64_t endTime = Nanotime();
  
  delete list;
  return (endTime - startTime) / iterations;
}

template <typename T>
uint64_t ProfileOffsetAlignFragmented(size_t iterations, size_t bitCount) {
  assert(bitCount % (sizeof(T) * 8) == 0);
  T * list = new T[bitCount / (sizeof(T) * 8)];
  Bitmap<T, size_t> allocator(list, bitCount);
  
  size_t result;
  for (size_t i = 0; i < (bitCount / 2) - 1; ++i) {
    allocator.Alloc(result, 2);
    assert(result == i * 2);
    allocator.Dealloc(result, 1);
  }
  uint64_t startTime = Nanotime();
  for (size_t i = 0; i < iterations; ++i) {
    allocator.OffsetAlign(result, 2, 1, 1);
    assert(result == bitCount - 1);
    allocator.Dealloc(result, 1);
  }
  uint64_t endTime = Nanotime();
  
  delete list;
  return (endTime - startTime) / iterations;
}
