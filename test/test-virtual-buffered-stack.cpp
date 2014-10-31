#include "scoped-pass.hpp"
#include "posix-virtual-aligner.hpp"
#include <analloc2/buffered-stack>

using namespace analloc;

PosixVirtualAligner aligner;
bool hasOverflowed = false;
uintptr_t overflowAddress;
size_t overflowSize;

void TestInitialization();
void TestApplyBufferMinimum();
void TestApplyAllocDealloc();
void TestReallocFree();
void TestOverflows();

template <typename T>
void FailureOverflowHandler(T *, uintptr_t, size_t);

template <typename T>
void ExpectedOverflowHandler(T *, uintptr_t, size_t);

int main() {
  TestInitialization();
  assert(aligner.GetAllocCount() == 0);
  TestApplyBufferMinimum();
  assert(aligner.GetAllocCount() == 0);
  TestApplyAllocDealloc();
  assert(aligner.GetAllocCount() == 0);
  TestReallocFree();
  assert(aligner.GetAllocCount() == 0);
  TestOverflows();
  assert(aligner.GetAllocCount() == 0);
  return 0;
}

void TestInitialization() {
  ScopedPass pass("[Virtual]BufferedStack()");
  
  BufferedStack<0x10, uintptr_t, size_t> stack(aligner, 1, 0xf, 0x20,
      FailureOverflowHandler);
  assert(stack.GetSoftMinimum() == 1);
  assert(stack.GetSoftMaximum() == 0xf);
  assert(stack.GetObjectSize() == 0x20);
  assert(stack.GetCount() == 0);
  
  VirtualBufferedStack<0x10> stack1(aligner, 1, 0xf, 0x20, 
      FailureOverflowHandler);
  assert(stack1.GetSoftMinimum() == 1);
  assert(stack1.GetSoftMaximum() == 0xf);
  assert(stack1.GetObjectSize() == 0x20);
  assert(stack1.GetCount() == 0);
}

void TestApplyBufferMinimum() {
  ScopedPass pass("VirtualBufferedStack::ApplyBuffer() [minimum]");
  
  // Stack with minimum = 0.
  VirtualBufferedStack<0x10> emptyStack(aligner, 0, 0xf, 0x20, 
      FailureOverflowHandler);
  assert(emptyStack.GetCount() == 0);
  assert(aligner.GetAllocCount() == 0);
  assert(emptyStack.ApplyBuffer());
  assert(emptyStack.GetCount() == 0);
  assert(aligner.GetAllocCount() == 0);
  
  // Stack with minimum = 2.
  VirtualBufferedStack<0x10> stack(aligner, 2, 0xf, 0x20, 
      FailureOverflowHandler);
  assert(stack.GetCount() == 0);
  assert(aligner.GetAllocCount() == 0);
  assert(stack.ApplyBuffer());
  assert(stack.GetCount() == 2);
  assert(aligner.GetAllocCount() == 2);
}

void TestApplyAllocDealloc() {
  ScopedPass pass("VirtualBufferedStack::[ApplyBuffer/Alloc/Dealloc]()");
  uintptr_t addr1, addr2;
  
  VirtualBufferedStack<3> stack(aligner, 1, 2, 0x20,
      FailureOverflowHandler);
  
  assert(stack.ApplyBuffer());
  assert(stack.GetCount() == 1);
  assert(aligner.GetAllocCount() == 1);
  
  // Allocate a buffer
  assert(stack.Alloc(addr1, 1));
  assert(stack.ApplyBuffer());
  assert(stack.GetCount() == 1);
  assert(aligner.GetAllocCount() == 2);
  
  // Allocate another buffer
  assert(stack.Alloc(addr2, 1));
  assert(addr1 != addr2);
  assert(stack.ApplyBuffer());
  assert(stack.GetCount() == 1);
  assert(aligner.GetAllocCount() == 3);
  
  // Deallocate the second buffer
  stack.Dealloc(addr2, 1);
  assert(stack.GetCount() == 2);
  assert(stack.ApplyBuffer());
  assert(stack.GetCount() == 2);
  assert(aligner.GetAllocCount() == 3);
  
  // Deallocate the first buffer, causing an overflow
  stack.Dealloc(addr1, 1);
  assert(stack.ApplyBuffer());
  assert(stack.GetCount() == 2);
  assert(aligner.GetAllocCount() == 2);
  
  // Make sure that addr2 is still at the top of the stack
  assert(stack.Alloc(addr1, 1));
  assert(addr1 == addr2);
  stack.Dealloc(addr1, 1);
}

void TestReallocFree() {
  ScopedPass pass("VirtualBufferedStack::[Realloc/Free]()");
  
  uintptr_t addr;
  VirtualBufferedStack<3> stack(aligner, 1, 2, 0x20,
      FailureOverflowHandler);
  
  assert(stack.ApplyBuffer());
  assert(stack.GetCount() == 1);
  assert(aligner.GetAllocCount() == 1);
  
  assert(stack.Alloc(addr, 1));
  assert(stack.ApplyBuffer());
  uintptr_t oldAddr = addr;
  assert(stack.Realloc(addr, 0x20));
  assert(oldAddr == addr);
  assert(!stack.Realloc(addr, 0x21));
  stack.Free(addr);
  assert(stack.Alloc(addr, 0x20));
  assert(addr == oldAddr);
  stack.Free(addr);
}

void TestOverflows() {
  ScopedPass pass("VirtualBufferedStack::[Alloc/Dealloc]() [overflows]");
  
  uintptr_t addr;
  VirtualBufferedStack<3> stack(aligner, 1, 2, 0x20,
      ExpectedOverflowHandler);
  
  assert(!hasOverflowed);
  
  // Push some bogus buffers onto the stack.
  stack.Dealloc(10, 1);
  stack.Dealloc(15, 1);
  stack.Dealloc(20, 1);
  assert(!hasOverflowed);
  
  // Ensure that the stack overflows
  stack.Dealloc(25, 1);
  assert(hasOverflowed);
  assert(overflowAddress == 25);
  assert(overflowSize == 1);
  hasOverflowed = false;
  
  // Clear out the other bogus buffers
  stack.Alloc(addr, 1);
  stack.Alloc(addr, 1);
  stack.Alloc(addr, 1);
  
  // Test simple size overflow
  assert(stack.ApplyBuffer());
  assert(stack.GetCount() == 1);
  assert(!stack.Alloc(addr, 0x21));
  assert(stack.Alloc(addr, 0x20));
  stack.Dealloc(addr, 0x20);
}

template <typename T>
void FailureOverflowHandler(T *, uintptr_t, size_t) {
  std::cerr << "FailureOverflowHandler called" << std::endl;
  abort();
}

template <typename T>
void ExpectedOverflowHandler(T *, uintptr_t addr, size_t size) {
  hasOverflowed = true;
  overflowAddress = addr;
  overflowSize = size;
}
