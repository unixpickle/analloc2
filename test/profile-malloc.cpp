#include "microtime.hpp"
#include "../src/malloc.hpp"
#include "../src/tree/btree.hpp"
#include "../src/tree/bbtree.hpp"
#include <iostream>

using namespace ANAlloc;

template <class T>
uint64_t TimeMallocFreeSingles();

template <class T>
uint64_t TimeCreation();

int main() {
  std::cout << "Malloc::[Alloc/Free]() [BTree]... " << std::flush
    << TimeMallocFreeSingles<BTree>() << std::endl;
  std::cout << "Malloc::[Alloc/Free]() [BBTree]... " << std::flush
    << TimeMallocFreeSingles<BBTree>() << std::endl;
  std::cout << "Malloc::WrapRegion<BTree>()... " << std::flush
    << TimeCreation<BTree>() << std::endl;
  std::cout << "Malloc::WrapRegion<BBTree>()... " << std::flush
    << TimeCreation<BBTree>() << std::endl;
  return 0;
}

template <class T>
uint64_t TimeMallocFreeSingles() {
  uint8_t * buffer = new uint8_t[0x200000];
  uint8_t ** buffers = new uint8_t *[0x20000];
  Malloc & allocator = *Malloc::WrapRegion<T>(buffer, 0x200000, 6);
  uint64_t start = Microtime();
  
  for (int i = 0; i < 0x2000; i++) {
    buffers[i] = (uint8_t *)allocator.Alloc(0x40);
    assert(buffers[i] != NULL);
  }
  for (int i = 0; i < 0x2000; i++) {
    allocator.Free((uint8_t *)buffers[i]);
  }
  
  uint64_t total = Microtime() - start;
  delete[] buffer;
  delete[] buffers;
  return total;
}

template <class T>
uint64_t TimeCreation() {
  uint8_t * buffer = new uint8_t[0x200000];
  uint64_t start = Microtime();
  
  for (int i = 0; i < 0x1000; i++) {
    Malloc::WrapRegion<T>(buffer, 0x200000, 6);
  }
  
  uint64_t total = Microtime() - start;
  delete[] buffer;
  return total;
}
