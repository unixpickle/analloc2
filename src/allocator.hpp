#ifndef __ANALLOC2_ALLOCATOR_HPP__
#define __ANALLOC2_ALLOCATOR_HPP__

#include "tree/tree.hpp"
#include "log.hpp"

namespace ANAlloc {

/**
 * This lightweight template wraps the tree of your choice to provide a simple
 * numerical chunk allocator (for physical memory or any other address space)
 */
class Allocator {  
public:
  Allocator(UInt start, Tree & tree, int psLog);
  
  /**
   * Allocate a chunk of address space.
   */
  bool Alloc(UInt size, UInt & result);
  
  /**
   * Allocate an address of a certain size, aligned by a certain size.
   * @discussion The alignment must be a power of two
   */
  bool Align(UInt size, UInt align, UInt & result);
  
  /**
   * Free a buffer by passing the pointer to its start.
   */
  void Free(UInt addr);
  
  /**
   * Check if a pointer is contained within the boundary of the allocator's
   * buffer.
   */
  bool OwnsAddress(UInt addr) const;
  
  /**
   * Returns the internal tree used by the allocator.
   */
  const Tree & GetTree() const;
  
  /**
   * Returns the log base 2 of the base node size.
   */
  int GetPageSizeLog() const;
  
protected:
  UInt start;
  UInt length;
  
  Tree & tree;
  int psLog;
  
  int DepthForSize(UInt size) const;
};

}

#endif
