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
