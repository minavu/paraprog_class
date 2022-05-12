// A producer-consumer program using the cqueue module 
//
// Version 1: Base version: single producer and single consumer
//

use cqueue;

config const numItems = 32;

proc producer() { 
  // ... fill ...
}

proc consumer() { 
  // ... fill ...
}

proc main() {
  cobegin {
    producer();
    consumer();
  }
} 
