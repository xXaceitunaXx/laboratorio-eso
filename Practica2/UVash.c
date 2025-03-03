#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define ERRUVASH "An error has occurred\n"
#define PROMPT "UVash> "
#define EXIT "exit"
#define SEPRCHR " "
#define REDRCHR '>'
#define PARLCHR "&"

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
void *clearprcss (process_t *);
char *clearwhites (char *);
char **createargv (char *);
process_t *parseprcss (char *);
bool builtin (process_t *);
void execprcss (process_t *);

/* ################################# */

int
main (int argc, char *argv[]) {

  if (argc > 2) {
    fprintf(stderr, ERRUVASH);
    return 1;
  }

  // if no file provided, interactive mode on
  INTERACTIVE = (argc == 1);
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

  if (!line || !*line)
    return NULL;

  while (*line == ' ' || *line == '\t')
    line++;

  if (!*line)
    return NULL;
  
  return line;
}

void
*clearprcss (process_t *head) {

  process_t *ptr;

  if (!head)
    return head;

  while(head) {
    ptr = head;
    head = head->next;
    free(ptr);
  }

  return head;
}

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
  
  while (arguments) {
    rfi->pstring = strsep(&arguments, SEPRCHR);
    argc++;

    if ((arguments = clearwhites(arguments))) {
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
  for (size_t i = 0; i < argc; i++) {
    argv[i] = refs->pstring;
    rfi = refs;
    refs = refs->next;
    free(rfi);
  }

  argv[argc] = NULL;
  
  return argv;
}

process_t
*parseprcss (char *cmdline) {

  char *command;
  char **argv;
  process_t *p, *head;

  if (*cmdline == '&') {
    fprintf(stderr, ERRUVASH);
    return NULL;
  }
  
  if (!(head = p = (process_t *) malloc(sizeof(process_t)))) {
    perror("malloc");
    exit(1);
  }

  while ((command = strsep(&cmdline, PARLCHR))) {

    argv = createargv(command); // arguments on *argv[] style
    command = strsep(&command, SEPRCHR); // separates command from all args

    p->command = command;
    p->arguments = argv;
    p->next = NULL;

    if ((cmdline = clearwhites(cmdline))) {
      if (!(p->next = (process_t *) malloc(sizeof(process_t)))) {
	perror("malloc");
	exit(1);
      }
    }

    p = p->next;
  }
  /* ------------------------------------------------------------------- */
  
  return head; // process linked list ready to execute via execprcss
}

void
owncd (char *arguments[]) {
  if (!arguments[1] || arguments[2]) {
    fprintf(stderr, ERRUVASH);
    return;
  }

  chdir(*(arguments + 1));

  return;
}

bool
builtin (process_t *p) {
  
  if (!strcmp(EXIT, p->command)) {
    if (p->arguments[1])
      fprintf(stderr, ERRUVASH);
    
    exit(0);
  }
  
  if (!strcmp("cd", p->command)) {
    owncd(p->arguments);
    return true;
  }
  
  return false;
}

/*
  TODO execprcss (process *)
  Must find a way to print result in the file, maybe it's easy?
*/
void
execprcss (process_t *p) {

  process_t *prev;
  pid_t pid;
  child_t *head, *c;

  if (!(head = c = (child_t *) malloc(sizeof(child_t)))) {
    perror("malloc");
    exit(1);
  }

  prev = p;
  while (prev) {
    
    if (!builtin(prev)) {
      pid = fork(); // start new child process
      switch (pid) {
      case -1: // forking error case
	perror("fork");
	exit(1);
      case 0: // child code
	if (execvp(prev->command, prev->arguments))
	  fprintf(stderr, ERRUVASH);
	
	fflush(stdout);
	fflush(stderr);
	exit(0);
      } // parent code
    }
    
    prev = prev->next;
      
    c->pid = pid; // all process should be launched until waiting, pids are stored
    if (prev)
      c->next = (child_t *) malloc(sizeof(child_t));
    c = c->next;
  }

  while(head) { // we loop childs list for waiting
    waitpid(head->pid, &(head->status), 0);
    c = head;
    head = head->next;
    free(c);
  }

  clearprcss(p);
  
  // the moment code reaches this return zone, asserts all processes have ended 
  return;
}

void
startsh () {

  int nread;
  size_t n = 0;
  char *line = NULL, *ptr;
  process_t *prcsslist = NULL;

  while (true) {
    if (INTERACTIVE) // when interactive mode activated, prompt gets printed
      printf(PROMPT);

    if ((nread = getline(&line, &n, INPUT)) == -1)
      return; // read untill EOF (interactive mode don't reach EOF, run exit)

    ptr = line;
    line = strsep(&line, "\n"); // deletes '\n' if found (in batchfiles last instruction could have no '\n'

    if ((line = clearwhites(line)))
      prcsslist = parseprcss(line);

    if (prcsslist)
      execprcss(prcsslist); // exec all processes in cmdline (parallelism implemented)
  }

  free(ptr);

  return;
}
