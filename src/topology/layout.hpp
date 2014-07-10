#ifndef __ANALLOC2_LAYOUT_HPP__
#define __ANALLOC2_LAYOUT_HPP__

#include "desc-list.hpp"
#include "region-list.hpp"

namespace ANAlloc {

class Layout {
public:
  Layout(DescList & descs, const RegionList & regions, int psLog,
         UInt maxAlignment, UInt minAlignment);
  
  void Run();
  
protected:
  bool GenerateDesc();
  bool BiggestFree(const Region & reg, Desc & desc);
  Desc * NextDescInRegion(const Region & reg, UInt start);
  UInt DescEnd(const Desc & d);
  
private:
  DescList & descs;
  const RegionList & regions;
  int psLog;
  UInt maxAlignment;
  UInt minAlignment;
  
  UInt alignment;
};

}

#endif
