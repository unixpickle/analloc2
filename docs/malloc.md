# The Malloc API

The `ANAlloc::Malloc<T>` class will make dynamic memory allocation a joy. An instance of `ANAlloc::Malloc<T>` manages allocation in one fixed-size contiguous memory chunk. Thus, if you have a way to allocate large contiguous chunks of memory, the Malloc API can take care of the details.

## Usage

For this example, I will assume that you have a function called `Allocate2MBChunk()` which allocates a contiguous 2MiB chunk of memory.

To create a new Malloc object, this is roughly what you'd do:

    #include <analloc2>
    #include <new>
    typedef MallocRegion ANAlloc::Malloc<ANAlloc::BBTree>;
    ...
    uint8_t * region = (uint8_t *)Allocate2MBChunk();
    MallocRegion * allocator = new(region) MallocRegion(region, 0x40,
                                                        sizeof(MallocRegion),
                                                        0x200000);

This will leave you with a variable `allocator` which points to an instance of `ANAlloc::Malloc<ANAlloc::BBTree>` which manages a 2MiB region of memory. This region of memory will contain the allocator itself, as well as the binary tree that helps it manage memory.

**Note:** for this particular example, I passed a page size of `0x40` (64 bytes). This means that if you allocate less than 64 bytes, it will be equivalent to allocating a full 64 bytes. It is important to keep this value large enough that the backing binary tree uses a small percentage of the total memory in the region.

Anyways, back to the usage. You may use the allocator you just created as follows:

    void * myBuffer = allocator->AllocBuf(30);
    // ... do something with myBuffer here
    allocator->FreeBuf(myBuffer);

When an allocation fails because there is not enough space in the region, `AllocBuf()` will return `NULL`.