#include "scoped-pass.hpp"
#include <analloc2>

using namespace analloc;

void TestScaled();
void TestOffset();
void TestScaledOffset();

int main() {
  TestScaled();
  TestOffset();
  TestScaledOffset();
  return 0;
}

void TestScaled() {
  ScopedPass pass("TransformedBitmapAllocator [scaled]");
  
  // The bitmap will contain some extra bits so that we can be sure the
  // allocator doesn't overrun any bytes.
  uint32_t bitmap = 0xffffffff;
  
  // Create a bitmap allocator which is scaled by a factor of 3 and contains
  // 16 bits. Range of addresses: [0, 0x30)
  TransformedBitmapAllocator<uint32_t, uint16_t> allocator(3, 0, &bitmap, 16);
  assert(bitmap == 0xffff0000);
  
  uint16_t address;
  
  assert(!allocator.Alloc(address, 49));
  
  // Size <= 3
  assert(allocator.Alloc(address, 1));
  assert(address == 0);
  assert(allocator.Alloc(address, 2));
  assert(address == 3);
  assert(allocator.Alloc(address, 3));
  assert(address == 6);
  allocator.Dealloc(0, 1);
  assert(allocator.Alloc(address, 2));
  assert(address == 0);
  allocator.Dealloc(3, 2);
  assert(allocator.Alloc(address, 3));
  assert(address == 3);
  allocator.Dealloc(0, 9);
  assert(bitmap == 0xffff0000);
  
  // Size <= 6
  assert(allocator.Alloc(address, 4));
  assert(address == 0);
  assert(allocator.Alloc(address, 5));
  assert(address == 6);
  assert(allocator.Alloc(address, 6));
  assert(address == 12);
  allocator.Dealloc(6, 5);
  assert(allocator.Alloc(address, 6));
  assert(address == 6);
  allocator.Dealloc(0, 30);
  assert(bitmap == 0xffff0000);
  
  // Filling the thing up
  assert(allocator.Alloc(address, 46));
  assert(address == 0);
  assert(!allocator.Alloc(address, 1));
  allocator.Dealloc(0, 46);
  assert(bitmap == 0xffff0000);
  assert(allocator.Alloc(address, 48));
  assert(address == 0);
  allocator.Dealloc(0, 48);
  assert(bitmap == 0xffff0000);
}

void TestOffset() {
  ScopedPass pass("TransformedBitmapAllocator [offset]");
  
  // The bitmap will contain some extra bits so that we can be sure the
  // allocator doesn't overrun any bytes.
  uint32_t bitmap = 0xffffffff;
  
  // Create a bitmap allocator which is translated by 0x103 units and contains
  // 16 bits. Range of addresses: [0x103, 0x113)
  TransformedBitmapAllocator<uint32_t, uint16_t, uint8_t>
      allocator(1, 0x103, &bitmap, 16);
  assert(bitmap == 0xffff0000);
  
  uint16_t address;
  
  assert(!allocator.Alloc(address, 0x11));
  
  // Filling allocation
  assert(allocator.Alloc(address, 0x10));
  assert(address == 0x103);
  assert(!allocator.Alloc(address, 1));
  assert(bitmap == 0xffffffff);
  allocator.Dealloc(0x103, 0x10);
  assert(bitmap == 0xffff0000);
  
  // Simple allocations
  assert(allocator.Alloc(address, 0xf));
  assert(address == 0x103);
  assert(allocator.Alloc(address, 1));
  assert(address == 0x112);
  assert(!allocator.Alloc(address, 1));
  assert(bitmap == 0xffffffff);
  allocator.Dealloc(0x103, 0x10);
  assert(bitmap == 0xffff0000);
}

void TestScaledOffset() {
  ScopedPass pass("TransformedBitmapAllocator [both]");
  
  // The bitmap will contain some extra bits so that we can be sure the
  // allocator doesn't overrun any bytes.
  uint32_t bitmap = 0xffffffff;
  
  // Create a bitmap allocator which is scaled by 3 units, translated by 0x300
  // units, and contains 16 bits. Range of addresses: [0x300, 0x330)
  TransformedBitmapAllocator<uint32_t, uint16_t, uint8_t>
      allocator(3, 0x300, &bitmap, 16);
  assert(bitmap == 0xffff0000);
  
  uint16_t address;
  const uint32_t full = 0xffffffff;
  const uint32_t empty = 0xffff0000;
  
  assert(!allocator.Alloc(address, 0x31));
  
  // Filling allocation
  assert(allocator.Alloc(address, 0x30));
  assert(address == 0x300);
  assert(!allocator.Alloc(address, 1));
  assert(bitmap == full);
  allocator.Dealloc(0x300, 0x30);
  assert(bitmap == empty);
  
  // Two small allocations + large allocation
  assert(allocator.Alloc(address, 1));
  assert(address == 0x300);
  assert(allocator.Alloc(address, 1));
  assert(address == 0x303);
  assert(!allocator.Alloc(address, 43));
  assert(allocator.Alloc(address, 42));
  assert(address == 0x306);
  assert(bitmap == full);
  allocator.Dealloc(0x300, 0x30);
  assert(bitmap == empty);
  
  // Fragmented allocations
  assert(allocator.Alloc(address, 3));
  assert(address == 0x300);
  assert(allocator.Alloc(address, 3));
  assert(address == 0x303);
  assert(allocator.Alloc(address, 3));
  assert(address == 0x306);
  allocator.Dealloc(0x303, 3);
  assert(allocator.Alloc(address, 1));
  assert(address == 0x303);
  allocator.Dealloc(0x300, 9);
  assert(bitmap == empty);
}
