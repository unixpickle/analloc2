import 'dart:math';
import 'placed_free_list.dart';

PlacedFreeList freeList;
Random randomizer;
OutputBuffer output;

void main() {
  print('This may take some time. Theoretically, it could take forever.');
  while (!generateWithMaxNesting(3)) {
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
    if (nextRandomIsAllocation()) {
      performAllocation();
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
  output.add("{1, $address, $size, ${freeList.stack.length}}");
}

void performDeallocation() {
  var regions = freeList.computeAllocatedRegions();
  var choice = regions[randomizer.nextInt(regions.length)];
  var offset = randomizer.nextInt(choice.size);
  var size = randomizer.nextInt(choice.size - offset) + 1;
  var address = choice.start + offset;
  freeList.dealloc(address, size);
  output.add("{-1, $address, $size, ${freeList.stack.length}}");
}
