#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define BUFFERSIZE 100

void printfile(char *filename);

int main (int argc, char *argv[]) {

  if (argc == 1) {
    exit(0);
  }

  for (int i = 1; i < argc; i++) {
    printfile(argv[i]);
  }

  return 0;
}

void printfile(char *filename) {

  FILE *file;
  char *buffer;
  int bytesread;
  
  if (!(file = fopen(filename, "r"))) {
    perror(strerror(ENOENT));
    exit(1);
  }

  if (!(buffer = (char *) malloc(BUFFERSIZE + 1))) {
    perror("Malloc: Could not allocate memory");
    exit(1);
  }

  while ((bytesread = fread(buffer, 1, BUFFERSIZE, file))) {
    buffer[bytesread] = '\0';
    printf("%s", buffer);
  }

  printf("\n");

  free(buffer);
  fclose(file);

  return;
}
