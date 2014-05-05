all: build/bitmap build/tree build/btree build/bbtree build/allocator build/topology

build/bitmap: build
	g++ -Wall ../src/bitmap.cc test_bitmap.cc -o build/bitmap

build/tree: build
	g++ -Wall ../src/tree.cc test_tree.cc -o build/tree

build/btree: build
	g++ -Wall ../src/bitmap.cc ../src/tree.cc ../src/btree.cc test_btree.cc -o build/btree

build/bbtree: build
	g++ -Wall ../src/bitmap.cc ../src/tree.cc ../src/bbtree.cc test_bbtree.cc -o build/bbtree

build/allocator: build
	g++ -Wall ../src/bitmap.cc ../src/tree.cc ../src/btree.cc ../src/bbtree.cc test_allocator.cc -o build/allocator

build/topology: build
	g++ -Wall ../src/bitmap.cc ../src/tree.cc ../src/btree.cc ../src/bbtree.cc test_topology.cc -o build/topology

build:
	mkdir build

clean:
	rm -rf build