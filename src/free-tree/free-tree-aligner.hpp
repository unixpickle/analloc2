#ifndef __ANALLOC2_FREE_TREE_ALIGNER_HPP__
#define __ANALLOC2_FREE_TREE_ALIGNER_HPP__

#include "free-tree-allocator.hpp"
#include "dynamic-tree.hpp"

namespace analloc {

template <template <class T> class Tree, typename AddressType,
          typename SizeType = AddressType>
class FreeTreeAligner : public FreeTreeAllocator<Tree, AddressType, SizeType> {
public:
  typedef FreeTreeAllocator<Tree, AddressType, SizeType> super;
  using typename super::FreeRegion;
  using typename super::AddressedRegion;
  using typename super::SizedRegion;
  using typename super::FailureHandler;
  
  FreeTreeAligner(VirtualAllocator & allocator, FailureHandler handler)
      : super(allocator, handler) {}
  
  class FittingEnumerator
      : public DynamicTree<AddressedRegion>::EnumerateCallback {
  public:
    FittingEnumerator(AddressType _align, SizeType _size)
        : align(_align), size(_size) {}
    
    bool Yield(const AddressedRegion & region) {
      offset = 0;
      if (region.address % align) {
        offset = align - (region.address % align);
      }
      if (offset + size > region.size) {
        // Continue enumerating
        return true;
      } else {
        // Stop enumerating
        result = region;
        return false;
      }
    }
    
    // Result
    AddressType offset;
    AddressedRegion result;
    
  protected:
    // Parameters
    AddressType align;
    SizeType size;
  };
  
  virtual bool Align(AddressType & addressOut, AddressType align,
                     SizeType size) {
    FittingEnumerator callback(align, size);
    if (this->addressedTree.Enumerate(callback)) {
      // The callback didn't find a suitable region
      return false;
    }
    this->addressedTree.Remove(callback.result);
    this->sizedTree.Remove(SizedRegion(callback.result));
    // Return the allocated address.
    addressOut = callback.result.address + callback.offset;
    // Split up the block as needed.
    if (callback.offset > 0) {
      // A sliver of free space remains at the beginning of the affected
      // region.
      this->AddRegion(FreeRegion(callback.result.address, 
                                 (SizeType)callback.offset));
    }
    if (callback.offset + size < callback.result.size) {
      // A sliver of free space remains at the end of the affected region.
      
      // We can cast callback.offset to a SizeType in this case because we know
      // it is smaller than callback.result.size which *is* a SizeType.
      SizeType remainingSize = callback.result.size -
          (SizeType)(callback.offset + size);
      
      this->AddRegion(FreeRegion(addressOut + size, remainingSize));
    }
    return true;
  }
};

}

#endif