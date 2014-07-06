CPPFLAGS = -Wextra -Wall -std=c++11

all: build/bitmap build/tree build/btree build/bbtree build/allocator build/topology build/malloc build/profile_bbtree_o0 build/profile_bbtree_o2

build/bitmap: build
	g++ $(CPPFLAGS) ../src/bitmap.cpp test-bitmap.cpp -o build/bitmap

build/path: build
	g++ $(CPPFLAGS) test-path.cpp -o build/path

build/btree: build
	g++ $(CPPFLAGS) ../src/bitmap.cpp ../src/btree.cpp tree-tester.cpp test-btree.cpp -o build/btree

build/bbtree: build
	g++ $(CPPFLAGS) ../src/bitmap.cpp ../src/tree.cpp ../src/bbtree.cpp test_bbtree.cpp -o build/bbtree

build/allocator: build
	g++ $(CPPFLAGS) ../src/bitmap.cpp ../src/tree.cpp ../src/btree.cpp ../src/bbtree.cpp test_allocator.cpp -o build/allocator

build/topology: build
	g++ $(CPPFLAGS) ../src/bitmap.cpp ../src/tree.cpp ../src/btree.cpp ../src/bbtree.cpp ../src/topology/*.cpp test_topology.cpp -o build/topology

build/malloc: build
	g++ $(CPPFLAGS) ../src/bitmap.cpp ../src/tree.cpp ../src/bbtree.cpp ../src/topology/*.cpp test_malloc.cpp -o build/malloc

build/profile_bbtree_o0: build
	g++ -O0 $(CPPFLAGS) ../src/bitmap.cpp ../src/tree.cpp ../src/bbtree.cpp ../src/btree.cpp profile_tree.cpp -o build/profile_bbtree_o0

build/profile_bbtree_o2: build
	g++ -O2 $(CPPFLAGS) ../src/bitmap.cpp ../src/tree.cpp ../src/bbtree.cpp ../src/btree.cpp profile_tree.cpp -o build/profile_bbtree_o2

build:
	mkdir build

clean:
	rm -rf build
