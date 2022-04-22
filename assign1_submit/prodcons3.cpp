/* Author: Mina Vu
 * File:   prodcons3.cpp
 * About:  Multiple producers and multiple consumers program
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
condition_variable cvar, pvar;
int pcount {0};
Queue* queue;
int numCons {0};
int numProd {0};
int* counts;

void producer(int k);
void consumer(int k);

// Main
int main(int argc, char **argv) {
  if (argc == 2) {
    numCons = atoi(argv[1]);
  }
  if (argc == 3) {
    numCons = atoi(argv[1]);
    numProd = atoi(argv[2]);
  }
  if (numCons == 0) numCons = 1;
  if (numProd == 0) numProd = 1;
  cout << "Running prodcons3.cpp with " + to_string(numProd) + " producers and " + to_string(numCons) + " consumers\n";

  counts = new int[numCons] {0};
  queue = new Queue(BUFSIZE);
  vector<thread> producers;
  vector<thread> consumers;
  for (int k {1}; k <= numProd; ++k) {
    producers.push_back(thread(producer, k));
  }
  for (int k {1}; k <= numCons; ++k) {
    consumers.push_back(thread(consumer, k));
  }

  for (int i {0}; i < numProd; ++i) {
    producers[i].join();
  }
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
void producer(int k) {
  int seg = NUMITEMS / numProd;
  int rem = NUMITEMS % numProd;
  int low = (k - 1) * seg + 1;
  int high = k * seg;
  if (k == numProd) high += rem;

  cout << "Producer[" + to_string(k) + "] for segment [" + to_string(low) + "..." + to_string(high) + "] starting on core " + to_string(sched_getcpu()) + "\n";

  for (int i {low}; i <= high; ++i) {
    unique_lock<mutex> lck(mtx);
    while (queue->isFull()) {
      cvar.wait(lck);
    }
    queue->add(i);
    lck.unlock();
    cvar.notify_one();
    cout << "Producer[" + to_string(k) + "] added " + to_string(i) + " (qsz: " + to_string(queue->size()) + ")\n";
  }

  unique_lock<mutex> lck(mtx);
  if (pcount != numProd - 1) {
    ++pcount;
    pvar.wait(lck);
  }
  lck.unlock();

  if (k == numProd) {
    for (int i {1}; i <= numCons; ++i) {
      unique_lock<mutex> lck(mtx);
      while (queue->isFull()) {
        cvar.wait(lck);
      }
      queue->add(ENDSIG);
      lck.unlock();
      cvar.notify_one();
      cout << "Producer[" + to_string(k) + "] added an ENDSIG (qsz: " + to_string(queue->size()) + ")\n";
    }
  }
  pvar.notify_all();

  cout << "Producer[" + to_string(k) + "] ending\n";
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
    lck.unlock();
    cvar.notify_one();

    if (val != ENDSIG) {
      cout << "Consumer[" + to_string(k) + "] rem'd " + to_string(val) + " (qsz: " + to_string(queue->size()) + ")\n";
    } else {
      cout << "Consumer[" + to_string(k) + "] rem'd an ENDSIG (qsz: " + to_string(queue->size()) + ")\n";
    }
  }

  cout << "Consumer[" + to_string(k) + "] ending\n";
}

