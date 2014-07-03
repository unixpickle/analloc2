#ifndef __ANALLOC2_REGION_H__
#define __ANALLOC2_REGION_H__

#include <cstdint>
#include <cstddef>

namespace ANAlloc {

/**
 * Represents an abstract region of memory.
 */
class Region {
private:
  uintptr_t start = 0;
  size_t size = 0;
  
public:
  Region(uintptr_t _start, size_t _size);
  Region();
  Region(const Region & reg);
  Region & operator=(const Region & reg);
  
  uintptr_t GetStart() const;
  uintptr_t GetEnd() const;
  size_t GetSize() const;
  bool Contains(uintptr_t ptr) const;
   
};

}

#endif
