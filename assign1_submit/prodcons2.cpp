/* Author: Mina Vu
 * File:   prodcons2.cpp
 * About:  1 producer and multiple consumers program
 */
#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <sched.h>
#include "queue.h"

using namespace std;

int BUFSIZE = 20;
int NUMITEMS = 100;
int ENDSIG = -1;
mutex mtx;
condition_variable cvar;
Queue* queue;
int numCons {0};
int* counts;

void producer();
void consumer(int k);

// Main
int main(int argc, char **argv) {
  if (argc > 1) {
    numCons = atoi(argv[1]);
  }
  if (numCons == 0) numCons = 1;
  cout << "Running prodcons2.cpp with 1 producer and " + to_string(numCons) + " consumers\n";

  counts = new int[numCons] {0};
  queue = new Queue(BUFSIZE);
  thread prod(producer);
  vector<thread> consumers;
  for (int k {1}; k <= numCons; ++k) {
    consumers.push_back(thread(consumer, k));
  }

  prod.join();
  for (int i {0}; i < numCons; ++i) {
    consumers[i].join();
  }

  cout << "Consumer stats: [";
  int sum {0};
  for (int i {0}; i < numCons; ++i) {
    sum += counts[i];
    cout << counts[i];
    if (i != numCons - 1) {
      cout << ",";
    }
  }
  cout << "] = " + to_string(sum) + "\n";

  cout << "Main: all done!\n";

  return 0;
}

// Producer
void producer() {
  cout << "Producer starting on core " + to_string(sched_getcpu()) + "\n";

  for (int i {1}; i <= NUMITEMS; ++i) {
    unique_lock<mutex> lck(mtx);
    while (queue->isFull()) {
      cvar.wait(lck);
    }
    queue->add(i);
    cvar.notify_one();
    lck.unlock();
    cout << "Producer added " + to_string(i) + " (qsz: " + to_string(queue->size()) + ")\n";
  }

  for (int i {1}; i <= numCons; ++i) {
    unique_lock<mutex> lck(mtx);
    while (queue->isFull()) {
      cvar.wait(lck);
    }
    queue->add(ENDSIG);
    cvar.notify_one();
    lck.unlock();
    cout << "Producer added an ENDSIG (qsz: " + to_string(queue->size()) + ")\n";
  }

  cout << "Producer ending\n";
}

// Consumer
void consumer(int k) {
  cout << "Consumer[" + to_string(k) + "] starting on core " + to_string(sched_getcpu()) + "\n";

  for (int i {1}; i <= NUMITEMS; ++i) {
    unique_lock<mutex> lck(mtx);
    while (queue->isEmpty()) {
      cvar.wait(lck);
    }
    int val = queue->remove();
    if (val == ENDSIG) {
      break;
    }
    ++counts[k-1];
    cvar.notify_one();
    lck.unlock();

    if (val != ENDSIG) {
      cout << "Consumer[" + to_string(k) + "] rem'd " + to_string(val) + " (qsz: " + to_string(queue->size()) + ")\n";
    } else {
      cout << "Consumer[" + to_string(k) + "] rem'd an ENDSIG (qsz: " + to_string(queue->size()) + ")\n";
    }
  }

  cout << "Consumer[" + to_string(k) + "] ending\n";
}

