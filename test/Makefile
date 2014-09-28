all: build/test-bitmap

export EXTRA_FLAGS=-I../include -I../dependencies/ansa/include
export OBJECTS=build/objects/*.o
export CXXFLAGS=-std=c++11 -Wall -Wextra

all: build/test-bitmap build/test-bitmap-allocator

build/test-bitmap: build/objects
	$(CXX) $(CXXFLAGS) $(EXTRA_FLAGS) $(OBJECTS) test-bitmap.cpp -o build/test-bitmap

build/test-bitmap-allocator: build/objects
	$(CXX) $(CXXFLAGS) $(EXTRA_FLAGS) $(OBJECTS) test-bitmap-allocator.cpp -o build/test-bitmap-allocator

build/objects: build
	anbuild --output build ../build.dart
	make -C build

build:
	mkdir build

clean:
	$(RM) -r build
	$(RM) -r ../dependencies
