CPPFLAGS = -Wextra -Wall -std=c++11

all: build/bitmap build/tree build/btree build/bbtree build/allocator build/topology build/malloc build/profile_bbtree_o0 build/profile_bbtree_o2 build/profile_allocator_o0 build/profile_allocator_o2 build/profile_malloc_o0 build/profile_malloc_o2

build/bitmap: build
	g++ $(CPPFLAGS) ../src/tree/bitmap.cpp test-bitmap.cpp -o build/bitmap

build/path: build
	g++ $(CPPFLAGS) test-path.cpp -o build/path

build/btree: build
	g++ $(CPPFLAGS) ../src/tree/tree.cpp ../src/tree/bitmap.cpp ../src/tree/btree.cpp tree-tester.cpp test-btree.cpp -o build/btree

build/bbtree: build
	g++ $(CPPFLAGS) ../src/tree/tree.cpp ../src/tree/bitmap.cpp ../src/tree/bbtree.cpp tree-tester.cpp test-bbtree.cpp -o build/bbtree

build/malloc: build
	g++ $(CPPFLAGS) ../src/tree/bitmap.cpp ../src/tree/tree.cpp ../src/tree/bbtree.cpp ../src/tree/btree.cpp ../src/log.cpp ../src/malloc.cpp test-malloc.cpp -o build/malloc

build/allocator: build
	g++ $(CPPFLAGS) ../src/bitmap.cpp ../src/tree.cpp ../src/btree.cpp ../src/bbtree.cpp test_allocator.cpp -o build/allocator

build/topology: build
	g++ $(CPPFLAGS) ../src/bitmap.cpp ../src/tree.cpp ../src/btree.cpp ../src/bbtree.cpp ../src/topology/*.cpp test_topology.cpp -o build/topology

build/prof-tree-o0: build
	g++ -O0 $(CPPFLAGS) ../src/tree/tree.cpp ../src/tree/bitmap.cpp ../src/tree/bbtree.cpp ../src/tree/btree.cpp profile-tree.cpp -o build/prof-tree-o0

build/prof-tree-o2: build
	g++ -O2 $(CPPFLAGS) ../src/tree/tree.cpp ../src/tree/bitmap.cpp ../src/tree/bbtree.cpp ../src/tree/btree.cpp profile-tree.cpp -o build/prof-tree-o2

build/profile_allocator_o0: build
	g++ -O0 $(CPPFLAGS) ../src/bitmap.cc ../src/tree.cc ../src/bbtree.cc ../src/btree.cc profile_allocator.cc -o build/profile_allocator_o0

build/profile_allocator_o2: build
	g++ -O2 $(CPPFLAGS) ../src/bitmap.cc ../src/tree.cc ../src/bbtree.cc ../src/btree.cc profile_allocator.cc -o build/profile_allocator_o2

build/profile_malloc_o0: build
	g++ -O0 $(CPPFLAGS) ../src/bitmap.cc ../src/tree.cc ../src/bbtree.cc ../src/btree.cc profile_malloc.cc -o build/profile_malloc_o0

build/profile_malloc_o2: build
	g++ -O2 $(CPPFLAGS) ../src/bitmap.cc ../src/tree.cc ../src/bbtree.cc ../src/btree.cc profile_malloc.cc -o build/profile_malloc_o2

build:
	mkdir build

clean:
	rm -rf build
