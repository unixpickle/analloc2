library placed_free_list;

import 'dart:math';

int maxNesting = 0;

class OutputBuffer {
  String fullContents = '';
  int lineLength = 0;
  
  void add(String str) {
    if (lineLength == 0) {
      fullContents += '    ' + str + ',';
      lineLength = str.length + 5;
    } else if (lineLength + 2 + str.length > 79) {
      fullContents += '\n';
      lineLength = 0;
      add(str);
    } else {
      fullContents = fullContents + ' ' + str + ',';
      lineLength += 2 + str.length;
    }
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
  
  int offsetAlign(int align, int alignOffset, int size) {
    for (var region in regions) {
      int misalign = (region.start + alignOffset) % align;
      int offset = 0;
      if (misalign != 0) {
        offset = align - misalign;
      }
      int result = null;
      if (region.size - offset < size) {
        continue;
      } else if (offset == 0 && size == region.size) {
        result = region.start;
        regions.remove(region);
        stack.add(region.address);
      } else if (offset == 0 && size < region.size) {
        result = region.start;
        region.start += size;
        region.size -= size;
      } else if (offset != 0 && size + offset == region.size) {
        result = region.start + offset;
        region.size = offset;
      } else {
        result = region.start + offset;
        int regionAddr = stack.last;
        stack.removeLast();
        var newRegion = new FreeRegion(regionAddr, region.start + offset +
            size, region.size - (offset + size));
        region.size = offset;
        regions.insert(regions.indexOf(region) + 1, newRegion);
      }
      applyBuffer();
      return result;
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
    int nestCount = 0;
    while (stack.length == capacity) {
      int last = stack.last;
      stack.removeLast();
      dealloc(last, 1);
      if (nestCount > maxNesting) {
        maxNesting = nestCount;
      }
      ++nestCount;
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