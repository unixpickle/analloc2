all: build/bitmap build/tree build/btree

build/bitmap: build
	g++ ../src/bitmap.cc test_bitmap.cc -o build/bitmap

build/tree: build
	g++ ../src/tree.cc test_tree.cc -o build/tree

build/btree: build
	g++ ../src/bitmap.cc ../src/tree.cc ../src/btree.cc test_btree.cc -o build/btree

build:
	mkdir build

clean:
	rm -rf build