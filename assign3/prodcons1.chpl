// A producer-consumer program using the cqueue module 
//
// Version 1: Base version: single producer and single consumer
//

use cqueue;

config const numItems = 32;

proc producer() { 
  for i in 1..numItems {
    var idx = cqueue.add(i);
    writef("Producer added %i to buf[%i]\n", i, idx);
  }
}

proc consumer() { 
  for i in 1..numItems {
    var pair = cqueue.remove();
    writef("Consumer rem'd %i from buf[%i]\n", pair[1], pair[0]);
  }
}

proc main() {
  cobegin {
    producer();
    consumer();
  }
} 
