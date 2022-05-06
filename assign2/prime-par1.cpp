//----------------------------------------------------------------------------- 
// Program code for CS 415P/515 Parallel Programming, Portland State University
//----------------------------------------------------------------------------- 

// A prime-finding program (Naive Multi-Thread Version).
//
// Usage: 
//   linux> ./prime-par1 N [P]
//
#include <iostream>
#include <cmath> 
#include <chrono>
#include <atomic>
#include <thread>
#include <vector>
#include <string>
using namespace std; 

int N, P {1};
atomic<int> totalPrimes(0);
int* sieve;
bool* candidate;

void worker(int k);

int main(int argc, char **argv) {
  if (argc < 2) {
    cout << "Usage: ./prime-par1 N [P]\n"; 
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
  cout << "prime-par1 (" << P << " threads) over [2.." << N << "] starting...\n";

  auto start_time = chrono::steady_clock::now();

  int sqrtN = sqrt(N);

  sieve = new int[sqrtN+1] {0};
  candidate = new bool[N+1];
  for (int i = 2; i <= sqrtN; i++)
    candidate[i] = true;
    
  for (int i = 2; i <= sqrtN; i++)
    if (candidate[i]) {
      sieve[i] = i;
      ++totalPrimes;
      for (int j = i+i; j <= sqrtN; j += i) {
        candidate[j] = false;
      }
    }

  vector<thread> workers(P);
  for (int i {0}; i < P; ++i) {
    workers[i] = thread(worker, i);
  }

  for (int i {0}; i < P; ++i) {
    workers[i].join();
  }

  auto end_time = chrono::steady_clock::now();
  auto duration = chrono::duration<double, std::milli> (end_time - start_time);

  cout << "prime-par1 (" << P << " threads) found " << totalPrimes << " primes in "
       << duration.count() << " ms \n";
}

void worker(int k) {
  int sqrtN = sqrt(N);
  int sections = (N - sqrtN) / P;
  int low = k * sections + sqrtN + 1;
  int high = (k+1) * sections + sqrtN;
  if (k == P-1) high = N;

  cout << "Worker[" + to_string(k) + "] starting work on section [" + to_string(low) + "..." + to_string(high) + "]\n";

  for (int i {low}; i <= high; ++i)
    candidate[i] = true;

  for (int i {2}; i <= sqrtN; ++i) {
    if (!sieve[i]) continue;
    int starting = low / sieve[i] * sieve[i];
    int remainder = low % sieve[i];
    if (remainder) starting += sieve[i];
    for (int j = starting; j <= high; j += sieve[i]) {
      candidate[j] = false;
    }
  }

  for (int i {low}; i <= high; ++i)
    if (candidate[i])
      ++totalPrimes;
}
