#include <iostream>
#include "../src/malloc.h"
#include "../src/bbtree.h"

using namespace std;

typedef ANAlloc::BBTree BBTree;
typedef ANAlloc::Malloc<BBTree> Malloc;

int main() {
  cout << "testing Malloc wrapper... ";
  
  uint8_t * buf = new uint8_t[0x200000];
  Malloc * m = new(buf) Malloc(buf, 0x40, sizeof(Malloc), 0x200000);
  
  // there are 0x8000=2^15 base nodes, so depth=16.
  size_t prefix = BBTree::MemorySize(16) + sizeof(Malloc);
  if (prefix & 0x3f) prefix += 0x40 - (prefix & 0x3f);
  
  void * result = m->AllocBuf(0x40);
  assert((uintptr_t)result == (uintptr_t)prefix + (uintptr_t)buf);
  m->FreeBuf(result);
  
  void * newResult = m->AllocBuf(0x40);
  assert(newResult == result);
  m->FreeBuf(newResult);
  
  assert(!m->AllocBuf(0x100001));
  
  result = m->AllocBuf(0x100000);
  assert(result == (void *)(buf + 0x100000));
  m->FreeBuf(result);
  
  delete buf;
  
  cout << "passed!" << endl;
  return 0;
}
