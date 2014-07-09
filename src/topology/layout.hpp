#ifndef __ANALLOC2_LAYOUT_HPP__
#define __ANALLOC2_LAYOUT_HPP__

#include "desc-list.hpp"
#include "region-list.hpp"

namespace ANAlloc {

class Layout {
public:
  Layout(DescList & descs, int psLog, UInt maxAlignment, UInt minAlignment);
  
  void Run(const RegionList & regs);
  
protected:
  bool GenerateDesc(const RegionList & regs, UInt alignment);
  bool BiggestFree(const Region & reg, UInt alignment, Desc & desc);
  Desc * NextDescInRegion(const Region & reg, UInt start);
  UInt DescEnd(const Desc & d);
  
private:
  DescList & descs;
  int psLog;
  UInt maxAlignment;
  UInt minAlignment;
};

}

#endif
