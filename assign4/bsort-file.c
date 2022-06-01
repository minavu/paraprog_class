//----------------------------------------------------------------------------- 
// Program code for CS 415P/515 Parallel Programming, Portland State University
//----------------------------------------------------------------------------- 

// Bucket sort from file (sequential version)
//
// Usage: 
//   linux> ./bsort-file B <infile> <outfile>
//   -- B (#buckets) must be a power of 2; B defaults to 10
//   -- use B buckets to sort data in <infile> and write result to <outfile>
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

// Verify that array is sorted (and report error if exits)
void verify_array(int *a, int n) {
  for (int i = 0; i < n-1; i++)
    if (a[i] > a[i+1]) {
      printf("FAILED: a[%d]=%d, a[%d]=%d\n", i, a[i], i+1, a[i+1]);
      return;
    }
  printf("%d element array is sorted.\n", n);  
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
  if (argc < 4) {
    printf("Usage: ./bsort-file B <infile> <outfile> (B must be a power of 2)\n");
    exit(0);
  }

  int B = atoi(argv[1]);  // get param B, verify it's a power of 2
  if (!IsPowerOf2(B)) {
    printf("B (#buckets) must be a power of 2\n");
    exit(0);
  }

  FILE *fin = fopen(argv[2], "rb");
  FILE *fout = fopen(argv[3], "wb");
  
  fseek(fin, 0L, SEEK_END);
  int N = ftell(fin) / sizeof(int);
  if (N == 0) {
    printf("%s contains no data\n", argv[2]);
    exit(0);
  }

  rewind(fin);
  int a[N];
  fread(a, sizeof(int), N, fin);
  bucket_sort(a, N, B);

  fwrite(a, sizeof(int), N, fout);

#ifdef DEBUG
  printf("Result: ");
  print_array(a, N);
#endif
  verify_array(a, N);

  fclose(fin);
  fclose(fout);
}
