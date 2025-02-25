#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define ERRUVASH "An error has ocurred\n"
#define PROMPT "UVash> "
#define REDRCHR '>'
#define PARLCHR '&'

/* ################################# */
/* Structures */

// Linked list of processes to run
typedef struct process {
  char *command; // command name to execute, can't be NULL
  char **arguments; // arguments for the command, could be NULL, last element should be NULL
  char *output; // file name, if no REDRCHR on parsing default is stdout
  struct process *next; // when PARLCHR present, next parallel command referenced
} process_t;

/* ################################# */
/* Usefull global variables */

bool INTERACTIVE; // enables interactive mode (program to print PROMPT, sets INPUT to stdin) 
FILE *INPUT; // stream where to get command inputs

/* ################################# */

FILE *openfile (char *);
int execprcss (process_t *);

/* ################################# */

int
main (int argc, char *argv[]) {

  int nread;
  size_t n = 0;
  char *line = NULL;
  
  if (argc > 2) {
    fprintf(stderr, ERRUVASH);
    return 1;
  }

  INTERACTIVE = (argc == 1); // if no file provided, interactive mode on
  INPUT = INTERACTIVE ? stdin : openfile(argv[1]);

  while ((nread = getline(&line, &n, INPUT)) != -1) { // FIXME: look for UVagrep
    if (strcmp(line, "exit"))
	return 0;
    printf("%s", line);
  }
  
  return 0;
}

/* ################################# */

FILE
*openfile (char *filename) {

  FILE *file;
  if (!(file = fopen(filename, "r"))) {
    fprintf(stderr, ERRUVASH);
    exit(1);
  }

  return file;
}

//TODO execprcss (process *)
int
execprcss (process_t *p) {

  execvp(p->command, p->arguments);
  
  return 0;
}
