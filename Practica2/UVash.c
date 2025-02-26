#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define ERRUVASH "An error has ocurred\n"
#define PROMPT "UVash> "
#define EXIT "exit"
#define SEPRCHR " "
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

typedef struct child {
  pid_t pid;
  int status;
  struct child *next;
} child_t;

/* ################################# */
/* Usefull global variables */

bool INTERACTIVE; // enables interactive mode (program to print PROMPT, sets INPUT to stdin) 
FILE *INPUT; // stream where to get command inputs

/* ################################# */

FILE *openfile (char *);
void startsh ();
char **createargv (char *); // TODO
process_t *parseprcss (char *);
int execprcss (process_t *);

/* ################################# */

int
main (int argc, char *argv[]) {

  if (argc > 2) {
    fprintf(stderr, ERRUVASH);
    return 1;
  }

  INTERACTIVE = (argc == 1); // if no file provided, interactive mode on
  INPUT = INTERACTIVE ? stdin : openfile(argv[1]);

  startsh();
  
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


/*
  TODO
*/
char
**createargv (char *arguments) {
  fprintf(stderr, "NOT IMPLEMENTED YET\n");

  return NULL;
}

process_t
*parseprcss (char *cmdline) {

  char *command, *arguments;
  char **argv;
  process_t *p;
  
  command = cmdline;
  arguments = strsep(&cmdline, SEPRCHR);
  argv = (command == arguments ? NULL : createargv(arguments));
  
  p = (process_t *)  malloc(sizeof(process_t));
  p->command = command;
  p->arguments = argv;
  p->next = NULL;

  return p;
}

/*
  TODO execprcss (process *)
  Must find a way to print result in the file, maybe it's easy
*/
int
execprcss (process_t *p) {

  process_t *prev;
  pid_t pid;
  child_t *head, *c;

  head = c = (child_t *) malloc(sizeof(child_t));

  while(p) {
    pid = fork();

    switch (pid) {
    case -1:
      perror("fork");
      exit(1);
    case 0:
      execvp(p->command, p->arguments);
      exit(0);
    default:
      prev = p;
      p = p->next;
      free(prev);
      c->pid = pid;
      if (p)
	c->next = (child_t *) malloc(sizeof(child_t));
      c = c->next;
    }
  }

  while(head) {
    waitpid(head->pid, &(head->status), 0);
    c = head;
    head = head->next;
    free(c);
  }
    
  return 0;
}

void
startsh () {
  
  int nread;
  size_t n = 0;
  char *line = NULL;
  process_t *prcsslist;
  
  while ((nread = getline(&line, &n, INPUT)) != -1) {
    prcsslist = parseprcss(strsep(&line, "\n"));
    execprcss(prcsslist);
  }

  return;
}
