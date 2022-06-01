//----------------------------------------------------------------------------- 
// Program code for CS 415P/515 Parallel Programming, Portland State University
//----------------------------------------------------------------------------- 

// Bucket sort from file (parallel version)
//
// Usage: 
//   linux> mpirun -n P bsort-mpi2 <infile> <outfile> (P must be a power of 2)
//   -- use P buckets to sort data in <infile> and write result to <outfile>
// 
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <mpi.h>

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
  int rank, P, offset, bufsize, *buf;;
  MPI_Offset N;
  MPI_File fin, fout;
  MPI_Status st;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &P);

  if (argc != 3) {
    if (rank == 0)
      printf("Usage: mpirun -n P bsort-mpi2 <infile> <outfile> (P must be a power of 2)\n");
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

  // get file size and compare to number of processes
  MPI_File_get_size(fin, &N);
  if (N % P != 0) {
    if (rank == 0)
      printf("Error: Data size of infile does not evenly divide into number of processes\n");
    MPI_Finalize();
    return 0;
  }

  // read subset of data from infile at offset correlating to rank
  bufsize = N/P/sizeof(int);
  buf = (int*) malloc(bufsize * sizeof(int));
  offset = rank * bufsize * sizeof(int);
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

  // concat buckets into buf
  int i = 0;
  for (int j = 0; j < P; ++j)
    for (int k = 0; k < bcnt[j]; ++k)
      buf[i++] = bucket[j][k];

  // compute send displacement array with correct offset for each processor
  int sdisp[P];
  int soffset = 0;
  for (int p = 0; p < P; ++p) {
    sdisp[p] = soffset;
    soffset += bcnt[p];
  }

  // collectively compute count of each processor's receive buffer
  int rcnt[P];
  MPI_Allreduce(bcnt, rcnt, P, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

  // create recv buffer with enough room to store all items from all processors
  int *rbuf = (int*) malloc(rcnt[rank] * sizeof(int));

  // collectively compute number of items to be received from each processor
  int rcount[P];
  MPI_Alltoall(bcnt, 1, MPI_INT, rcount, 1, MPI_INT, MPI_COMM_WORLD);

  // compute recv displacement array with correct offset for each processor
  int rdisp[P];
  int roffset = 0;
  for (int p = 0; p < P; ++p) {
    rdisp[p] = roffset;
    roffset += rcount[p];
  }

  // collectively send/recv buf to all processors
  MPI_Alltoallv(buf, bcnt, sdisp, MPI_INT, rbuf, rcount, rdisp, MPI_INT, MPI_COMM_WORLD);

  // bubble sort rbuf
  bubble_sort(rbuf, rcnt[rank]);

  // collectively compute file-view offsets
  int file_view[P];
  MPI_Scan(&rcnt[rank], &file_view[rank], 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
  file_view[rank] -= rcnt[rank];

  // all processes write to same file
  MPI_File_set_view(fout, file_view[rank] * sizeof(int), MPI_INT, MPI_INT, "native", MPI_INFO_NULL);
  MPI_File_write(fout, rbuf, rcnt[rank], MPI_INT, &st);

  MPI_File_close(&fin);
  MPI_File_close(&fout);
  MPI_Finalize();
  return 0;
}

