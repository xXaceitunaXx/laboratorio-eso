#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>


// i'll implement in the future my own strstr() function
bool iswordfound(char *line, char *word) {

  if (strlen(line) < strlen(word))
    return false;

  return strstr(line, word);
}

void processfile(FILE *file, char *word) {

  size_t n = 0; 
  size_t bytesread;
  char *line = NULL;

  while ((bytesread = getline(&line, &n, file)) != -1) {
    if (iswordfound(line, word))
      printf("%s", line);
  }

  free(line);
  
  return;
}

FILE *openfile(char *filename) {

  FILE *file;
  
  if (!(file = fopen(filename, "r"))) {
    printf("UVagrep: cannot open file\n");
    exit(1);
  }

  return file;
}

int main (int argc, char *argv[]) {

  FILE *file;
  char *word;
  
  if (argc == 1) {
    printf("UVagrep: searchterm [file ...]\n");
    return(1);
  }

  word = argv[1];

  if (argc == 2) {
    processfile(stdin, word);
    return(0);
  }
  
  for (int i = 2; i < argc; i++) {
    file = openfile(argv[i]);
    processfile(file, word);
  }
  
  return(0);
}
