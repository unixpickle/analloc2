CPPFLAGS = -Wextra -Wall -std=c++11

all: build/bitmap build/tree build/btree build/bbtree build/allocator build/topology build/malloc build/profile_bbtree_o0 build/profile_bbtree_o2

build/bitmap: build
	g++ $(CPPFLAGS) ../src/bitmap.cc test_bitmap.cc -o build/bitmap

build/tree: build
	g++ $(CPPFLAGS) ../src/tree.cc test_tree.cc -o build/tree

build/btree: build
	g++ $(CPPFLAGS) ../src/bitmap.cc ../src/tree.cc ../src/btree.cc test_btree.cc -o build/btree

build/bbtree: build
	g++ $(CPPFLAGS) ../src/bitmap.cc ../src/tree.cc ../src/bbtree.cc test_bbtree.cc -o build/bbtree

build/allocator: build
	g++ $(CPPFLAGS) ../src/bitmap.cc ../src/tree.cc ../src/btree.cc ../src/bbtree.cc test_allocator.cc -o build/allocator

build/topology: build
	g++ $(CPPFLAGS) ../src/bitmap.cc ../src/tree.cc ../src/btree.cc ../src/bbtree.cc ../src/topology/*.cc test_topology.cc -o build/topology

build/malloc: build
	g++ $(CPPFLAGS) ../src/bitmap.cc ../src/tree.cc ../src/bbtree.cc ../src/topology/*.cc test_malloc.cc -o build/malloc

build/profile_bbtree_o0: build
	g++ -O0 $(CPPFLAGS) ../src/bitmap.cc ../src/tree.cc ../src/bbtree.cc ../src/btree.cc profile_tree.cc -o build/profile_bbtree_o0

build/profile_bbtree_o2: build
	g++ -O2 $(CPPFLAGS) ../src/bitmap.cc ../src/tree.cc ../src/bbtree.cc ../src/btree.cc profile_tree.cc -o build/profile_bbtree_o2

build:
	mkdir build

clean:
	rm -rf build
