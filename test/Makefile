all: build/test-bitmap

export EXTRA_FLAGS=-I../include -I../dependencies/ansa/include
export OBJECTS=build/objects/*.o
export CXXFLAGS=-std=c++11 -Wall -Wextra

all: build/test-bitmap build/test-bitmap-allocator build/profile-bitmap-allocator build/test-bitmap-aligner build/test-free-list-allocator build/test-free-list-aligner build/profile-free-list-allocator build/test-avl-tree build/profile-avl-tree build/test-free-tree-allocator build/test-free-tree-aligner build/profile-free-tree-allocator build/test-transformed-bitmap-allocator build/test-virtual-bitmap-allocator build/test-transformed-bitmap-aligner build/test-virtual-bitmap-aligner build/profile-virtual-bitmap-aligner

build/test-bitmap: build/objects
	$(CXX) $(CXXFLAGS) $(EXTRA_FLAGS) $(OBJECTS) test-bitmap.cpp -o build/test-bitmap

build/test-bitmap-allocator: build/objects
	$(CXX) $(CXXFLAGS) $(EXTRA_FLAGS) $(OBJECTS) test-bitmap-allocator.cpp -o build/test-bitmap-allocator

build/profile-bitmap-allocator: build/objects
	$(CXX) $(CXXFLAGS) $(EXTRA_FLAGS) $(OBJECTS) -O3 profile-bitmap-allocator.cpp -o build/profile-bitmap-allocator

build/test-bitmap-aligner: build/objects
	$(CXX) $(CXXFLAGS) $(EXTRA_FLAGS) $(OBJECTS) test-bitmap-aligner.cpp -o build/test-bitmap-aligner

build/test-free-list-allocator: build/objects
	$(CXX) $(CXXFLAGS) $(EXTRA_FLAGS) $(OBJECTS) test-free-list-allocator.cpp posix-virtual-aligner.cpp -o build/test-free-list-allocator

build/test-free-list-aligner: build/objects
	$(CXX) $(CXXFLAGS) $(EXTRA_FLAGS) $(OBJECTS) test-free-list-aligner.cpp posix-virtual-aligner.cpp -o build/test-free-list-aligner

build/profile-free-list-allocator: build/objects
	$(CXX) $(CXXFLAGS) $(EXTRA_FLAGS) $(OBJECTS) -O3 posix-virtual-aligner.cpp profile-free-list-allocator.cpp -o build/profile-free-list-allocator

build/test-avl-tree: build/objects
	$(CXX) $(CXXFLAGS) $(EXTRA_FLAGS) $(OBJECTS) test-avl-tree.cpp posix-virtual-aligner.cpp -o build/test-avl-tree

build/profile-avl-tree: build/objects
	$(CXX) $(CXXFLAGS) $(EXTRA_FLAGS) $(OBJECTS) -O3 profile-avl-tree.cpp posix-virtual-aligner.cpp -o build/profile-avl-tree

build/test-free-tree-allocator: build/objects
	$(CXX) $(CXXFLAGS) $(EXTRA_FLAGS) $(OBJECTS) test-free-tree-allocator.cpp posix-virtual-aligner.cpp -o build/test-free-tree-allocator

build/test-free-tree-aligner: build/objects
	$(CXX) $(CXXFLAGS) $(EXTRA_FLAGS) $(OBJECTS) test-free-tree-aligner.cpp posix-virtual-aligner.cpp -o build/test-free-tree-aligner

build/profile-free-tree-allocator: build/objects
	$(CXX) $(CXXFLAGS) $(EXTRA_FLAGS) $(OBJECTS) -O3 posix-virtual-aligner.cpp profile-free-tree-allocator.cpp -o build/profile-free-tree-allocator

build/test-transformed-bitmap-allocator: build/objects
	$(CXX) $(CXXFLAGS) $(EXTRA_FLAGS) $(OBJECTS) test-transformed-bitmap-allocator.cpp -o build/test-transformed-bitmap-allocator

build/test-virtual-bitmap-allocator: build/objects
	$(CXX) $(CXXFLAGS) $(EXTRA_FLAGS) $(OBJECTS) test-virtual-bitmap-allocator.cpp -o build/test-virtual-bitmap-allocator

build/test-transformed-bitmap-aligner: build/objects
	$(CXX) $(CXXFLAGS) $(EXTRA_FLAGS) $(OBJECTS) test-transformed-bitmap-aligner.cpp -o build/test-transformed-bitmap-aligner

build/test-virtual-bitmap-aligner: build/objects
	$(CXX) $(CXXFLAGS) $(EXTRA_FLAGS) $(OBJECTS) test-virtual-bitmap-aligner.cpp -o build/test-virtual-bitmap-aligner

build/profile-virtual-bitmap-aligner: build/objects
	$(CXX) $(CXXFLAGS) $(EXTRA_FLAGS) $(OBJECTS) -O3 profile-virtual-bitmap-aligner.cpp -o build/profile-virtual-bitmap-aligner

build/objects: build
	anbuild --output build ../build.dart
	make -C build

build:
	mkdir build

clean:
	$(RM) -r build

clean-all: clean
	$(RM) -r ../dependencies

