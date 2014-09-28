#include "scoped-pass.hpp"
#include <analloc2>
#include <ansa/numeric-info>
#include <cstdint>

using namespace ansa;
using namespace analloc;

template <typename T>
void TestStepAllocation();

template <typename T>
void TestFragmentedAllocation();

int main() {
  TestStepAllocation<unsigned char>();
  TestStepAllocation<unsigned short>();
  TestStepAllocation<unsigned int>();
  TestStepAllocation<unsigned long>();
  TestStepAllocation<unsigned long long>();
  TestFragmentedAllocation<unsigned char>();
  TestFragmentedAllocation<unsigned short>();
  TestFragmentedAllocation<unsigned int>();
  TestFragmentedAllocation<unsigned long>();
  TestFragmentedAllocation<unsigned long long>();
  return 0;
}

template <typename T>
void TestStepAllocation() {
  ScopedPass pass("BitmapAllocator<", NumericInfo<T>::name,
                  ", uint8_t> [step]");
  // a 256-bit bitmap allocator
  uint8_t addr = 0;
  T cells[256 / (sizeof(T) * 8)];
  BitmapAllocator<T, uint8_t> allocator(0, cells, 0x100);
  for (int i = 0; i < 256; ++i) {
    assert(allocator.Alloc(addr, 1));
    assert(addr == (uint8_t)i);
  }
  assert(!allocator.Alloc(addr, 1));
  for (int i = 0; i < 256; ++i) {
    allocator.Dealloc((uint8_t)i, 1);
  }
  for (int i = 0; i < 128; ++i) {
    uint8_t addr = 0;
    assert(allocator.Alloc(addr, 2));
    assert(addr == (uint8_t)(i * 2));
  }
  assert(!allocator.Alloc(addr, 2));
  assert(!allocator.Alloc(addr, 1));
  for (int i = 0; i < 128; ++i) {
    allocator.Dealloc((uint8_t)(i * 2), 2);
  }
}

template <typename T>
void TestFragmentedAllocation() {
  ScopedPass pass("BitmapAllocator<", NumericInfo<T>::name,
                  ", uint32_t, uint16_t>");
  uint32_t addr;
  T cells[256 / (sizeof(T) * 8)];
  
  // Offset the addresses by 65537 bits just for kicks
  BitmapAllocator<T, uint32_t, uint16_t> allocator(0x10001, cells, 0x100);
  
  // Allocate all of the memory, free it again, and allocate it again, just to
  // verify that huge allocations work with weird address offsets
  assert(allocator.Alloc(addr, 0x100));
  assert(addr == 0x10001);
  assert(!allocator.Alloc(addr, 1));
  allocator.Dealloc(0x10001, 0x100);
  assert(allocator.Alloc(addr, 0x100));
  assert(addr == 0x10001);
  assert(!allocator.Alloc(addr, 1));
  
  // Attempt to free a single space in the middle of the buffer, and then
  // allocate it again. This is not strictly allowed through the Allocator
  // interface, but it works for BitmapAllocator.
  allocator.Dealloc(0x10081, 1);
  assert(allocator.Alloc(addr, 1));
  assert(addr == 0x10081);
  assert(!allocator.Alloc(addr, 1));
  
  // Free a single space near the beginning of the buffer, then a larger space
  // near the middle of the buffer.
  allocator.Dealloc(0x10011, 1);
  allocator.Dealloc(0x10021, 2);
  
  // Verify that the allocator knows that a larger buffer needs more room.
  assert(allocator.Alloc(addr, 2));
  assert(addr == 0x10021);
  allocator.Dealloc(0x10021, 2);
  
  // Verify that the allocator puts small buffers at the first available spot.
  assert(allocator.Alloc(addr, 1));
  assert(addr = 0x10011);
  assert(allocator.Alloc(addr, 1));
  assert(addr == 0x10021);
  assert(allocator.Alloc(addr, 1));
  assert(addr == 0x10022);
  assert(!allocator.Alloc(addr, 1));
}
