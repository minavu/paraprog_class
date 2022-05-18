// A producer-consumer program using the cqueue module 
//
// Version 2: Extended version: single producer and multiple consumers
//

use cqueue;

config const numItems = 32;
config const numCons = 2;

var item$: sync int = 1;

proc producer() { 
  for i in 1..numItems {
    var idx = cqueue.add(i);
    writef("Producer added %i to buf[%i]\n", i, idx);
  }
}

proc consumer(k) { 
  while true {
    var i = item$.readFE();
    item$.writeEF(i + 1);
    if (i <= numItems) {
      var pair = cqueue.remove();
      writef("Consumer[%i] rem'd %i from buf[%i]\n", k, pair[1], pair[0]);
    } else {
      break;
    }
  }
}

proc main() {
  cobegin {
    producer();
    coforall k in 1..numCons do
      consumer(k);
  }
} 
