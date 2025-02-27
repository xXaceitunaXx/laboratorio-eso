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

// linked list of process arguments (another aproach could be coping in memory the whole argument)
typedef struct argument {
  char *pstring; // pointer to the argument in memory
  struct argument *next;
} argument_t;

// linked list of processes to run
typedef struct process {
  char *command; // command name to execute, can't be NULL
  char **arguments; // arguments for the command, could be NULL, last element should be NULL
  char *output; // file name, if no REDRCHR on parsing default is stdout
  struct process *next; // when PARLCHR present, next parallel command referenced
} process_t;

// linked list referencing all the processes launched
typedef struct child {
  pid_t pid; // pid of the child process
  int status; // status
  struct child *next;
} child_t;

/* ################################# */
/* Usefull global variables */

bool INTERACTIVE; // enables interactive mode (program to print PROMPT, sets INPUT to stdin) 
FILE *INPUT; // stream where to get command inputs

/* ################################# */

FILE *openfile (char *);
void startsh ();
char *clearwhites(char *);
char **createargv (char *);
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

char
*clearwhites (char *line) {
  while (*line == ' ' || *line == '\t')
    line++;

  return line;
}

/*
  TODO **createargv (char *arguments)
  parse all arguments into {char *, char *, ..., NULL} array
*/
char
**createargv (char *arguments) {

  char **argv = NULL;
  size_t argc = 0;
  argument_t *refs, *rfi;
  
  if (!arguments)
    return argv;

  if (!(refs = rfi = (argument_t *) malloc(sizeof(argument_t)))) {
      perror("malloc");
      exit(1);
    }
  
  while(arguments) {  
    arguments = clearwhites(arguments);
    rfi->pstring = strsep(&arguments, SEPRCHR);
    argc++;

    if (arguments) {
      if (!(rfi->next = (argument_t *) malloc(sizeof(argument_t)))) {
	perror("malloc");
	exit(1);
      }
      rfi = rfi->next;
    }
  } // need linked list or I loose all the references, strsep inserts '\0'

  if (!(argv = (char **) malloc(sizeof(char **) * (argc + 1)))) {
    perror("malloc");
    exit(1);
  }

  // go through arguments list freeing nodes while storing the pointers 

  printf("argc: %d for args: %p\n", (int) argc, refs);

  for (size_t i = 0; i < argc; i++) {
    argv[i] = refs->pstring;
    rfi = refs;
    refs = refs->next;
    free(rfi);
  }

  argv[argc] = NULL;
  
  fprintf(stderr, "NOT IMPLEMENTED YET\n");
  return argv;
}

process_t
*parseprcss (char *cmdline) {

  char *command;
  char **argv;
  process_t *p;

  cmdline = clearwhites(cmdline); // removes spaces (SEPRCHR, \t, ...) before command word 
  argv = createargv(cmdline); // arguments on *argv[] style
  command = strsep(&cmdline, SEPRCHR); // separates command from all args
  
  if (!(p = (process_t *) malloc(sizeof(process_t)))) {
    perror("malloc");
    exit(1);
  }

  /* --- this zone must take on acount on the future the parallelism --- */

  p->command = command;
  p->arguments = argv;
  p->next = NULL;
  
  /* ------------------------------------------------------------------- */
  
  return p; // process linked list ready to execute via execprcss
}

/*
  TODO execprcss (process *)
  Must find a way to print result in the file, maybe it's easy?
*/
int
execprcss (process_t *p) {

  process_t *prev;
  pid_t pid;
  child_t *head, *c;

  if (!(head = c = (child_t *) malloc(sizeof(child_t)))) {
    perror("malloc");
    exit(1);
  }

  while(p) {
    pid = fork(); // start new child process

    switch (pid) {
    case -1: // forking error case
      perror("fork");
      exit(1);
    case 0: // child code
      execvp(p->command, p->arguments);
      exit(0);
    default: // parent code
      prev = p;
      p = p->next;
      free(prev);
      
      c->pid = pid; // all process should be launched until waiting, pids are stored
      if (p)
	c->next = (child_t *) malloc(sizeof(child_t));
      c = c->next;
    }
  }

  while(head) { // we loop childs list for waiting
    waitpid(head->pid, &(head->status), 0);
    c = head;
    head = head->next;
    free(c);
  }

  // the moment code reaches this return zone, asserts all processes have ended 
  return 0;
}

void
startsh () {
  
  int nread;
  size_t n = 0;
  char *line = NULL;
  process_t *prcsslist;
  
  while (true) {
    if (INTERACTIVE) // when interactive mode activated, prompt gets printed
      printf(PROMPT);

    if ((nread = getline(&line, &n, INPUT)) == -1)
      return; // read untill EOF (interactive mode don't reach EOF, run exit)

    line = strsep(&line, "\n"); // deletes '\n' if found (in batchfiles last instruction could have no '\n'
    prcsslist = parseprcss(line);
    execprcss(prcsslist); // exec all processes in cmdline (parallelism implemented)
  }

  free(line);

  return;
}
