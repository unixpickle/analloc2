# Another Allocator?

I hope this will be the last pure Buddy Allocator that I implement. I am trying really hard to abstract as much as I can so that this is modular and I can improve on it in the future.

# Usage

You may see the usage for the basic malloc interface in [docs/malloc.md](docs/malloc.md). In addition, you may wish to read the explanation of the BBTree algorithm at [docs/bbtree.md](docs/bbtree.md). Soon, I will write up some docs on the topology API&ndash;stay tuned.

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

# TODO

My current concerns about this library are usability and performance.

Performance: I have found that allocating memory on a typical x86-64 system usually takes upwards of 3,000 clock-cycles. I think this is rather high, considering that a linked-list implementation would only take a few hundred cycles.

Usability: Integer types are a major headache in any C-based language, and they plague analloc2. On a 32-bit system, usually it will make sense to use 32-bit integers for a virtual memory allocator. However, on x86 with PAE, you will probably want 64-bit integers for physical memory allocation. Because of situations like these, I think it might be best to make the integer type a template argument.

By making integer types a template argument, I will be forcing myself to implement everything in headers. This will allow for better optimization and potentially remove the need for virtual methods. However, I am not sure if I really *want* to do such a thing.