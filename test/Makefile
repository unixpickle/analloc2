all: build/test-bitmap

export EXTRA_FLAGS=-std=c++11 -I../include -I../dependencies/ansa/include
export OBJECTS=build/objects/*.o

build/test-bitmap: build/objects
	$(CXX) $(CXXFLAGS) $(EXTRA_FLAGS) $(OBJECTS) test-bitmap.cpp -o build/test-bitmap

build/objects: build
	anbuild --output build ../build.dart
	make -C build

build:
	mkdir build

clean:
	$(RM) -r build
	$(RM) -r ../dependencies
