#include "process.h"
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void process_launch(process_t* p, pid_t pgid, int in, int out, int err)
{
  pid_t pid;

  pid = getpid ();
  if (pgid == 0) pgid = pid;
  setpgid (pid, pgid);

  tcsetpgrp (STDIN_FILENO, pgid);

  /* unblock job control signals */
  signal (SIGINT, SIG_DFL);
  signal (SIGQUIT, SIG_DFL);
  signal (SIGTSTP, SIG_DFL);
  signal (SIGTTIN, SIG_DFL);
  signal (SIGTTOU, SIG_DFL);
  signal (SIGCHLD, SIG_DFL);

  if (in != STDIN_FILENO) {
    dup2 (in, STDIN_FILENO);
    close (in);
  }

  if (out != STDOUT_FILENO) {
    dup2 (out, STDOUT_FILENO);
    close (out);
  }

  if (err != STDERR_FILENO) {
    dup2 (err, STDERR_FILENO);
    close (err);
  }

  execvp (p->argv[0], p->argv);
  perror (p->argv[0]);
  exit (1);
}
