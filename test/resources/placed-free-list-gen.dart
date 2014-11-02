import 'dart:math';

PlacedFreeList freeList;
Random randomizer;
String currentLine = '';

void main() {
  // Configuration for randomized test.
  var totalSize = 0x100;
  var operationCount = 0x400;
  var capacity = 4;
  
  freeList = new PlacedFreeList(capacity, totalSize);
  randomizer = new Random();
  
  // Perform random operations
  for (var i = 0; i < operationCount; ++i) {
    if (nextRandomIsAllocation()) {
      performAllocation();
    } else {
      performDeallocation();
    }
  }
  print(currentLine);
}

bool nextRandomIsAllocation() {
  if (freeList.computeLargestFreeRegion() == 0) {
    return false;
  } else if (freeList.computeAllocatedRegions().length == 0) {
    return true;
  } else {
    return randomizer.nextBool();
  }
}

void performAllocation() {
  var size = randomizer.nextInt(freeList.computeLargestFreeRegion()) + 1;
  var address = freeList.alloc(size);
  assert(address != null);
  printNext("{1, $address, $size, ${freeList.stack.length}}");
}

void performDeallocation() {
  var regions = freeList.computeAllocatedRegions();
  var choice = regions[randomizer.nextInt(regions.length)];
  var offset = randomizer.nextInt(choice.size);
  var size = randomizer.nextInt(choice.size - offset) + 1;
  var address = choice.start + offset;
  freeList.dealloc(address, size);
  printNext("{-1, $address, $size, ${freeList.stack.length}}");
}

void printNext(String line) {
  if (currentLine.length == 0) {
    currentLine = '    ' + line + ',';
  } else if (currentLine.length + 2 + line.length > 79) {
    print(currentLine);
    currentLine = '';
    printNext(line);
  } else {
    currentLine = currentLine + ' ' + line + ',';
  }
}

class FreeRegion {
  final int address;
  int start;
  int size;
  
  int get end => start + size;
  
  FreeRegion(this.address, this.start, this.size);
}

class AllocatedRegion {
  int start;
  int size = 0;
  
  AllocatedRegion(this.start);
}

class PlacedFreeList {
  final int totalSize;
  final int capacity;
  final List<FreeRegion> regions = [];
  final List<int> stack = [];
  bool applyingBuffer = false;
  
  PlacedFreeList(this.capacity, this.totalSize) {
    assert(totalSize > 2);
    stack.add(0);
    regions.add(new FreeRegion(1, 2, totalSize - 2));
  }
  
  int alloc(int size) {
    for (var region in regions) {
      if (region.size >= size) {
        int result = region.start;
        if (region.size == size) {
          regions.remove(region);
          stack.add(region.address);
        } else {
          region.start += size;
          region.size -= size;
        }
        applyBuffer();
        return result;
      }
    }
    return null;
  }
  
  void dealloc(int start, int size) {
    var afterIndex = regions.length;
    for (var i = 0; i < regions.length; ++i) {
      if (regions[i].start > start) {
        afterIndex = i;
        break;
      }
    }
    if (afterIndex > 0 && regions[afterIndex - 1].end == start) {
      // Touches the previous region
      regions[afterIndex - 1].size += size;
      if (afterIndex < regions.length &&
          regions[afterIndex].start == regions[afterIndex - 1].end) {
        // Touches the next region
        regions[afterIndex - 1].size += regions[afterIndex].size;
        stack.add(regions[afterIndex].address);
        regions.removeAt(afterIndex);
      }
    } else if (afterIndex < regions.length &&
               regions[afterIndex].start == start + size) {
      // Touches the next region
      regions[afterIndex].start -= size;
      regions[afterIndex].size += size;
    } else {
      // Insert a free-floating region
      int regionAddr = stack.last;
      stack.removeLast();
      regions.insert(afterIndex, new FreeRegion(regionAddr, start, size));
    }
    applyBuffer();
  }
  
  void applyBuffer() {
    if (applyingBuffer) {
      return;
    }
    applyingBuffer = true;
    while (stack.length < 1) {
      int next = alloc(1);
      if (next == null) {
        throw new StateError('failed to allocate region');
      }
      stack.add(next);
    }
    while (stack.length == capacity) {
      int last = stack.last;
      stack.removeLast();
      dealloc(last, 1);
    }
    applyingBuffer = false;
  }
  
  List<AllocatedRegion> computeAllocatedRegions() {
    var result = [];
    var current = null;
    for (var i = 0; i < totalSize; ++i) {
      if (isAllocated(i)) {
        if (current == null) {
          current = new AllocatedRegion(i);
        }
        ++current.size;
      } else if (current != null) {
        result.add(current);
        current = null;
      }
    }
    if (current != null) {
      result.add(current);
    }
    return result;
  }
  
  bool isAllocated(int idx) {
    for (var region in regions) {
      if (region.address == idx) {
        return false;
      }
      if (idx >= region.start && idx < region.end) {
        return false;
      }
    }
    return !stack.contains(idx);
  }
  
  int computeLargestFreeRegion() {
    int result = 0;
    for (var region in regions) {
      result = max(region.size, result);
    }
    return result;
  }
}
