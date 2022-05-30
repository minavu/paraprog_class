//----------------------------------------------------------------------------- 
// Program code for CS 415P/515 Parallel Programming, Portland State University
//----------------------------------------------------------------------------- 

// An utility program for verifying that the integers in a data file
// are sorted in the ascending order.
//
// Usage: 
//   linux> ./verify <datafile>
//
//
#include <stdio.h>

int main(int argc, char **argv) 
{
  FILE *fp;
  int fsize;

  if (argc != 2) {
    printf("Usage: ./verify <datafile>\n");
    return 0;
  }
  if (!(fp = fopen(argv[1], "r"))) {
    printf("Can't open datafile %s\n", argv[1]);
    return 0;
  }

  fseek(fp, 0L, SEEK_END);
  if (! (fsize = ftell(fp))) {
    printf("No data in %s\n", argv[1]);
    return 0;
  }

  int val1, val2, intSize = sizeof(int);
  rewind(fp);            
  fread(&val1, intSize, 1, fp);
  while (fread(&val2, intSize, 1, fp)) {
    if (val1 > val2) {
      printf("Failed at %07ld: %u > %u\n", ftell(fp), val1, val2);
      return 0;
    }
    val1 = val2;
  }
  printf("Data in '%s' are sorted (total %d items)\n", 
	 argv[1], fsize/intSize);
}

