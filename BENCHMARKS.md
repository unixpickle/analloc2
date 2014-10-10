# Performance Benchmarks

Running my benchmarks has given me a decent set of results on a 2.6 GHz Intel Core i7. In no way do I claim that my allocators will always perform this well, but these benchmarks do give a good ballpark estimate.

## Bitmap Allocator

The allocation time is **O**(*n*), where *n* is the number of bits which must be read before a set of free bits are found.

On a dataset in which only the last bit is free, the algorithm scans about *7 bits per clockcycle*.

## Free-list Allocator

The allocation time is **O**(*n*), where *n* is the number of free regions which must be scanned before a large enough free region is found.

My benchmarks showed that it takes roughly *5 clockcycles for each free region to be traversed*.

NOTE: The performance benchmark for this allocator uses a high-performance single-sized allocator to allocate free regions. This way, the performance of the standard UNIX malloc() and free() will not affect the results of the benchmark.

## Free-tree Allocator

This allocator uses two binary search trees to manage regions of free memory in **O**(*log(n)*) time, where *n* is the number of free regions in the allocator.

The current test uses the free-tree allocator in conjunction with my native AVL-tree implementation. This benchmarked showed that that, in the best case,  an allocation and deallocation operation together take *32 more clockcycles per factor of 2 in the number of free regions*. This means that doubling the number of free regions increases the combined Alloc and Dealloc time by roughly 32 clockcycles.
