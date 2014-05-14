# Another Allocator?

I hope this will be the last pure Buddy Allocator that I implement. I am trying really hard to abstract as much as I can so that this is modular and I can improve on it in the future.

# Usage

While this is not completely done, here's the gist of how it works. Essentially, you need to setup some sort of binary tree, and then you setup an "allocator" on top of that to manage allocations. Then, on top of that, there will be some sort of concrete virtual allocator, but I haven't made that yet.

Currently, I have two types of binary trees implemented. I'm going to show you how to use the `BBTree`, but the `BTree` works very similarly. In the future, I will probably implement better algorithms like AVL trees.

Setup a `BBTree` of depth `d` like so:

    size_t size = BBTree::MemorySize(d);
    // here, allocate a buffer `b` of size `size`
    ANAlloc::BBTree tree(d, b);

Now, you may attach the `tree` to a new allocator:

    ANAlloc::Allocator<ANAlloc::BBTree> alloc(&tree);

While you use the `alloc` allocator, `tree` must remain in scope. But how do you use `alloc`? Here's how:

    ANAlloc::Path p;
    bool result = alloc.Alloc(aDepth, p);

If `result` is true, then `p` now points to a path in the binary tree which has been marked as data. The allocator automatically creates containers recursively, splitting large buddies into smaller buddies.

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
