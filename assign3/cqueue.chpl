//----------------------------------------------------------------------------- 
// Program code for CS 415/515 Parallel Programming, Portland State University.
//----------------------------------------------------------------------------- 

// A circular queue (suitable for concurrent accesses)
//

module cqueue {

  config const bufSize = 10;		// buffer size (configurable)

  var buf$: [0..bufSize-1] sync int;	// buffer (cells are self-sync)
  var head$: sync int = 0;              // head idx 
  var tail$: sync int = 0;	        // tail idx 

  // add a new item at tail
  // return idx of position
  proc add(item) {
    var idx = tail$.readFE();
    tail$.writeEF((idx + 1) % bufSize);
    buf$[idx].writeEF(item);
    return idx;
  }

  // remove an item from head
  // return (idx, item) pair
  proc remove() {
    var idx = head$.readFE();
    head$.writeEF((idx + 1) % bufSize);
    var item = buf$[idx].readFE();
    var pair = (idx, item);
    return pair;
  }

}

