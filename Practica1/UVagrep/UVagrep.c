#include <stdio.h>

int main (int argc, char *argv[]) {

  if (argc == 1) {
    printf("UVagrep: searchterm [file ...]\n");
    return(1);
  }

  printf("%s", argv[1]);

  return(0);
}
