//----------------------------------------------------------------------------- 
// Program code for CS 415/515 Parallel Programming, Portland State University.
//----------------------------------------------------------------------------- 

// Oddven sort (sequential version)
//

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// Print array
void print_array(int *a, int n) {
  for (int i = 0; i < n; i++)
    printf("%3d ", a[i]);
  printf("\n");
}

// Initialize array with random 8-bit int values
// (except if WORST flag is on, set array to the reverse of 1..N)
void init_array(int *a, int n) {
#ifdef WORST
  for (int i = 0; i < n; i++)
    a[i] = n - i + 1;
#else
  srand(time(NULL));
  for (int i = 0; i < n; i++)
    a[i] = (rand()*1./RAND_MAX) * 0xff;
#endif  
#ifdef DEBUG
  printf("Init: ");
  print_array(a, n);
#endif
}

// Verify that array is sorted
// (if not sorted, show the violation pair of elements)
void verify_array(int *a, int n) {
  for (int i = 0; i < n-1; i++)
    if (a[i] > a[i+1]) {
      printf("FAILED: a[%d]=%d, a[%d]=%d\n", i, a[i], i+1, a[i+1]);
      return;
    }
  printf("%d element array is sorted.", n);  
}

// Swap two elments
void swap(int *x, int *y) {
  int tmp = *x;
  *x = *y;
  *y = tmp;
}

// Oddeven sort
// 
void oddeven_sort(int *a, int n) {
  for (int t = 1; t <= (n+1)/2; t++) {
    for (int i = 0; i < n-1; i += 2)      // even phase
      if (a[i] > a[i+1]) 
	swap(&(a[i]), &(a[i+1]));
    for (int i = 1; i < n-1; i += 2)      // odd phase
      if (a[i] > a[i+1])
	swap(&(a[i]), &(a[i+1]));
#ifdef DEBUG
    printf("t=%d:  ", t);
    print_array(a, n);
#endif
  }
}

// Main routine
int main(int argc, char **argv) {
  int N = 10;
  if (argc > 1) {
    if ((N = atoi(argv[1])) < 2) {
      printf("Use default N = 10\n"); 
      N = 10;
    }
  }
  int a[N];
  init_array(a, N);
  oddeven_sort(a, N);
  verify_array(a, N);
}
