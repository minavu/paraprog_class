//----------------------------------------------------------------------------- 
// Program code for CS 415P/515 Parallel Programming, Portland State University
//----------------------------------------------------------------------------- 

// Bucket sort (sequential version)
//
// Usage: 
//   linux> ./bsort B [N]   
//   -- B (#buckets) must be a power of 2; B defaults to 10
// 
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#define DATABITS 13     // assume data are 13-bit integers: [0,8191] 

// return true if x is a power of 2
#define IsPowerOf2(x) (!((x) & ((x) - 1)))

// bucket index for integer x of b bits (B is #buckets)
#define BktIdx(x,b,B) ((x) >> ((b) - (int)log2(B)))

// Print array
void print_array(int *a, int n) {
  for (int i = 0; i < n; i++)
    printf("%4d ", a[i]);
  printf("\n");
}

// Initialize array with random 13-bit int values
// (except if WORST flag is on, set array to the reverse of 1..N)
void init_array(int *a, int n) {
#ifdef WORST
  for (int i = 0; i < n; i++)
    a[i] = n - i + 1;
#else
  srand(time(NULL));
  for (int i = 0; i < n; i++)
    a[i] = rand() % 8192;
#endif  
#ifdef DEBUG
  printf("Init (%d elements): ", n);
  print_array(a, n);
#endif
}

// Verify that array is sorted (and report error if exits)
void verify_array(int *a, int n) {
  for (int i = 0; i < n-1; i++)
    if (a[i] > a[i+1]) {
      printf("FAILED: a[%d]=%d, a[%d]=%d\n", i, a[i], i+1, a[i+1]);
      return;
    }
  printf("%d element array is sorted.", n);  
}

// Bubble sort
//
void bubble_sort(int *a, int n) {
  for (int i = 0; i < n; i++)
    for (int j = i+1; j < n; j++) 
      if (a[i] > a[j]) {
	int tmp = a[i];
	a[i] = a[j];
	a[j] = tmp;
      }
}

// Bucket sort
//
void bucket_sort(int *a, int n, int num_buckets) {
  // allocate buckets with a safe bucket size
  int bucket[num_buckets][2*n/num_buckets]; 

  // individual bucket count
  int bcnt[num_buckets];
  for (int k = 0; k < num_buckets; k++)
    bcnt[k] = 0;

  // distribute data to buckets
  for (int i = 0; i < n; i++) {
    int k = BktIdx(a[i], DATABITS, num_buckets);
    bucket[k][bcnt[k]++] = a[i];
  }
#ifdef DEBUG
  for (int k = 0; k < num_buckets; k++) {
    printf("bucket[%d]: ", k);
    print_array(bucket[k], bcnt[k]);
  }
#endif

  // bubble sort each bucket
  for (int k = 0; k < num_buckets; k++)
    bubble_sort(bucket[k], bcnt[k]);
   
  // copy sorted data back to array a
  int i = 0;
  for (int k = 0; k < num_buckets; k++) 
    for (int j = 0; j < bcnt[k]; j++)
      a[i++] = bucket[k][j];
}

// Main routine 
// 
int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: ./bsort B [N]\n");
    exit(0);
  }

  int B = atoi(argv[1]);  // get param B, verify it's a power of 2
  if (!IsPowerOf2(B)) {
    printf("B (#buckets) must be a power of 2\n");
    exit(0);
  }
  
  int N = 10;             // N defaults to 10
  if (argc == 3) {        // get param B, verify it's a power of 2
    if ((N = atoi(argv[2])) < 1) {
      printf("N must be positive, use default value of 10\n");
      N = 10; 
    }
  }

  // initialize data array
  int a[N];
  init_array(a, N);     

  bucket_sort(a, N, B);

#ifdef DEBUG
  printf("Result: ");
  print_array(a, N);
#endif
  verify_array(a, N);
}
