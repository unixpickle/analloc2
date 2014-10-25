# TODO

## Crucial

Upcoming completion of free-list allocator.

 * Test `FreeListAligner::OffsetAlign()` with non-zero offsets.
 * Test integer overflow cases in free-list.
 * Create transformed suite for free-list.
 * Test transformed suite for free-list.
 * Create a way to wrap a virtual region of memory with a free-list.
 * Test virtual free list allocator.

Upcoming completion of free-tree allocator:

 * Implement `OffsetAlign()` in free-tree.
 * Create transformed suite for `FreeTreeAllocator`.

## Specific features

 * Performance monitoring template argument for AVL tree
 * Add LastUsedAddress() to `BitmapAllocator` and `FreeListAllocator`.
 * Add RegionCount() to `FreeListAllocator`.
 * (Possibly) add some sort of standard deviation algorithm to see how fragmented allocators are.
 * Implement red-black tree
 * Implement splay tree

## Areas of focus

I will focus on the following features for version 1.0.0:

 * Template-determined integer types
 * Scalable memory management
 * Sub-allocators
 * Distributed allocators for "NUMA" domains
 * Multiple types of allocators:
   * Buddy allocation
   * Page allocation queue
   * Free bitmap

Here is my current TODO list. I will update it as I think of new things to implement:

 * Depend on ansa for bitmap, logarithm, and locking features
 * Bitmap allocator class

## Concerns from version 0.1.0

Performance: I have found that allocating memory on a typical x86-64 system usually takes upwards of 3,000 clock-cycles. I think this is rather high, considering that a linked-list implementation would only take a few hundred cycles.

Usability: Integer types are a major headache in any C-based language, and they plague analloc2. On a 32-bit system, usually it will make sense to use 32-bit integers for a virtual memory allocator. However, on x86 with PAE, you will probably want 64-bit integers for physical memory allocation. Because of situations like these, I think it might be best to make the integer type a template argument.

By making integer types a template argument, I will be forcing myself to implement everything in headers. This will allow for better optimization and potentially remove the need for virtual methods. However, I am not sure if I really *want* to do such a thing.