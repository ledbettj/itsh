#include "process.h"
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include "builtin.h"

process_t* process_alloc(int max_args)
{
  process_t* p = calloc(1, sizeof(process_t));
  p->argm = max_args;
  p->argv = malloc(p->argm * sizeof(char*));

  return p;
}

void process_list_free(process_t* head)
{
  process_t* next = NULL, *p = head;

  while(p) {
    next = p->next;
    process_free(p);
    p = next;
  }
}

void process_free(process_t* p)
{
  for(int i = 0; i < p->argc; ++i) {
    // not yet!
    //free(p->argv[i]);
  }
  free(p->argv);
  free(p);
}

void process_push_arg(process_t* p, char* arg)
{
  if (p->argc == p->argm) {
    p->argm *= 2;
    p->argv = realloc(p->argv, p->argm * sizeof(char*));
  }
  p->argv[p->argc++] = arg;

  if (!arg) {
    /* if we're pushing a trailing NULL don't count it as an arg */
    p->argc--;
  }
}


void process_status(process_t* p, int status)
{
  p->status = status;
  if (WIFSTOPPED(status)) {
    p->stopped = true;
  } else {
    p->completed = true;
    if (WIFSIGNALED(status)) {
      /* it were a bloody death */
    }
  }
}

process_t* process_by_pid(process_t* first, pid_t pid)
{
  for(process_t* p = first; p; p = p->next) {
    if (p->pid == pid) {
      return p;
    }
  }
  return NULL;
}

void process_launch(process_t* p, pid_t pgid, int in, int out, int err)
{
  pid_t pid = getpid();

  pgid = pgid ? pgid : pid;

  setpgid(pid, pgid);

  tcsetpgrp(STDIN_FILENO, pgid);

  /* unblock job control signals */
  signal(SIGINT, SIG_DFL);
  signal(SIGQUIT, SIG_DFL);
  signal(SIGTSTP, SIG_DFL);
  signal(SIGTTIN, SIG_DFL);
  signal(SIGTTOU, SIG_DFL);
  signal(SIGCHLD, SIG_DFL);

  if (in != STDIN_FILENO) {
    dup2(in, STDIN_FILENO);
    close(in);
  }

  if (out != STDOUT_FILENO) {
    dup2(out, STDOUT_FILENO);
    close(out);
  }

  if (err != STDERR_FILENO) {
    dup2(err, STDERR_FILENO);
    close(err);
  }

  builtin_t* b;

  if ((b = builtin_lookup(p->argv[0], BFLAG_INCHILD | BFLAG_ENABLED))) {
    exit(b->callback(NULL, p->argc, (const char**)p->argv));
  } else {
    execvp(p->argv[0], p->argv);
    perror(p->argv[0]);
    exit (1);
  }
}
