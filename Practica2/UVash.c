#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define ERRUVASH "An error has occurred\n"

#define PROMPT "UVash> "

#define EXIT "exit"
#define CD "cd"

#define SEPRCHR " \t"
#define REDRCHR ">"
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
void *clearprcss (process_t *);
char *clearwhites (char *);
char **createargv (char *);
process_t *parseprcss (char *);
bool builtin (process_t *);
void redirection (char *);
void execprcss (process_t *);
void startsh ();

/* ################################# */
  
/**
 * Utils function, opens a stream given the file name. Manages fopen errors.abort
 * 
 * @param filename name of the file to open
 * @return stream to the file
 */
FILE
*openfile (char *filename) {

  FILE *file;

  if (!(file = fopen(filename, "r"))) {
    fprintf(stderr, ERRUVASH);
    exit(1);
  }

  return file;
}

/**
 * Utils function. Could be replaced with iterations of strsep, but i liked it this way
 */
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

/**
 * Safely deletes a "execution block" queue
 * 
 * @param head head of the queue to delete
 */
void
*clearprcss (process_t *head) {

  process_t *ptr;

  if (!head)
    return head;

  while(head) {
    ptr = head;
    if (head->arguments)
      free(ptr->arguments);
    head = head->next;
    free(ptr);
  }

  return head;
}

/**
 * Creates C argv like for a command. Supports indefinite arguments.
 * 
 * @param arguments plain text arguments
 * @return argv like arguments
 */
char
**createargv (char *arguments) {

  char **argv = NULL;
  size_t argc = 0;
  argument_t *refs, *rfi;
  
  if (!arguments) // empty command
    return argv;

  refs = rfi = (argument_t *) malloc(sizeof(argument_t));
  
  while (arguments) {
    rfi->pstring = strsep(&arguments, SEPRCHR); // tokenize by " \t"
    argc++;

    if ((arguments = clearwhites(arguments))) {
      rfi->next = (argument_t *) malloc(sizeof(argument_t));
      rfi = rfi->next;
    }
  } // need linked list or all references get lost, strsep just inserts '\0'

  argv = (char **) malloc(sizeof(char **) * (argc + 1));

  // go through arguments list freeing nodes while storing the pointers
  for (size_t i = 0; i < argc; i++) {
    argv[i] = refs->pstring;
    rfi = refs;
    refs = refs->next;
    free(rfi);
  }

  argv[argc] = NULL; // asserts NULL at the end of commands
  
  return argv;
}

/**
 * Parses the command line recived from imput. Supports indefinite commands to paralellise.
 * 
 * @param cmdline command line input
 * @return an execution block queue ready for paralellism
 */
process_t
*parseprcss (char *cmdline) {

  char *command, *output;
  char **argv;
  process_t *p, *head;

  if (*cmdline == '&') { // base case of paralellism error, "& [commands ...]""
    fprintf(stderr, ERRUVASH);
    return NULL;
  }
  
  head = p = (process_t *) malloc(sizeof(process_t));

  while ((command = strsep(&cmdline, PARLCHR))) { // tokenize by &

    output = command;
    strsep(&output, REDRCHR);

    if (!clearwhites(command)) {
      fprintf(stderr, ERRUVASH);
      return clearprcss(head);
    }

    if (output) {
      output = clearwhites(output);
      p->output = strsep(&output, SEPRCHR);

      if (p->output == output || clearwhites(output)) {
	      fprintf(stderr, ERRUVASH);
	      return clearprcss(head);
      }
    } else
      p->output = NULL;
    
    argv = createargv(command); // arguments on *argv[] style
    command = strsep(&command, SEPRCHR); // separates command from all args

    p->command = command;
    p->arguments = argv;
    p->next = NULL;

    if ((cmdline = clearwhites(cmdline)))
      p->next = (process_t *) malloc(sizeof(process_t));

    p = p->next;
  }
  
  /* ------------------------------------------------------------------- */
  
  return head; // process linked list ready to execute via execprcss
}

/**
 * Simple cd implementation
 * 
 * @param arguments cd arguments
 */
void
owncd (char *arguments[]) {
  if (!arguments[1] || arguments[2]) {
    fprintf(stderr, ERRUVASH);
    return;
  }

  chdir(*(arguments + 1));

  return;
}

/**
 * Executes, if the command in p is a builtin, the corresponding one
 * 
 * @param p execution block
 * @return if the command was a builtin
 */
bool
builtin (process_t *p) {
  
  if (!strcmp(EXIT, p->command)) { // exit
    if (p->arguments[1])
      fprintf(stderr, ERRUVASH);
    
    exit(0);
  }
  
  if (!strcmp(CD, p->command)) { // cd
    owncd(p->arguments);
    return true;
  }
  
  return false;
}

/**
 * Redirects standar output (stdout) and standar error (stderr) to a given file, if 
 * file not exist, it gets created.
 * 
 * @param filename Route to the new output file
 */
void
redirection (char *filename) {

  int output;
  
  if ((output = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1) {
    perror(filename);
    exit(1);
  }

  if (dup2(output, 1) == -1) { // 
    perror("dup2");
    exit(1);
  }

  if (dup2(output, 2) == -1) {
    perror("dup2");
    exit(1);
  }

  close(output);
  return;
}

/**
 * Executes an "execution block" (process_t queue). There is no need to free p, as
 * the function handless both succes or failure memory management
 * 
 * @param p Pointer to the "execution block", can't be NULL
 */
void
execprcss (process_t *p) {

  process_t *prev;
  pid_t pid;
  child_t *head, *c;
  
  head = c = (child_t *) malloc(sizeof(child_t));

  prev = p;
  while (prev) {
    
    if (!builtin(prev)) {
      pid = fork(); // start new child process
      switch (pid) {
      case -1: // forking error case
	      perror("fork");
	      exit(1);
      case 0: // child code
        if (prev->output) // handle file redirection
          redirection(prev->output);
        
        execvp(prev->command, prev->arguments);
        
        // program reaches this section only if an error ocurred on execvp invocation
        // (command to execute not found)

        fprintf(stderr, ERRUVASH);
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

/**
 * Main loop of the UVa shell, asks and waits for input, then executes.
 * To be especific, these steps are followed:
 * 1. Gets input
 * 2. Parses the line, creating an execution block
 * 3. Executes the block, ensuring paralellism
 * 
 * In batch mode, the loop can be scaped whenever the EOF is reached, but interactive
 * needs to invoke EXIT command (unless input is redirected on execution "./UVash < input")
 */
void
startsh () {

  int nread;
  size_t n = 0;
  char *line = NULL, *aux;
  process_t *prcsslist = NULL;

  while (true) {
    if (INTERACTIVE) // when interactive mode activated, prompt gets printed
      printf(PROMPT);

    if ((nread = getline(&line, &n, INPUT)) == -1)
      exit(0); // read untill EOF (interactive mode don't reach EOF, use exit)

    aux = line;

    line = strsep(&line, "\n"); // deletes '\n' if found (in batchfiles last instruction could have no '\n'

    if ((line = clearwhites(line))) // only begin parsing if there is an instruction
      prcsslist = parseprcss(line);

    if (prcsslist)
      execprcss(prcsslist); // exec all processes in cmdline (parallelism implemented)
  }
    free(aux);
}

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
