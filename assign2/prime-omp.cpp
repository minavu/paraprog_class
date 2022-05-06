//----------------------------------------------------------------------------- 
// Program code for CS 415P/515 Parallel Programming, Portland State University
//----------------------------------------------------------------------------- 

// A prime-finding program (OpenMP version).
//
// Usage: 
//   linux> ./prime-omp N [P]
//
#include <iostream>
#include <cmath> 
#include <chrono>
using namespace std; 

int main(int argc, char **argv) {
  int N, P {1};
  if (argc < 2) {
    cout << "Usage: ./prime-omp N [P]\n"; 
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
  cout << "prime-omp (" << P << " threads) over [2.." << N << "] starting ...\n";

  auto start_time = chrono::steady_clock::now();

  bool candidate[N+1];
#pragma omp parallel for num_threads(P) shared(candidate)
  for (int i = 2; i <= N; i++)
    candidate[i] = true;
    
  int sqrtN = sqrt(N);
  for (int i = 2; i <= sqrt(N); i++)
    if (candidate[i]) 
#pragma omp parallel for num_threads(P) shared(candidate)
      for (int j = i+i; j <= N; j += i)
        candidate[j] = false;

  int totalPrimes = 0;
#pragma omp parallel for num_threads(P) reduction(+:totalPrimes) shared(candidate)
  for (int i = 2; i <= N; i++)
    if (candidate[i]) 
      totalPrimes++;

  auto end_time = chrono::steady_clock::now();
  auto duration = chrono::duration<double, std::milli> (end_time - start_time);

  cout << "prime-omp (" << P << " threads) found " << totalPrimes << " primes in "
       << duration.count() << " ms \n";
}
