//----------------------------------------------------------------------------- 
// Program code for CS 415P/515 Parallel Programming, Portland State University
//----------------------------------------------------------------------------- 

// A prime-finding program (Second OpenMP version).
//
// Usage: 
//   linux> ./prime-omp2 N
//
#include <iostream>
#include <cmath> 
#include <chrono>
#include <string>
#include "omp.h"
using namespace std; 

int main(int argc, char **argv) {
  int N, P {1};
  if (argc < 2) {
    cout << "Usage: ./prime-par2 N [P]\n"; 
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
  cout << "prime-omp2 (" << P << " threads) over [2.." << N << "] starting ...\n";

  auto start_time = chrono::steady_clock::now();

  int sqrtN = sqrt(N);

  int stats[P] {0};
  int sieve[sqrtN+1] {0};
  bool candidate[N+1];
  for (int i = 2; i <= N; i++)
    candidate[i] = true;
    
  for (int i = 2; i <= sqrtN; i++) {
    if (candidate[i]) {
      sieve[i] += i;
      for (int j = i+i; j <= sqrtN; j += i)
        candidate[j] = false;
    }
  }

  omp_set_num_threads(P);
#pragma omp parallel
{
#pragma omp single
  for (int i = 2; i <= sqrtN; ++i) {
    if (sieve[i]) {
      int starting = (sqrtN+1) / i * i;
      int remainder = (sqrtN+1) % i;
      if (remainder) starting += i;
#pragma omp task shared(i)
{
      int k = omp_get_thread_num();
      ++stats[k];
      for (int j = starting; j <= N; j += i)
        candidate[j] = false;
}
    }
  }
}

  int totalPrimes {0};
  for (int i = 2; i <= N; i++)
    if (candidate[i]) 
      totalPrimes++;

  auto end_time = chrono::steady_clock::now();
  auto duration = chrono::duration<double, std::milli> (end_time - start_time);

  int sum = 0;
  cout << "Stats are [";
  for (int i = 0; i < P; ++i) {
    cout << to_string(stats[i]) + ",";
    sum += stats[i];
  }
  cout << "] = " + to_string(sum) + "\n";
  cout << "prime-omp2 (" << P << " threads) found " << totalPrimes << " primes in "
       << duration.count() << " ms \n";
}
