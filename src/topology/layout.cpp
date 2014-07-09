#include "../logs.hpp"

namespace ANAlloc {

Layout::Layout(DescList & _descs, int _psLog, UInt _maxAlignment,
               UInt _minAlignment)
  : descs(_descs), psLog(_psLog), maxAlignment(_maxAlignment), minAlignment(_minAlignment) {
}

void Layout::Run(const RegionList & regs) {
  // keep adding descriptions until we can't anymore
  UInt alignment = maxAlignment;
  while (alignment >= minAlignment && descs.CanPush()) {
    if (!GenerateDesc(regs, alignment, keepGoing)) {
      alignment >>= 1;
    }
  }
}

// PROTECTED //

bool Layout::GenerateDesc(const RegionList & regs, UInt alignment) {
  Desc biggest(0, 0);
  keepGoing = true;
  for (int i = 0; i < regs.GetCount(); i++) {
    const Region & reg = regs[i];
    Desc output;
    if (BiggestFree(reg, alignment, output)) {
      if (output.GetDepth() > biggest.GetDepth()) {
        biggest = output;
      }
    }
  }
  if (!biggest.GetDepth()) return false;
  return descs.Push(biggest);
}

bool Layout::BiggestFree(const Region & reg, UInt alignment, Desc & desc) {
  const UInt pageSize = 1UL << psLog;
  desc = Desc(0, 0);
  
  UInt chunkStart = reg.GetStart();
  while (chunkStart + pageSize <= reg.GetEnd()) {
    Desc * nextDesc = NextDescInRegion(reg, chunkStart);
    
    // calculate the beginning and end of this free chunk
    UInt chunkEnd = nextDesc ? nextDesc.GetStart() : reg.GetEnd();
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
    int possibleDepth = Log2Floor(chunkEnd - chunkStart);
    if (possibleDepth > desc.GetDepth()) {
      desc = Desc(chunkStart, possibleDepth);
    }
    
    if (!nextDesc) break;
    chunkStart = DescEnd(*nextDesc);
    continue;
  }
  
  return desc.depth != 0;
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