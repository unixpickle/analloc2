all: build/test-bitmap

export EXTRA_FLAGS=-I../include -I../dependencies/ansa/include
export OBJECTS=build/objects/*.o
export CXXFLAGS=-std=c++11 -Wall -Wextra

all: build/test-bitmap build/test-bitmap-allocator build/profile-bitmap-allocator-o0 build/profile-bitmap-allocator-o3 build/test-bitmap-aligner

build/test-bitmap: build/objects
	$(CXX) $(CXXFLAGS) $(EXTRA_FLAGS) $(OBJECTS) test-bitmap.cpp -o build/test-bitmap

build/test-bitmap-allocator: build/objects
	$(CXX) $(CXXFLAGS) $(EXTRA_FLAGS) $(OBJECTS) test-bitmap-allocator.cpp -o build/test-bitmap-allocator

build/profile-bitmap-allocator-o0: build/objects
	$(CXX) $(CXXFLAGS) $(EXTRA_FLAGS) $(OBJECTS) profile-bitmap-allocator.cpp -o build/profile-bitmap-allocator-o0

build/profile-bitmap-allocator-o3: build/objects
	$(CXX) $(CXXFLAGS) $(EXTRA_FLAGS) $(OBJECTS) -O3 profile-bitmap-allocator.cpp -o build/profile-bitmap-allocator-o3

build/test-bitmap-aligner: build/objects
	$(CXX) $(CXXFLAGS) $(EXTRA_FLAGS) $(OBJECTS) test-bitmap-aligner.cpp -o build/test-bitmap-aligner

build/objects: build
	anbuild --output build ../build.dart
	make -C build

build:
	mkdir build

clean:
	$(RM) -r build
	$(RM) -r ../dependencies
