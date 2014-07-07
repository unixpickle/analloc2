#include "../src/malloc.hpp"
#include "../src/bbtree.hpp"
#include "../src/btree.hpp"
#include "microtime.hpp"
#include <iostream>

using namespace ANAlloc;

template <class T>
uint64_t TimeMallocFreeSingles();

template <class T>
uint64_t TimeCreation();

int main() {
  std::cout << "Malloc<BTree>::[Alloc/Free]()... " << std::flush
    << TimeMallocFreeSingles<BTree>() << std::endl;
  std::cout << "Malloc<BBTree>::[Alloc/Free]()... " << std::flush
    << TimeMallocFreeSingles<BBTree>() << std::endl;
  std::cout << "Malloc<BTree>::Malloc()... " << std::flush
    << TimeCreation<BTree>() << std::endl;
  std::cout << "Malloc<BBTree>::Malloc()... " << std::flush
    << TimeCreation<BBTree>() << std::endl;
  return 0;
}

template <class T>
uint64_t TimeMallocFreeSingles() {
  uint8_t * buffer = new uint8_t[0x200000];
  uint8_t ** buffers = new uint8_t *[0x20000];
  Malloc<T> allocator(buffer, 0x40, 0, 0x200000);
  uint64_t start = Microtime();
  
  for (int i = 0; i < 0x2000; i++) {
    buffers[i] = (uint8_t *)allocator.AllocBuf(0x40);
    assert(buffers[i] != NULL);
  }
  for (int i = 0; i < 0x2000; i++) {
    allocator.FreeBuf((uint8_t *)buffers[i]);
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
    Malloc<T> allocator(buffer, 0x40, 0, 0x200000);
  }
  
  uint64_t total = Microtime() - start;
  delete[] buffer;
  return total;
}
