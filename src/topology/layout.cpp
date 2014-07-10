#include "../log.hpp"
#include "layout.hpp"
#include <cstddef>
#include <cassert>

namespace ANAlloc {

Layout::Layout(DescList & _descs, const RegionList & _regions, int _psLog,
               UInt _maxAlignment, UInt _minAlignment)
  : descs(_descs), regions(_regions), psLog(_psLog),
    maxAlignment(_maxAlignment), minAlignment(_minAlignment) {
  assert(minAlignment != 0);
  assert(maxAlignment != 0);
}

void Layout::Run() {
  // keep adding descriptions until we can't anymore
  alignment = maxAlignment;
  while (alignment >= minAlignment) {
    if (!GenerateDesc()) {
      alignment >>= 1;
    }
  }
}

// PROTECTED //

bool Layout::GenerateDesc() {
  Desc biggest(0, 0);
  for (int i = 0; i < regions.GetCount(); i++) {
    const Region & reg = regions[i];
    Desc output;
    if (BiggestFree(reg, output)) {
      if (output.GetDepth() > biggest.GetDepth()) {
        biggest = output;
      }
    }
  }
  if (!biggest.GetDepth()) return false;
  return descs.Push(biggest);
}

bool Layout::BiggestFree(const Region & reg, Desc & desc) {
  const UInt pageSize = 1UL << psLog;
  desc = Desc(0, 0);
  
  UInt chunkStart = reg.GetStart();
  while (chunkStart + pageSize <= reg.GetEnd()) {
    Desc * nextDesc = NextDescInRegion(reg, chunkStart);
    
    // calculate the beginning and end of this free chunk
    UInt chunkEnd = nextDesc ? nextDesc->GetStart() : reg.GetEnd();
    if (chunkStart % alignment) {
      chunkStart += alignment - (chunkStart % alignment);
    }
    
    // if the free chunk is not large enough, continue after the description
    // that's in the way
    if (chunkEnd <= chunkStart || chunkEnd - chunkStart < pageSize) {
      if (!nextDesc) break;
      chunkStart = DescEnd(*nextDesc);
      continue;
    }
    
    // calculate the depth of this free region and create an allocator in it
    int possibleDepth = Log2Floor(chunkEnd - chunkStart) - psLog + 1;
    if (possibleDepth > desc.GetDepth()) {
      desc = Desc(chunkStart, possibleDepth);
    }
    
    if (!nextDesc) break;
    chunkStart = DescEnd(*nextDesc);
    continue;
  }
  
  return desc.GetDepth() != 0;
}

Desc * Layout::NextDescInRegion(const Region & reg, UInt start) {
  Desc * result = NULL;
  for (int i = 0; i < descs.GetCount(); i++) {
    Desc & d = descs[i];
    if (!reg.Contains(d.GetStart())) continue;
    if (d.GetStart() < start) continue;
    if (result && result->GetStart() < d.GetStart()) continue;
    result = &d;
  }
  return result;
}

UInt Layout::DescEnd(const Desc & d) {
  return d.GetStart() + (1UL << (psLog + d.GetDepth() - 1));
}

}