#include "disassembler.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  char *filename = NULL;
  FILE *fptr;

  // load filename from input args
  if (argc > 1) {
    filename = argv[1];
  } else {
    printf("You need to specify a file.\n");
    return 1;
  }

  // open our binary file
  fptr = fopen(filename, "rb");
  if (fptr == NULL) {
    printf("File does not exist or could not be opened.");
    return 1;
  }

  // determine file size and create a buffer
  // set pointer to end of file
  fseek(fptr, 0L, SEEK_END);
  // read position at end of file for total size
  int fsize = ftell(fptr);
  // reset pointer to beginning of file
  fseek(fptr, 0L, SEEK_SET);

  // allocate a buffer on the heap to fit this file
  unsigned char *buffer = malloc(fsize * sizeof(unsigned char));
  fread(buffer, fsize, 1, fptr);
  // close the file; we have read it and are done with it
  fclose(fptr);

  // disassemble our code!
  int pc = 0;
  while (pc < fsize) {
    pc += disassemble(buffer, pc);
  }

  free(buffer);
  return 0;
}
