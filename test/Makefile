CPPFLAGS ?= -Wextra -Wall -std=c++11

all: build/bitmap build/path build/btree build/bbtree build/allocator build/malloc build/layout build/prof-tree-o0 build/prof-tree-o2 build/prof-malloc-o0 build/prof-malloc-o2

build/bitmap: build
	g++ $(CPPFLAGS) ../src/tree/bitmap.cpp test-bitmap.cpp -o build/bitmap

build/path: build
	g++ $(CPPFLAGS) test-path.cpp -o build/path

build/btree: build
	g++ $(CPPFLAGS) ../src/tree/tree.cpp ../src/tree/bitmap.cpp ../src/tree/btree.cpp tree-tester.cpp test-btree.cpp -o build/btree

build/bbtree: build
	g++ $(CPPFLAGS) ../src/tree/tree.cpp ../src/tree/bitmap.cpp ../src/tree/bbtree.cpp tree-tester.cpp test-bbtree.cpp -o build/bbtree

build/allocator: build
	g++ $(CPPFLAGS) ../src/tree/bitmap.cpp ../src/tree/tree.cpp ../src/tree/bbtree.cpp ../src/tree/btree.cpp ../src/log.cpp ../src/allocator.cpp test-allocator.cpp -o build/allocator

build/malloc: build
	g++ $(CPPFLAGS) ../src/tree/bitmap.cpp ../src/tree/tree.cpp ../src/tree/bbtree.cpp ../src/tree/btree.cpp ../src/log.cpp ../src/allocator.cpp ../src/malloc.cpp test-malloc.cpp -o build/malloc

build/layout: build
	g++ $(CPPFLAGS) ../src/topology/layout.cpp test-layout.cpp -o build/layout

build/cluster: build
	g++ $(CPPFLAGS) ../src/tree/bitmap.cpp ../src/tree/tree.cpp ../src/tree/bbtree.cpp ../src/tree/btree.cpp ../src/log.cpp ../src/allocator.cpp ../src/cluster/cluster.cpp test-cluster.cpp -o build/cluster

build/prof-tree-o0: build
	g++ -O0 $(CPPFLAGS) ../src/tree/tree.cpp ../src/tree/bitmap.cpp ../src/tree/bbtree.cpp ../src/tree/btree.cpp profile-tree.cpp -o build/prof-tree-o0

build/prof-tree-o2: build
	g++ -O2 $(CPPFLAGS) ../src/tree/tree.cpp ../src/tree/bitmap.cpp ../src/tree/bbtree.cpp ../src/tree/btree.cpp profile-tree.cpp -o build/prof-tree-o2
	
build/prof-malloc-o0: build
	g++ -O0 $(CPPFLAGS) ../src/tree/tree.cpp ../src/tree/bitmap.cpp ../src/tree/bbtree.cpp ../src/tree/btree.cpp ../src/allocator.cpp ../src/malloc.cpp ../src/log.cpp profile-malloc.cpp -o build/prof-malloc-o0

build/prof-malloc-o2: build
	g++ -O2 $(CPPFLAGS) ../src/tree/tree.cpp ../src/tree/bitmap.cpp ../src/tree/bbtree.cpp ../src/tree/btree.cpp ../src/allocator.cpp ../src/malloc.cpp ../src/log.cpp profile-malloc.cpp -o build/prof-malloc-o2

build:
	mkdir build

clean:
	rm -rf build
