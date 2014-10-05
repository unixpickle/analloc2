/**
 * Generate a set of random tree operations for testing any tree
 * implementation.
 */

import 'dart:io';
import 'dart:math';

var inside = [];
var outside = [];
var r = new Random();

void main() {
  for (int i = 1; i <= 100; ++i) {
    outside.add(i);
  }

  // Build up a decently sized tree
  for (int i = 0; i < 30; ++i) {
    doInsertion();
  }
  // Tear down the tree
  for (int i = 0; i < 30; ++i) {
    doDeletion();
  }

  // Perform random insertions and deletions
  for (int i = 0; i < 30; ++i) {
    if (inside.length == 0 || (r.nextBool() && outside.length != 0)) {
      // Do a set of insertions
      int insertCount = r.nextInt(outside.length);
      for (int j = 0; j < insertCount; ++j) {
        doInsertion();
      }
    } else {
      // Do a set of deletions
      int deleteCount = r.nextInt(inside.length);
      for (int j = 0; j < deleteCount; ++j) {
        doDeletion();
      }
    }
  }
  
  stdout..writeln('')
        ..writeln('inside are $inside')
        ..writeln('outside are $outside');
}

void doInsertion() {
  assert(outside.length != 0);
  int idx = r.nextInt(outside.length);
  stdout.write('${outside[idx]}, ');
  inside.add(outside[idx]);
  outside.removeAt(idx);
}

void doDeletion() {
  assert(inside.length != 0);
  int idx = r.nextInt(inside.length);
  stdout.write('-${inside[idx]}, ');
  outside.add(inside[idx]);
  inside.removeAt(idx);
}

