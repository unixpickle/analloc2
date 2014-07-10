#include "malloc.hpp"

namespace ANAlloc {

Malloc::Malloc(uint8_t * _start, Tree & _tree, int _psLog)
  : super((UInt)_start, _tree, _psLog) {
}

void Malloc::Reserve(size_t resStart, size_t resSize) {
  super::Reserve((UInt)resStart, (UInt)resSize);
}

void * Malloc::Alloc(size_t size) {
  UInt result;
  if (!super::Alloc((UInt)size, result)) {
    return NULL;
  } else {
    return (void *)result;
  }
}

void * Malloc::Align(size_t size, size_t align) {
  UInt result;
  if (!super::Align((UInt)size, (UInt)align, result)) {
    return NULL;
  } else {
    return (void *)result;
  }
}

void Malloc::Free(void * buff) {
  super::Free((UInt)buff);
}

bool Malloc::OwnsPointer(void * ptr) const {
  return super::OwnsAddress((UInt)ptr);
}

size_t Malloc::GetFreeSize() const {
  return (size_t)super::GetFreeSize();
}

size_t Malloc::GetTotalSize() const {
  return (size_t)super::GetTotalSize();
}

void * Malloc::GetStart() const {
  return (void *)super::GetStart();
}

}
