# BBTree

The BBTree data structure is an efficient way to represent a binary tree in memory. I designed it specifically for buddy memory allocation. This is implemented by `ANAlloc::BBTree`, which I recommend you use as the template argument to pretty much any `ANAlloc` class that takes one. The alternative, `ANAlloc::BTree` is optimal in storage, but has `O(n)` allocation time in the worst case.

## TL;DR

The BBTree algorithm uses approximately `1.632843` bits of memory for each "node" in the binary tree. This means that for a binary tree of depth D, the BBTree will use about `1.632843 * (2^D - 1)` bits of memory. Of course, this is `O(n)`, as would be expected.

Searching for a free node in a BBTree takes `O(log(n))`, and allocation also takes `O(log(n))`.

## Structure Overview

A BBTree holds a series of nodes. There is a root node, and then each node underneath it has two children up until the *base nodes* (nodes at the lowest level of the tree and thus with no children). Each node contains the size of the largest free subnode (including itself). So, a completely free node would contain its own size. A node with one free child and one allocated child would contain the size of its children. A node that is allocated contains a size of 0.

### Procedures

With this structure, a lookup is quick: if you are looking for a free node of at least size `n`, you depth first search. If you hit a node which contains a size smaller than `n`, you stop exploring that path. Thus, finding a free node takes `O(log(n))` time because you will never fruitlessly explore a path.

Allocation of a node is likewise quite simple. Once you have a node that's the size you want, you set it to size 0. Now, you unwind the stack and regenerate each node's size based on the maximum of the two sizes stored in its chidlren. Since all you are doing is unwinding the stack, this is `O(log(n))` as well.

Freeing a node is similar to allocation. This time, you set the node to contain its own size. Now, you unwind the stack and update each parent node's size based on its two children. If both children are free, you combine the two sizes and the parent becomes free. This is `O(log(n))` for the same reason allocation is.

### Naive Representation

You could represent the BBTree structure as a linear array of integer types. These integer types would need to be large enough to store the maximum size in the tree. Note, however, that this is a complete waste: when your maximum node size is `n`, you only need to store a total of `log2(n)` different sizes (since each node is half the size of its parent). Furthermore, base nodes can only contain two sizes, and their parents can only contain 3 sizes, etc., so even `log2(n)` sizes per node is sub-optimal.

### The Optimal Representation

Suppose you have a tree of depth `D` (a tree with one node is considered a tree of depth 1, a tree with 3 nodes is depth 2, etc.). For a node at a given depth `d` (starting at 0 for the root), there are only `D - d + 1` possible sizes for that node (including the 0 size), and thus you only need `ceil(log2(D - d + 1))` bits for that node. Thus, for root nodes you only need 1 bit, and for their parents the number of required bits increases logarithmically.

If you sit down and write a program to calculate the asymptotic average number of bits per node, you will find that it is approximately `1.6328430180437862874161594750`.
