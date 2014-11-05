import 'dart:math';
import 'placed_free_list.dart';

PlacedFreeList freeList;
Random randomizer;
OutputBuffer output;

void main() {
  print('This may take some time. Theoretically, it could take forever.');
  while (!generateWithMaxNesting(1)) {
  }
}

bool generateWithMaxNesting(int requiredNesting) {
  // Configuration for randomized test
  var totalSize = 0x100;
  var operationCount = 0x400;
  var capacity = 4;
  
  // Setup global environment
  maxNesting = 0;
  freeList = new PlacedFreeList(capacity, totalSize);
  randomizer = new Random();
  output = new OutputBuffer();
  
  // Perform random operations
  for (var i = 0; i < operationCount; ++i) {
    if (nextRandomIsAlignment()) {
      performAlignment();
    } else {
      performDeallocation();
    }
  }
  if (maxNesting >= requiredNesting) {
    print(output.fullContents);
    return true;
  } else {
    return false;
  }
}

bool nextRandomIsAlignment() {
  if (freeList.computeLargestFreeRegion() == 0) {
    return false;
  } else if (freeList.computeAllocatedRegions().length == 0) {
    return true;
  } else {
    return randomizer.nextBool();
  }
}

void performAlignment() {
  // just keep trying to align something until it works.
  while (true) {
    int align = 1 << randomizer.nextInt(8);
    int offset = randomizer.nextInt(freeList.totalSize);
    int size = randomizer.nextInt(freeList.totalSize);
    var address = freeList.offsetAlign(align, offset, 1);
    if (address != null) {
      output.add("{1, $address, $align, $offset, $size,"
          " ${freeList.stack.length}}");
      return;
    }
  }
}

void performDeallocation() {
  var regions = freeList.computeAllocatedRegions();
  var choice = regions[randomizer.nextInt(regions.length)];
  var offset = randomizer.nextInt(choice.size);
  var size = randomizer.nextInt(choice.size - offset) + 1;
  var address = choice.start + offset;
  freeList.dealloc(address, size);
  output.add("{-1, $address, 0, 0, $size, ${freeList.stack.length}}");
}
