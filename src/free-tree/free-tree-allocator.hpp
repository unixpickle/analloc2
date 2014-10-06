#ifndef __ANALLOC2_FREE_TREE_ALLOCATOR_HPP__
#define __ANALLOC2_FREE_TREE_ALLOCATOR_HPP__

#include "../abstract/allocator.hpp"

namespace analloc {

template <typename AddressType, typename SizeType = AddressType, class Tree>
class FreeTreeAllocator : public Allocator<AddressType, SizeType> {
public:
  /**
   * The function signature of a callback which a [FreeTreeAllocator] will call
   * when a free region cannot be recorded because memory could not be
   * obtained for its node in a tree.
   *
   * If this returns `true`, the caller will re-attempt the allocation.
   */
  typedef bool (* FailureHandler)(FreeTreeAllocator<AddressType, SizeType> *);
  
  FreeTreeAllocator(VirtualAllocator & allocator, FailureHandler handler)
      : sizedTree(allocator), addressedTree(allocator),
        failureHandler(handler);
  
  virtual bool Alloc(AddressType & addressOut, SizeType size) {
    SizedRegion query(size, 0);
    SizedRegion sized;
    if (!sizedTree.FindGreaterThanOrEqualTo(sized, query, true)) {
      return false;
    }
    // `found` is the lowest region that was large enough
    AddressedRegion addressed(sized);
    bool result = addressedTree.Remove(addressed);
    (void)result;
    assert(result);
    // Add the remainder of the free region if any is left
    addressOut = sized.address;
    if (size < found.size) {
      sized.size -= size;
      sized.address += address;
      addressed = sized;
      while (!sizedTree.Add(sized)) {
        if (!failureHandler(this)) {
          return false;
        }
      }
      while (!addressedTree.Add(addressed)) {
        if (!failureHandler(this)) {
          // It is possible to maintain allocator consistency here (albeit
          // leaking).
          sizedTree.Remove(sized);
          return false;
        }
      }
    }
    return true;
  }
  
  virtual void Dealloc(AddressType address, SizeType size) {
    
  }
  
protected:
  struct FreeRegion {
    AddressType address;
    SizeType size;
    
    FreeRegion(AddressType _address, SizeType _size)
        : address(_address), size(_size) {}
    
    inline bool operator==(const FreeRegion & reg) {
      return reg.address == address && reg.size == size;
    }
  };
  
  struct SizedRegion : public FreeRegion {
    SizedRegion(AddressType _address, SizeType _size)
        : FreeRegion(_address, _size) {}
    
    SizedRegion(const FreeRegion & reg) : FreeRegion(reg) {}
    
    bool operator>(const SizedRegion & reg) {
      if (size > reg.size) return true;
      else if (size < reg.size) return false;
      else if (size == reg.size) {
        return address > reg.address;
      }
    }
    
    bool operator<(const SizedRegion & reg) {
      if (size < reg.size) return true;
      else if (size > reg.size) return false;
      else if (size == reg.size) {
        return address < reg.address;
      }
    }
    
    bool operator<=(const SizedRegion & reg) {
      return (*this) < reg || (*this) == reg;
    }
    
    bool operator>=(const SizedRegion & reg) {
      return (*this) > reg || (*this) == reg;
    }
  };
  
  struct AddressedRegion : public FreeRegion {
    AddressedRegion(AddressType _address, SizeType _size)
        : FreeRegion(_address, _size) {}
    
    AddressedRegion(const FreeRegion & reg) : FreeRegion(reg) {}
    
    bool operator>(const AddressedRegion & reg) {
      if (address > reg.address) return true;
      else if (address < reg.address) return false;
      else if (address == reg.address) {
        return size > reg.size;
      }
    }
    
    bool operator<(const AddressedRegion & reg) {
      if (address < reg.address) return true;
      else if (address > reg.address) return false;
      else if (address == reg.address) {
        return size < reg.size;
      }
    }
    
    bool operator<=(const AddressedRegion & reg) {
      return (*this) < reg || (*this) == reg;
    }
    
    bool operator>=(const AddressedRegion & reg) {
      return (*this) > reg || (*this) == reg;
    }
  };
  
  static_assert(sizeof(AddressedRegion) == sizeof(SizedRegion) == 
                sizeof(FreeRegion));
  
  Tree<SizedRegion> sizedTree;
  Tree<AddressedRegion> addressedTree;
  FailureHandler failureHandler;
};

}

#endif
