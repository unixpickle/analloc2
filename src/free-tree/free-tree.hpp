#ifndef __ANALLOC2_FREE_TREE_HPP__
#define __ANALLOC2_FREE_TREE_HPP__

#include "../abstract/offset-aligner.hpp"

namespace analloc {

template <template <class T> class Tree, typename AddressType,
          typename SizeType = AddressType>
class FreeTree : public virtual OffsetAligner<AddressType, SizeType> {
public:
  /**
   * The function signature of a callback which a [FreeTree] will call when a
   * free region cannot be recorded because memory could not be obtained for
   * its node in a tree.
   *
   * If this returns `true`, the caller will re-attempt the allocation.
   */
  typedef bool (* FailureHandler)(FreeTree<Tree, AddressType, SizeType> *);
  
  FreeTree(VirtualAllocator & allocator, FailureHandler handler)
      : sizedTree(allocator), addressedTree(allocator),
        failureHandler(handler) {}
  
  virtual bool Alloc(AddressType & addressOut, SizeType size) {
    SizedRegion sized;
    if (!sizedTree.FindGE(sized, SizedRegion(0, size), true)) {
      return false;
    }
    // [sized] is the lowest region whose size was >= [size]
    AddressedRegion addressed(sized);
    addressedTree.Remove(addressed);
    // The result is the beginning of this region
    addressOut = sized.address;
    if (size < sized.size) {
      // The free region has some excess; put it back in the trees.
      sized.size -= size;
      sized.address += size;
      return AddRegion(sized);
    }
    return true;
  }
  
  virtual void Dealloc(AddressType address, SizeType size) {
    AddressedRegion before, after;
    bool hasBefore = false, hasAfter = false;
    if (addressedTree.FindLT(before, AddressedRegion(address, size))) {
      if (before.address + before.size == address) {
        hasBefore = true;
      }
    }
    if (addressedTree.FindGT(after, AddressedRegion(address, size))) {
      if (address + size == after.address) {
        hasAfter = true;
      }
    }
    if (hasBefore && hasAfter) {
      // Remove both regions and join them all together
      addressedTree.Remove(before);
      addressedTree.Remove(after);
      sizedTree.Remove(SizedRegion(before));
      sizedTree.Remove(SizedRegion(after));
      AddRegion(FreeRegion(before.address, before.size + size + after.size));
    } else if (hasBefore) {
      // Remove the previous region and join it with the freed region
      addressedTree.Remove(before);
      sizedTree.Remove(SizedRegion(before));
      AddRegion(FreeRegion(before.address, before.size + size));
    } else if (hasAfter) {
      // Remove the next region and join it with the freed region
      addressedTree.Remove(after);
      sizedTree.Remove(SizedRegion(after));
      AddRegion(FreeRegion(address, after.size + size));
    } else {
      // Simple case: insert the free region
      AddRegion(FreeRegion(address, size));
    }
  }
  
  virtual bool OffsetAlign(AddressType & addressOut, AddressType align,
                           AddressType offset, SizeType size) {
    FittingEnumerator callback(align, offset, size);
    if (addressedTree.Enumerate(callback)) {
      // The callback didn't find a suitable region
      return false;
    }
    addressedTree.Remove(callback.result);
    sizedTree.Remove(SizedRegion(callback.result));
    // Return the allocated address.
    addressOut = callback.result.address + callback.offset;
    // Split up the block as needed.
    if (callback.offset > 0) {
      // A sliver of free space remains at the beginning of the affected
      // region.
      AddRegion(FreeRegion(callback.result.address, 
                           (SizeType)callback.offset));
    }
    if (callback.offset + size < callback.result.size) {
      // A sliver of free space remains at the end of the affected region.
      
      // We can cast callback.offset to a SizeType in this case because we know
      // it is smaller than callback.result.size which *is* a SizeType.
      SizeType remainingSize = callback.result.size -
          (SizeType)(callback.offset + size);
      
      AddRegion(FreeRegion(addressOut + size, remainingSize));
    }
    return true;
  }
  
