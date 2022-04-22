/* Author: Mina Vu
 * File:   prodcons1.cpp
 * About:  1 producer and 1 consumer program
 */
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <sched.h>
#include "queue.h"

using namespace std;

int BUFSIZE = 20;
int NUMITEMS = 100;
mutex mtx;
condition_variable cvar;
Queue* queue;

void producer();
void consumer();

// Main
int main(int argc, char **argv) {
  queue = new Queue(BUFSIZE);
  thread prod(producer);
  thread cons(consumer);

  prod.join();
  cons.join();

  cout << "Main: all done!\n";

  return 0;
}

// Producer
void producer() {
  cout << "Producer starting on core " + to_string(sched_getcpu()) + "\n";

  for (int i {1}; i <= NUMITEMS; ++i) {
    unique_lock<mutex> lck(mtx);
    if (queue->isFull()) {
      cvar.wait(lck);
    }
    queue->add(i);
    lck.unlock();
    cvar.notify_one();
    cout << "Producer added " + to_string(i) + " (qsz: " + to_string(queue->size()) + ")\n";
  }

  cout << "Producer ending\n";
}

// Consumer
void consumer() {
  cout << "Consumer starting on core " + to_string(sched_getcpu()) + "\n";

  for (int i {1}; i <= NUMITEMS; ++i) {
    unique_lock<mutex> lck(mtx);
    if (queue->isEmpty()) {
      cvar.wait(lck);
    }
    int val = queue->remove();
    lck.unlock();
    cvar.notify_one();
    cout << "Consumer rem'd " + to_string(val) + " (qsz: " + to_string(queue->size()) + ")\n";
  }

  cout << "Consumer ending\n";
}
