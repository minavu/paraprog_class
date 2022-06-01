//----------------------------------------------------------------------------- 
// Program code for CS 415P/515 Parallel Programming, Portland State University
//----------------------------------------------------------------------------- 

// Bucket sort from file (parallel version)
//
// Usage: 
//   linux> mpirun -n P bsort-mpi <infile> <outfile> (P must be a power of 2)
//   -- use P buckets to sort data in <infile> and write result to <outfile>
// 
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <mpi.h>

#define DATABITS 13     // assume data are 13-bit integers: [0,8191] 
#define TAG 1001

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

// Bubble sort
void bubble_sort(int *a, int n) {
  for (int i = 0; i < n; i++)
    for (int j = i+1; j < n; j++) 
      if (a[i] > a[j]) {
        int tmp = a[i];
        a[i] = a[j];
        a[j] = tmp;
      }
}

// Main routine 
int main(int argc, char **argv) {
  int rank, P;
  MPI_Offset N;
  MPI_File fin, fout;
  MPI_Status st;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &P);

  if (argc != 3) {
    if (rank == 0)
      printf("Usage: mpirun -n P bsort-mpi <infile> <outfile> (P must be a power of 2)\n");
    MPI_Finalize();
    return 0;
  }

  if (!IsPowerOf2(P)) {
    if (rank == 0)
      printf("Error: P must be a power of 2\n");
    MPI_Finalize();
    return 0;
  }

  // all processes open the same two files to read and write
  MPI_File_open(MPI_COMM_WORLD, argv[1], MPI_MODE_RDONLY, MPI_INFO_NULL, &fin);
  MPI_File_open(MPI_COMM_WORLD, argv[2], MPI_MODE_CREATE|MPI_MODE_WRONLY, MPI_INFO_NULL, &fout);

  // all processes get file size and compare to number of processes
  MPI_File_get_size(fin, &N);
  if (N % P != 0) {
    if (rank == 0)
      printf("Error: Data size of infile does not evenly divide into number of processes\n");
    MPI_Finalize();
    return 0;
  }

  // all processes read subset of data from infile at offset correlating to rank
  int bufsize = N/P/sizeof(int);
  int *buf = (int*) malloc(bufsize * sizeof(int));
  int offset = rank * bufsize * sizeof(int);
  MPI_File_set_view(fin, offset, MPI_INT, MPI_INT, "native", MPI_INFO_NULL);
  MPI_File_read(fin, buf, bufsize, MPI_INT, &st);

  // allocate buckets with a safe bucket size
  int bucsize = fmax(2*(N/P)/P, 8);
  int bucket[P][bucsize]; 

  // initialize individual bucket count
  int bcnt[P];
  for (int k = 0; k < P; k++)
    bcnt[k] = 0;

  // distribute data to buckets
  for (int i = 0; i < bufsize; i++) {
    int k = BktIdx(buf[i], DATABITS, P);
    bucket[k][bcnt[k]++] = buf[i];
  }
#ifdef DEBUG
  for (int k = 0; k < P; k++) {
    printf("rank[%d] bucket[%d]: ", rank, k);
    print_array(bucket[k], bcnt[k]);
  }
#endif

  // send bucket[k] to process k
  for (int k = 0; k < P; ++k) {
    MPI_Send(&bucket[k], bcnt[k], MPI_INT, k, TAG, MPI_COMM_WORLD);
  }

  // create receive buffer for incoming data and count
  int rcnt = 0;
  int rbuf[2*N/P];

  // receive bucket[k] from all processes into rbuf
  for (int k = 0; k < P; ++k) {
    int temp_cnt;
    int temp_buf[bucsize];
    MPI_Recv(temp_buf, bucsize, MPI_INT, k, MPI_ANY_TAG, MPI_COMM_WORLD, &st);
    MPI_Get_count(&st, MPI_INT, &temp_cnt);

    for (int j = 0; j < temp_cnt; ++j)
      rbuf[rcnt++] = temp_buf[j];
  }

  // bubble sort rbuf
  bubble_sort(rbuf, rcnt);

  // collectively compute file-view offsets
  int file_view[P];
  MPI_Scan(&rcnt, &file_view[rank], 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
  file_view[rank] -= rcnt;

  // all processes write to same file at different offsets
  MPI_File_set_view(fout, file_view[rank] * sizeof(int), MPI_INT, MPI_INT, "native", MPI_INFO_NULL);
  MPI_File_write(fout, rbuf, rcnt, MPI_INT, &st);

  MPI_File_close(&fin);
  MPI_File_close(&fout);
  MPI_Finalize();
  return 0;
}

