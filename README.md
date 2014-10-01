# Another Allocator?

I want to create a multi-purpose allocator, not just for memory but for any numerically-addressed resource! This could be file descriptors, I/O ports, physical memory, or something I haven't even conceived of yet.

# Usage

I am currently rewriting the allocator for version 1.0.0. When this rewrite is complete, I will add documentation to this project.

# Performance Benchmarks

Running my benchmarks has given me a decent set of results on a 2.6 GHz Intel Core i7. In no way do I claim that my allocators will always perform this well, but these benchmarks do give a good ballpark estimate.

## Bitmap Allocator

The allocation time is **O**(*n*), where *n* is the number of bits which must be read before a set of free bits are found.

On a dataset in which only the last bit is free, the algorithm scans about *7 bits per clockcycle*.

## Free-list Allocator

The allocation time is **O**(*n*), where *n* is the number of free regions which must be scanned before a large enough free region is found.

My benchmarks showed that it takes roughly *9 clockcycles for each free region to be traversed*.

NOTE: The performance benchmark for this allocator uses the standard POSIX `malloc()` and `free()` functions for region allocation/deallocation. The benchmarks take long enough to run that the running time of these functions tends not to affect the performance of the overall implementation.

# TODO

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

My current concerns about version 0.1.1 are usability and performance:

Performance: I have found that allocating memory on a typical x86-64 system usually takes upwards of 3,000 clock-cycles. I think this is rather high, considering that a linked-list implementation would only take a few hundred cycles.

Usability: Integer types are a major headache in any C-based language, and they plague analloc2. On a 32-bit system, usually it will make sense to use 32-bit integers for a virtual memory allocator. However, on x86 with PAE, you will probably want 64-bit integers for physical memory allocation. Because of situations like these, I think it might be best to make the integer type a template argument.

By making integer types a template argument, I will be forcing myself to implement everything in headers. This will allow for better optimization and potentially remove the need for virtual methods. However, I am not sure if I really *want* to do such a thing.

# License

analloc2 is licensed under the BSD 2-clause license. See [LICENSE](https://github.com/unixpickle/analloc2/blob/master/LICENSE).

```
Copyright (c) 2014, Alex Nichol.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer. 
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```
