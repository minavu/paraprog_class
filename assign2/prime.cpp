//----------------------------------------------------------------------------- 
// Program code for CS 415P/515 Parallel Programming, Portland State University
//----------------------------------------------------------------------------- 

// A prime-finding program (Sequential version).
//
// Usage: 
//   linux> ./prime N
//
#include <iostream>
#include <cmath> 
#include <chrono>
using namespace std; 

int main(int argc, char **argv) {
  int N;
  if (argc < 2) {
    cout << "Usage: ./prime N\n"; 
    exit(0);
  }
  if ((N = atoi(argv[1])) < 2) {
    cout << "N must be greater than 1\n"; 
    exit(0);
  }
  cout << "prime (seq) over [2.." << N << "] starting ...\n";

  auto start_time = chrono::steady_clock::now();

  bool candidate[N+1];
  for (int i = 2; i <= N; i++)
    candidate[i] = true;
    
  int sqrtN = sqrt(N);
  for (int i = 2; i <= sqrtN; i++)
    if (candidate[i]) 
      for (int j = i+i; j <= N; j += i)
        candidate[j] = false;

  int totalPrimes = 0;
  for (int i = 2; i <= N; i++)
    if (candidate[i]) 
      totalPrimes++;

  auto end_time = chrono::steady_clock::now();
  auto duration = chrono::duration<double, std::milli> (end_time - start_time);

  cout << "prime (seq) found " << totalPrimes << " primes in "
       << duration.count() << " ms \n";
}
