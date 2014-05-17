#include "region.hpp"

namespace ANAlloc {

Region::Region(uintptr_t _start, size_t _size) {
  start = _start;
  size = _size;
}
  
Region::Region() : start(0), size(0) {}
  
Region::Region(const Region & reg) {
  *this = reg;
}
  
Region & Region::operator=(const Region & reg) {
  start = reg.start;
  size = reg.size;
  return *this;
}

uintptr_t Region::GetStart() const {
  return start;
}

uintptr_t Region::GetEnd() const {
  return start + size;
}

size_t Region::GetSize() const {
  return size;
}

bool Region::Contains(uintptr_t ptr) const {
  return ptr >= GetStart() && ptr < GetEnd();
}

}