  /**
   * A "free region" in this allocator. Both subclasses of [FreeRegion], namely
   * [AddressedRegion] and [SizedRegion], have the same size and packing
   * properties as [FreeRegion].
   */
  struct FreeRegion {
    AddressType address;
    SizeType size;
    
    FreeRegion() : address(0), size(0) {}
    
    FreeRegion(AddressType _address, SizeType _size)
        : address(_address), size(_size) {}
    
    inline bool operator==(const FreeRegion & reg) const {
      return reg.address == address && reg.size == size;
    }
  };
  
  /**
   * A [FreeRegion] which is sorted first by size and then by address.
   */
  struct SizedRegion : public FreeRegion {
    SizedRegion() : FreeRegion() {}
    
    SizedRegion(AddressType _address, SizeType _size)
        : FreeRegion(_address, _size) {}
    
    SizedRegion(const FreeRegion & reg) : FreeRegion(reg) {}
    
    bool operator>(const SizedRegion & reg) const {
      if (this->size > reg.size) return true;
      else if (this->size < reg.size) return false;
      else {
        return this->address > reg.address;
      }
    }
    
    bool operator<(const SizedRegion & reg) const {
      if (this->size < reg.size) return true;
      else if (this->size > reg.size) return false;
      else {
        return this->address < reg.address;
      }
    }
    
    bool operator<=(const SizedRegion & reg) const {
      return (*this) < reg || (*this) == reg;
    }
    
    bool operator>=(const SizedRegion & reg) const {
      return (*this) > reg || (*this) == reg;
    }
  };
  
  /**
   * A [FreeRegion] which is sorted first by address and then by size.
   */
  struct AddressedRegion : public FreeRegion {
    AddressedRegion() : FreeRegion() {}
    
    AddressedRegion(AddressType _address, SizeType _size)
        : FreeRegion(_address, _size) {}
    
    AddressedRegion(const FreeRegion & reg) : FreeRegion(reg) {}
    
    bool operator>(const AddressedRegion & reg) const {
      if (this->address > reg.address) return true;
      else if (this->address < reg.address) return false;
      else {
        return this->size > reg.size;
      }
    }
    
    bool operator<(const AddressedRegion & reg) const {
      if (this->address < reg.address) return true;
      else if (this->address > reg.address) return false;
      else {
        return this->size < reg.size;
      }
    }
    
    bool operator<=(const AddressedRegion & reg) const {
      return (*this) < reg || (*this) == reg;
    }
    
    bool operator>=(const AddressedRegion & reg) const {
      return (*this) > reg || (*this) == reg;
    }
  };
  
  static_assert(sizeof(AddressedRegion) == sizeof(SizedRegion),
                "invalid FreeRegion subclasses");
  static_assert(sizeof(AddressedRegion) == sizeof(FreeRegion),
                "invalid FreeRegion subclasses");
  
  /**
   * An enumerator which allows the [Align] method to work.
   */
  class FittingEnumerator
      : public DynamicTree<AddressedRegion>::EnumerateCallback {
  public:
    FittingEnumerator(AddressType _align, AddressType _offset, SizeType _size)
        : align(_align), alignOffset(_offset), size(_size) {}
    
    bool Yield(const AddressedRegion & region) {
      offset = 0;
      if ((region.address + alignOffset) % align) {
        offset = align - ((region.address + alignOffset) % align);
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
    AddressType alignOffset;
    SizeType size;
  };
  
protected:
  Tree<SizedRegion> sizedTree;
  Tree<AddressedRegion> addressedTree;
  FailureHandler failureHandler;
  
  bool AddRegion(FreeRegion region) {
    while (!addressedTree.Add(AddressedRegion(region))) {
      if (!failureHandler(this)) {
        return false;
      }
    }
    while (!sizedTree.Add(SizedRegion(region))) {
      if (!failureHandler(this)) {
        // Avoid having inconsistent trees (although the region will leak).
        addressedTree.Remove(AddressedRegion(region));
        return false;
      }
    }
    return true;
  }
};

}

#endif
