//----------------------------------------------------------------------------- 
// Program code for CS 415P/515 Parallel Programming, Portland State University
//----------------------------------------------------------------------------- 

// A prime-finding program (Third Multi-Thread Version).
//
// Usage: 
//   linux> ./prime-par3 N [P]
//
#include <iostream>
#include <cmath> 
#include <chrono>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <string>
using namespace std; 

int N, P {1};
atomic<int> totalPrimes(0);
int* sieve;
bool* candidate;
mutex mtx;
condition_variable cvar;
int* stats;
int idx {0};
atomic<int> lastSieve(0);
atomic<bool> allFound(false);

void worker(int k);

int main(int argc, char **argv) {
  if (argc < 2) {
    cout << "Usage: ./prime-par3 N [P]\n"; 
    exit(0);
  }
  if ((N = atoi(argv[1])) < 2) {
    cout << "N must be greater than 1\n"; 
    exit(0);
  }
  if (argc == 3 && (P = atoi(argv[2])) == 0) {
    cout << "P must be greater than 0\n";
    exit(0);
  }
  cout << "prime-par3 (" << P << " threads) over [2.." << N << "] starting...\n";

  auto start_time = chrono::steady_clock::now();
  int sqrtN = sqrt(N);

  sieve = new int[sqrtN+1] {0};
  candidate = new bool[N+1];
  for (int i = 2; i <= N; i++)
    candidate[i] = true;
  stats = new int[P+1] {0};
    
  vector<thread> workers(P);
  for (int i {0}; i < P; ++i) {
    workers[i] = thread(worker, i);
  }

//Main finding sieves
  for (int i = 2; i <= sqrtN; i++) {
    if (candidate[i]) {
      lastSieve = i;
      unique_lock<mutex> lck(mtx);
      sieve[i] = i;
      lck.unlock();
      cvar.notify_all();
      ++totalPrimes;
      ++stats[0];
      for (int j = i+i; j <= sqrtN; j += i) {
        candidate[j] = false;
      }
    }
  }

  allFound = true;

  for (int i {0}; i < P; ++i) {
    workers[i].join();
  }

  for (int i = sqrtN+1; i <= N; ++i)
    if (candidate[i]) {
      ++totalPrimes;
    }

  auto end_time = chrono::steady_clock::now();
  auto duration = chrono::duration<double, std::milli> (end_time - start_time);

  cout << "prime-par3 (" << P << " threads) found " << totalPrimes << " primes in "
       << duration.count() << " ms \n";

  cout << "Number of sieve primes: " + to_string(stats[0]) + "\n";
  int sum = 0;
  cout << "Stats are [";
  for (int i = 1; i < P+1; ++i) {
    cout << to_string(stats[i]) + ",";
    sum += stats[i];
  }
  cout << "] = " + to_string(sum) + "\n";
}

void worker(int k) {
  cout << "Worker[" + to_string(k) + "] starting work\n";
  int low = sqrt(N)+1;
  int localIdx;
  do {
    unique_lock<mutex> lck(mtx);
    if (allFound && idx >= lastSieve) { //terminate
      lck.unlock();
      cvar.notify_all();
      break;
    }
    if (allFound && idx < lastSieve) { //compete
      localIdx = idx;
      lck.unlock();
      while (!candidate[++localIdx]) {}
      lck.lock();
      if (idx < localIdx) {
        idx = localIdx;
      } else {
        localIdx = -1;
      }
      lck.unlock();
      if (localIdx != -1) { //do work
        ++stats[k+1];
        int starting = low / localIdx * localIdx;
        int remainder = low % localIdx;
        if (remainder) starting += localIdx;
        for (int j = starting; j <= N; j += localIdx) {
          candidate[j] = false;
        }
      }
    }
    if (!allFound && idx >= lastSieve) { //wait
      cvar.wait(lck);
      lck.unlock();
    }
  } while (true);
  cout << "Worker[" + to_string(k) + "] ending\n";
}
