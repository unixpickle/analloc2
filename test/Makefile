all: build/bitmap

build/bitmap: build
	g++ ../src/bitmap.cc test_bitmap.cc -o build/bitmap

build:
	mkdir build

clean:
	rm -rf build