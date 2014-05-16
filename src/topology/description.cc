#include "description.hpp"

namespace ANAlloc {

Description::Description() : pageSize(0), start(0), depth(0) { }

Description::Description(size_t _pageSize)
  : pageSize(_pageSize), start(0), depth(0) { }

Description::Description(size_t _pageSize, uintptr_t _start, int _depth)
  : pageSize(_pageSize), start(_start), depth(_depth) { }

Description::Description(const Description & desc) {
  *this = desc;
}

Description & Description::operator=(const Description & desc) {
  pageSize = desc.pageSize;
  depth = desc.depth;
  start = desc.start;
}

int Description::GetDepth() const {
  return depth;
}

size_t Description::GetSize() const {
  assert(pageSize);
  if (!depth) return 0;
  return pageSize << (depth - 1);
}

uintptr_t Description::GetStart() const {
  return start;
}

uintptr_t Description::GetEnd() const {
  return start + GetSize();
}

bool Description::Contains(uintptr_t ptr) const {
  return ptr >= GetStart() && ptr < GetEnd();
}

void Description::SetDepth(int _depth) {
  depth = _depth;
}

void Description::SetStart(uintptr_t _start) {
  start = _start;
}

}