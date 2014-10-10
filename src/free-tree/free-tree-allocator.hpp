#ifndef __ANALLOC2_FREE_TREE_ALLOCATOR_HPP__
#define __ANALLOC2_FREE_TREE_ALLOCATOR_HPP__

#include "../abstract/allocator.hpp"

namespace analloc {

template <template <class T> class Tree, typename AddressType,
          typename SizeType = AddressType>
class FreeTreeAllocator : public Allocator<AddressType, SizeType> {
public:
  typedef FreeTreeAllocator<Tree, AddressType, SizeType> ThisType;
  
  /**
   * The function signature of a callback which a [FreeTreeAllocator] will call
   * when a free region cannot be recorded because memory could not be
   * obtained for its node in a tree.
   *
   * If this returns `true`, the caller will re-attempt the allocation.
   */
  typedef bool (* FailureHandler)(ThisType *);
  
  FreeTreeAllocator(VirtualAllocator & allocator, FailureHandler handler)
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
      addressed = sized;
      while (!sizedTree.Add(sized)) {
        if (!failureHandler(this)) {
          return false;
        }
      }
      while (!addressedTree.Add(addressed)) {
        if (!failureHandler(this)) {
          // It is possible to maintain allocator consistency here (albeit
          // leaking some address space).
          sizedTree.Remove(sized);
          return false;
        }
      }
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
      AddressedRegion newRegion(before.address, before.size + size +
                                after.size);
      addressedTree.Add(newRegion);
      sizedTree.Add(SizedRegion(newRegion));
    } else if (hasBefore) {
      // Remove the previous region and join it with the freed region
      addressedTree.Remove(before);
      sizedTree.Remove(SizedRegion(before));
      AddressedRegion newRegion(before.address, before.size + size);
      addressedTree.Add(newRegion);
      sizedTree.Add(SizedRegion(newRegion));
    } else if (hasAfter) {
      // Remove the next region and join it with the freed region
      addressedTree.Remove(after);
      sizedTree.Remove(SizedRegion(after));
      AddressedRegion newRegion(address, after.size + size);
      addressedTree.Add(newRegion);
      sizedTree.Add(SizedRegion(newRegion));
    } else {
      // Simple case: insert the free region
      addressedTree.Add(AddressedRegion(address, size));
      sizedTree.Add(SizedRegion(address, size));
    }
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
  
protected:
  Tree<SizedRegion> sizedTree;
  Tree<AddressedRegion> addressedTree;
  FailureHandler failureHandler;
};

}

#endif
