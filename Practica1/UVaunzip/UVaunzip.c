#include <stdio.h>
#include <stdlib.h>

typedef struct data_t {
  int n;
  char c;
} data_t;

FILE *openfile(char *);
void writedecompressed(data_t *);
void decompressfile(FILE *);

int
main (int argc, char *argv[]) {

  FILE *file;
  
  if (argc < 2) {
    printf("UVaunzip: file1 [file2 ...]\n");
    return 1;
  }

  for (int i = 1; i < argc; i++) {
    file = openfile(argv[i]);
    decompressfile(file);
    fclose(file);
  }

  return 0;
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
writedecompressed(data_t *data) {

  int n = data->n;
  char c = data->c;

  for (int i = 0; i < n; i++)
    printf("%c", c);

  return;
}

void
decompressfile(FILE *file) {

  data_t *data;

  data = (data_t *) malloc(sizeof(data_t));

  while ((fread(data, 1, 5, file)))
    writedecompressed(data);

  return;
}
  
