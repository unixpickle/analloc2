#ifndef __ANALLOC2_BUFFERED_STACK_HPP__
#define __ANALLOC2_BUFFERED_STACK_HPP__

#include <cstddef>
#include "../abstract/allocator.hpp"

namespace analloc {

/**
 * An allocator which returns fixed-sized objects from a stack.
 *
 * The "buffered" component of a [BufferedStack] describes its soft minimum
 * and soft maximum capacities. These allow the stack to make sure it
 * never gets too full or too empty.
 *
 * Since buffered stacks are fixed-size, the [Capacity] template argument is
 * necessary to specify the stack size.
 */
template <size_t Capacity, typename AddressType,
          typename SizeType = AddressType>
class BufferedStack : public virtual Allocator<AddressType, SizeType> {
public:
  typedef Allocator<AddressType, SizeType> SourceType;
  
  /**
   * A function which is called when a [BufferedStack] overflows.
   *
   * The calling [BufferedStack] will not be able to save the specified region.
   */
  typedef void (* OverflowHandler)(BufferedStack<Capacity, AddressType,
      SizeType> *, AddressType, SizeType);
  
  /**
   * Create a new [BufferedStack] which obtains objects from a [source].
   *
   * When [ApplyBuffer] is called on this instance, it will use [source] to
   * allocate or free objects so that the stack contains no less than
   * [softMinimum] objects and no more than [softMaximum] objects. Each object
   * will be exactly [objectSize] units.
   *
   * The [overflowHandler] is called when [Dealloc] is called on this instance
   * while it is full.
   */
  BufferedStack(SourceType & source, size_t softMinimum, size_t softMaximum,
                SizeType objectSize, OverflowHandler overflowHandler)
      : source(source), softMinimum(softMinimum), softMaximum(softMaximum),
        objectSize(objectSize), overflowHandler(overflowHandler) {
    assert(softMinimum <= softMaximum);
    assert(objectSize > 0);
  }
  
  virtual ~BufferedStack() {
    while (count) {
      source.Dealloc(stack[--count], objectSize);
    }
  }
  
  /**
   * Allocate an object from the stack.
   *
   * This will fail if [size] is greater than the object size of the stack, or
   * if the stack is empty.
   */
  virtual bool Alloc(AddressType & addressOut, SizeType size) {
    if (count == 0 || size > objectSize) {
      return false;
    } else {
      addressOut = stack[--count];
      return true;
    }
  }
  
  /**
   * Push an object to the stack.
   *
   * The [size] must be less than or equal to the object size of this stack.
   */
  virtual void Dealloc(AddressType address, SizeType size) {
    assert(size <= objectSize);
    if (count == Capacity) {
      overflowHandler(this, address, size);
    } else {
      stack[count++] = address;
    }
  }
  
  /**
   * Apply the soft minimum and maximum by allocating or freeing objects from
   * the source allocator.
   *
   * Returns `false` if and only if an allocation from the source allocator 
   * fails.
   */
  bool ApplyBuffer() {
    while (count < softMinimum) {
      AddressType address;
      if (!source.Alloc(address, objectSize)) {
        return false;
      }
      assert(count < Capacity);
      stack[count++] = address;
    }
    while (count > softMaximum) {
      source.Dealloc(stack[--count], objectSize);
    }
    return true;
  }
  
  inline size_t GetCount() {
    return count;
  }
  
  inline size_t GetSoftMinimum() {
    return softMinimum;
  }
  
  inline size_t GetSoftMaximum() {
    return softMaximum;
  }
  
  inline SizeType GetObjectSize() {
    return objectSize;
  }
  
protected:
  SourceType & source;
  AddressType stack[Capacity];
  size_t count = 0;
  size_t softMinimum;
  size_t softMaximum;
  SizeType objectSize;
  OverflowHandler overflowHandler;
};

}

#endif
