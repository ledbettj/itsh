#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "shell.h"
#include "parser.h"
#include "builtin.h"

static int   shell_execute      (shell_t* sh, process_t* proclist);
static char* shell_readline     (shell_t* sh);
static int   shell_doline       (shell_t* sh, char* line);
static void  shell_job_wait     (shell_t* sh, job_t* j);

int shell_init(shell_t* sh)
{
  memset(sh, 0, sizeof(*sh));
  prompt_init(&sh->prompt);
  sh->terminal = STDIN_FILENO;

  /* wait until we're in the foreground */
  while(tcgetpgrp(sh->terminal) != (sh->pgid = getpgrp())) {
    kill(-sh->pgid, SIGTTIN);
  }

  /* ignore job control signals */
  signal(SIGINT,  SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);
  signal(SIGTTOU, SIG_IGN);

  /* we need to be in charge of our own process group */
  sh->pgid = getpid();
  if (setpgid(sh->pgid, sh->pgid) < 0) {
    perror("set process group");
    return -1;
  }

  tcsetpgrp(sh->terminal, sh->pgid);
  tcgetattr(sh->terminal, &sh->tmodes);

  return 0;
}

int shell_run(shell_t* sh)
{
  char* line = NULL;
  sh->running = true;

  while(sh->running && (line = shell_readline(sh))) {
    if (line[0]) {
      shell_doline(sh, line);
    }
    free(line);
  }

  return sh->last_exit;
}

void shell_free(shell_t* sh)
{
  /* restore original termios modes */
  tcsetattr(sh->terminal, 0, &sh->tmodes);
  prompt_free(&sh->prompt);
  return;
}

static char* shell_readline(shell_t* sh)
{
  prompt_update(&sh->prompt);
  return readline(sh->prompt.value);
}


static int shell_doline(shell_t* sh,char* line)
{
  add_history(line);

  process_t* p = parse_line(line);
  builtin_t* b = NULL;

  if (p->argc) {
    /* single command; test for builtin */
    if (!p->next && (b = builtin_lookup(p->argv[0], BFLAG_INPARENT | BFLAG_ENABLED))) {
      sh->last_exit = b->callback(sh, p->argc, (const char**)p->argv);
    } else {
      shell_execute(sh, p);
    }
  }

  process_list_free(p);

  return 0;
}

static void shell_job_to_fg(shell_t* sh, job_t* j)
{
  /* job to fg */
  tcsetpgrp(sh->terminal, j->pgid);

  shell_job_wait(sh, j);

  /* shell back to fg */
  tcsetpgrp(sh->terminal, sh->pgid);

  tcgetattr(sh->terminal, &j->tmodes);
  tcsetattr(sh->terminal, TCSADRAIN, &sh->tmodes);
}

static int shell_execute(shell_t* sh, process_t* proclist)
{
  /* for now, job = process and it's always in the foreground. */
  job_t* j = job_alloc();
  j->in  = STDIN_FILENO;
  j->out = STDOUT_FILENO;
  j->err = STDERR_FILENO;

  j->procs = proclist;
  sh->jobs = j;

  job_launch(j);

  shell_job_to_fg(sh, j);

  job_free(j);
  return 0;
}

void shell_job_wait(shell_t* sh, job_t* j)
{
  pid_t pid;
  int status;
  process_t* p;

  while(!job_stopped(j) && !job_completed(j)) {
    if ((pid = waitpid(WAIT_ANY, &status, WUNTRACED)) > 0) {
      for(job_t* iter = sh->jobs; iter; iter = iter->next) {
        if ((p = process_by_pid(iter->procs, pid))) {
          process_status(p, status);
        }
      }
    } else if (errno != ECHILD && pid != 0) {
      perror("waitpid");
    }
  }

}
