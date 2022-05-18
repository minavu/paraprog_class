// Oddven sort (data parallel version)
//
use Random;
config const DEBUG = true;
config const WORST = false;
config const N = 10;

// Initialize array with random uint(8) values
// (except if WORST flag is on, set array to the reverse of 1..N)
proc init_array(a:[]) {
  if WORST then {
    forall i in 1..N do
      a[i] = N:uint(8) - i:uint(8) + 1;
  } else {
    forall i in 1..N {
      var rs = new RandomStream(uint(8));
      a[i] = rs.getNext() % 255:uint(8);
    }
  }

  if DEBUG then
    writeln("Init: ", a); 
}

// Verify that array is sorted
// (if not sorted, show the violation pair of elements)
proc verify_array(a:[]) {
  for i in 1..N-1 do
    if (a[i] > a[i+1]) {
      writef("FAILED: a[%i]=%i, a[%i]=%i\n", i, a[i], i+1, a[i+1]);
      return;
    }
  writeln(N, " element array is sorted.");  
}

// Oddeven sort
// 
proc oddeven_sort(a:[]) {
  for t in 1..(N+1)/2 {
    forall i in 1..N-1 by 2 do
      if (a[i] > a[i+1]) then
        a[i] <=> a[i+1];
    forall j in 2..N-1 by 2 do
      if (a[j] > a[j+1]) then
        a[j] <=> a[j+1];
    if DEBUG {
      writef("t=%-2i: ", t);
      writeln(a);
    }
  }
}

proc main() {
  var a: [1..N] uint(8);
  init_array(a);
  oddeven_sort(a);
  verify_array(a);
}
