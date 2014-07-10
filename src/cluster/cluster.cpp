#include "cluster.hpp"

namespace ANAlloc {

void Cluster::Reserve(UInt start, UInt length) {
  for (int i = 0; i < GetCount(); i++) {
    Allocator & allocator = (*this)[i];
    UInt allocStart = allocator.GetStart();
    UInt allocSize = allocator.GetTotalSize();
    if (allocStart >= start + length) {
      continue;
    } else if (allocStart + allocSize <= start) {
      continue;
    }
    
    // compute the intersection
    UInt reserveStart = allocStart < start ? start : allocStart;
    UInt reserveLen = 0;
    if (start + length > allocStart + allocSize) {
      reserveLen = allocStart + allocSize - reserveStart;
    } else {
      reserveLen = start + length - reserveStart;
    }
    allocator.Reserve(reserveStart - allocStart, reserveLen);
  }
}

UInt Cluster::GetTotalSize() const {
  UInt totalSize = 0;
  for (int i = 0; i < GetCount(); i++) {
    totalSize += (*this)[i].GetTotalSize();
  }
  return totalSize;
}

UInt Cluster::GetFreeSize() const {
  UInt freeSize = 0;
  for (int i = 0; i < GetCount(); i++) {
    freeSize += (*this)[i].GetFreeSize();
  }
  return freeSize;
}

bool Cluster::Alloc(UInt size, UInt & addrOut) {
  for (int i = 0; i < GetCount(); i++) {
    if ((*this)[i].Alloc(size, addrOut)) {
      return true;
    }
  }
  return false;
}

bool Cluster::Align(UInt size, UInt align, UInt & addrOut) {
  for (int i = 0; i < GetCount(); i++) {
    if ((*this)[i].Align(size, align, addrOut)) {
      return true;
    }
  }
  return false;
}

void Cluster::Free(UInt addr) {
  for (int i = 0; i < GetCount(); i++) {
    Allocator & allocator = (*this)[i];
    if (!allocator.OwnsAddress(addr)) continue;
    allocator.Free(addr);
  }
}

}
