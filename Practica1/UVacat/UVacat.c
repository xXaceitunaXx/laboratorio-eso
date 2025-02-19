#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>

#define BUFFERSIZE 100

// README said only program in main...
int main (int argc, char *argv[]) {

  FILE *file;              // Pointer to the file stream
  char *buffer, *filename; // Buffer where to read, file name

  // If no arguments terminate program
  if (argc == 1) {
    return 0;
  }

  // Loop over all given files
  for (int i = 1; i < argc; i++) {
    filename = argv[i];

    // Error handling
    if (!(file = fopen(filename, "r"))) {
      printf("UVacat: no puedo abrir fichero\n");
      exit(1);
    }

    if (!(buffer = (char *) malloc(BUFFERSIZE))) {
      printf("Malloc: no puedo alojar memoria\n");
      exit(1);
    }

    // Read until file end (ie, buffer == NULL)
    while ((fgets(buffer, BUFFERSIZE, file))) {
      printf("%s", buffer);
    }

    fclose(file);
  }

  free(buffer);

  return 0;
}
