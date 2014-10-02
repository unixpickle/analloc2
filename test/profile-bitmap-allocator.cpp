#include <iostream>
#include <cstddef>
#include <analloc2>
#include "nanotime.hpp"

using namespace analloc;

template <typename T>
uint64_t ProfileBitmapWorst(size_t iterations, size_t bitCount);

int main() {
  const size_t iters = 100000;
  for (int i = 0; i < 5; ++i) {
    size_t bc = 0x1000 << i;
    std::cout << "BitmapAllocator<unsigned char> (" << bc << " bits)... "
      << std::flush << ProfileBitmapWorst<unsigned char>(iters >> i, bc)
      << std::endl;
    std::cout << "BitmapAllocator<unsigned short> (" << bc << " bits)... "
      << std::flush << ProfileBitmapWorst<unsigned short>(iters >> i, bc)
      << std::endl;
    std::cout << "BitmapAllocator<unsigned int> (" << bc << " bits)... "
      << std::flush << ProfileBitmapWorst<unsigned int>(iters >> i, bc)
      << std::endl;
    std::cout << "BitmapAllocator<unsigned long> (" << bc << " bits)... "
      << std::flush << ProfileBitmapWorst<unsigned long>(iters >> i, bc)
      << std::endl;
    std::cout << "BitmapAllocator<unsigned long long> (" << bc << " bits)... "
      << std::flush << ProfileBitmapWorst<unsigned long long>(iters >> i, bc)
      << std::endl;
  }
  return 0;
}

template <typename T>
uint64_t ProfileBitmapWorst(size_t iterations, size_t bitCount) {
  assert(bitCount % (sizeof(T) * 8) == 0);
  T * list = new T[bitCount / (sizeof(T) * 8)];
  BitmapAllocator<T, size_t, size_t> allocator(list, bitCount);
  
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
