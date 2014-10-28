export EXTRA_FLAGS=-I../include -I../dependencies/ansa/include
export OBJECTS=build/objects/*.o
export CXXFLAGS=-std=c++11 -Wall -Wextra

SOURCES=$(wildcard *.cpp)
PRODUCTS=$(SOURCES:%.cpp=build/%)

.PHONY: all clean clean-all

all: $(PRODUCTS)

build/objects: build
	anbuild --output build --deps ../dependencies ../build.dart
	make -C build

build/profile%: profile%.cpp build/objects
	$(CXX) $(CXXFLAGS) $(EXTRA_FLAGS) $(OBJECTS) -O3 $< -o $@

build/test%: test%.cpp build/objects
	$(CXX) $(CXXFLAGS) $(EXTRA_FLAGS) $(OBJECTS) $< -o $@

build:
	mkdir build

clean:
	$(RM) -r build

clean-all: clean
	$(RM) -r ../dependencies
