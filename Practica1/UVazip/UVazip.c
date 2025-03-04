#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

void writecompressed(int, char, FILE *);
FILE *openfile(char *);
void compressfiles(char **);

int
main (int argc, char *argv[]) {
  
  if (argc < 2) {
    printf("UVazip: file1 [file2 ...]\n");
    return 1;
  }

  compressfiles(argv + 1);

  return 0;
}

void
writecompressed (int number, char character, FILE *file) {

  fwrite(&number, 1, 4, file); // Write 4 bytes in file (the number in format 0xXXXXXXXX 0xXXXXXXXX 0xXXXXXXXX 0xXXXXXXXX)
  fwrite(&character, 1, 1, file); // Write the character in file

  return;
}

FILE
*openfile (char *filename) {

  FILE *file;

  if (!(file = fopen(filename, "r"))) {
    printf("MENSAJE DE ERROR NO ESPECIFICADO");
    exit(1);
  }

  return file;
}

void
compressfiles (char **filenames) {

  int number = 0;
  char character, prev;
  FILE *file;
  bool notended;

  while (*filenames) {
    file = openfile(*(filenames++));
    notended = fread(&character, 1, 1, file);
    
    while (notended) {
      number++;
      
      prev = character;
      notended = fread(&character, 1, 1, file);

      if (prev != character) {
	      writecompressed(number, prev, stdout);
	      number = 0;
      }
    }

    fclose(file);
  }

  writecompressed(number, prev, stdout);
  
  return;
}
