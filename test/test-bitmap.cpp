#include "scoped-pass.hpp"
#include <analloc2/bitmap>
#include <ansa/numeric-info>
#include <cstdint>

using namespace ansa;
using namespace analloc;

template <typename T>
void TestStepAllocation();

template <typename T>
void TestFragmentedAllocation();

void TestLargeAlignment();
void TestUnitAlignment();
void TestOverflowAlignment();
void TestLastUnitAlignment();
void TestSimpleOffsetAlignment();
void TestMultitypeOffsetAlignment();

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
  
  TestLargeAlignment();
  TestUnitAlignment();
  TestOverflowAlignment();
  TestLastUnitAlignment();
  TestSimpleOffsetAlignment();
  TestMultitypeOffsetAlignment();
  
  return 0;
}

template <typename T>
void TestStepAllocation() {
  ScopedPass pass("Bitmap<", NumericInfo<T>::name,
                  ", uint8_t>::Alloc() [step]");
  // a 256-bit bitmap allocator
  uint16_t addr = 0;
  T cells[256 / (sizeof(T) * 8)];
  Bitmap<T, uint16_t> allocator(cells, 0x100);
  for (int i = 0; i < 256; ++i) {
    assert(allocator.Alloc(addr, 1));
    assert(addr == (uint16_t)i);
  }
  assert(!allocator.Alloc(addr, 1));
  for (int i = 0; i < 256; ++i) {
    allocator.Dealloc((uint16_t)i, 1);
  }
  for (int i = 0; i < 128; ++i) {
    assert(allocator.Alloc(addr, 2));
    assert(addr == (uint16_t)(i * 2));
  }
  assert(!allocator.Alloc(addr, 2));
  assert(!allocator.Alloc(addr, 1));
  for (int i = 0; i < 128; ++i) {
    allocator.Dealloc((uint16_t)(i * 2), 2);
  }
}

template <typename T>
void TestFragmentedAllocation() {
  ScopedPass pass("Bitmap<", NumericInfo<T>::name,
                  ", uint32_t, uint16_t>::Alloc()");
  uint16_t addr;
  T cells[256 / (sizeof(T) * 8)];
  
  Bitmap<T, uint16_t> allocator(cells, 0x100);
  
  // Allocate all of the memory, free it again, and allocate it again, just to
  // verify that huge allocations work with weird address offsets
  assert(allocator.Alloc(addr, 0x100));
  assert(addr == 0);
  assert(!allocator.Alloc(addr, 1));
  allocator.Dealloc(0, 0x100);
  assert(allocator.Alloc(addr, 0x100));
  assert(addr == 0);
  assert(!allocator.Alloc(addr, 1));
  
  // Attempt to free a single space in the middle of the buffer, and then
  // allocate it again. This is not strictly allowed through the Allocator
  // interface, but it works for Bitmap.
  allocator.Dealloc(0x80, 1);
  assert(allocator.Alloc(addr, 1));
  assert(addr == 0x80);
  assert(!allocator.Alloc(addr, 1));
  
  // Free a single space near the beginning of the buffer, then a larger space
  // near the middle of the buffer.
  allocator.Dealloc(0x10, 1);
  allocator.Dealloc(0x20, 2);
  
  // Verify that the allocator knows that a larger buffer needs more room.
  assert(allocator.Alloc(addr, 2));
  assert(addr == 0x20);
  allocator.Dealloc(0x20, 2);
  
  // Verify that the allocator puts small buffers at the first available spot.
  assert(allocator.Alloc(addr, 1));
  assert(addr = 0x10);
  assert(allocator.Alloc(addr, 1));
  assert(addr == 0x20);
  assert(allocator.Alloc(addr, 1));
  assert(addr == 0x21);
  assert(!allocator.Alloc(addr, 1));
}

void TestLargeAlignment() {
  ScopedPass pass("Bitmap::Alloc() [large]");
  
  uint16_t address;
  uint16_t units[8];
  Bitmap<uint16_t, uint16_t, uint8_t> aligner(units, 0x80);
  
  // Alignment by a number which exceeds SizeType
  assert(aligner.Align(address, 0x1000, 1));
  assert(address == 0);
  assert(!aligner.Align(address, 0x1000, 1));
  assert(!aligner.Align(address, 0x80, 1));
  aligner.Dealloc(0, 1);
  assert(!aligner.Align(address, 0x1000, 0x81));
  assert(aligner.Align(address, 0x1000, 0x80));
  assert(address == 0);
}

void TestUnitAlignment() {
  ScopedPass pass("Bitmap::Align() [unit]");
  
  uint16_t address;
  uint16_t units[8];
  Bitmap<uint16_t, uint16_t, uint8_t> aligner(units, 0x80);
  
  for (uint16_t align = 0; align < 2; ++align) {
    // Make sure that, indeed, alignments <= 1 result in normal allocations
    assert(aligner.Align(address, align, 1));
    assert(address == 0);
    assert(aligner.Align(address, align, 1));
    assert(address == 1);
    assert(aligner.Align(address, align, 4));
    assert(address == 2);
    aligner.Dealloc(1, 1);
    assert(aligner.Align(address, align, 2));
    assert(address == 6);
    assert(aligner.Align(address, align, 1));
    assert(address == 1);
    aligner.Dealloc(0, 8);
  }
}

void TestOverflowAlignment() {
  ScopedPass pass("Bitmap::Align() [overflow]");
  
  uint16_t address;
  uint16_t units[8];
  Bitmap<uint16_t, uint16_t, uint8_t> aligner(units, 0x80);
  
  assert(aligner.Align(address, 1, 0x40));
  assert(address == 0);
  assert(aligner.Align(address, 0x20, 0x20));
  assert(address == 0x40);
  aligner.Dealloc(0x50, 0x10);
  assert(!aligner.Align(address, 0x40, 0x10));
  assert(aligner.Align(address, 0x10, 0x10));
  assert(address == 0x50);
  assert(!aligner.Align(address, 0x80, 0x20));
  assert(aligner.Align(address, 0x20, 0x20));
  assert(address == 0x60);
  assert(!aligner.Align(address, 1, 1));
}

void TestLastUnitAlignment() {
  ScopedPass pass("Bitmap::Align() [last unit]");
  
  uint16_t address;
  uint16_t units[0x10];
  Bitmap<uint16_t, uint16_t, uint8_t> aligner(units, 0xff);
  
  assert(aligner.Align(address, 0x80, 1));
  assert(address == 0);
  assert(aligner.Align(address, 0x80, 1));
  assert(address == 0x80);
  assert(!aligner.Align(address, 0x80, 1));
}

void TestSimpleOffsetAlignment() {
  ScopedPass pass("Bitmap::Align() [simple offset]");
  
  uint8_t address;
  uint8_t units[2];
  Bitmap<uint8_t, uint8_t> aligner(units, 0x10);
  
  assert(!aligner.OffsetAlign(address, 0x10, 1, 2));
  assert(aligner.OffsetAlign(address, 0x10, 1, 1));
  assert(address == 0xf);
  assert(!aligner.OffsetAlign(address, 0x10, 1, 1));
  
  assert(!aligner.OffsetAlign(address, 0x10, 5, 5));
  assert(aligner.OffsetAlign(address, 0x10, 5, 1));
  assert(address == 0xb);
  assert(!aligner.OffsetAlign(address, 0x10, 5, 1));
  
  assert(aligner.OffsetAlign(address, 4, 1, 1));
  assert(address == 3);
  assert(aligner.OffsetAlign(address, 4, 1, 2));
  assert(address == 7);
}

void TestMultitypeOffsetAlignment() {
  ScopedPass pass("Bitmap::Align() [multitype offset]");
  
  uint16_t address;
  uint8_t units[2];
  Bitmap<uint8_t, uint16_t, uint8_t> aligner(units, 0x10);
  
  assert(!aligner.OffsetAlign(address, 0x200, 1, 1));
  assert(!aligner.OffsetAlign(address, 0x200, 0x201, 1));
  assert(aligner.OffsetAlign(address, 0x200, 0x200, 1));
  assert(address == 0);
  assert(!aligner.OffsetAlign(address, 0x200, 0x200, 1));
  assert(aligner.OffsetAlign(address, 0x200, 0x1f8, 1));
  assert(address == 0x8);
  assert(!aligner.OffsetAlign(address, 0x200, 0x1f8, 1));

  // Wrap-around alignments
  assert(aligner.OffsetAlign(address, 0x100, 0xfffe, 1));
  assert(address == 2);
  assert(!aligner.OffsetAlign(address, 0x100, 0xffff, 2));
  assert(aligner.OffsetAlign(address, 0x100, 0xffff, 1));
  assert(address == 1);
}
