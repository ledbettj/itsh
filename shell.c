#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "shell.h"
#include "parser.h"
#include "builtin.h"

static void  shell_prompt_init  (prompt_t* p);
static void  shell_prompt_update(prompt_t* p);
static int   shell_execute      (shell_t* sh, int argc, char** argv);
static char* shell_readline     (shell_t* sh);
static int   shell_doline       (shell_t* sh, char* line);
static void  shell_job_wait     (shell_t* sh, job_t* j);
int shell_init(shell_t* sh)
{
  memset(sh, 0, sizeof(sh));
  shell_prompt_init(&sh->prompt);
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

  return;
}

static char* shell_readline(shell_t* sh)
{
  shell_prompt_update(&sh->prompt);
  return readline(sh->prompt.value);
}


static int shell_doline(shell_t* sh,char* line)
{
  add_history(line);

  int argc = 0;
  char** args = parse_args(line, &argc);
  builtin_t* b;
  if ((b = builtin_lookup(args[0]))) {
    sh->last_exit = b->callback(sh, argc, (const char**)args);
  } else {
    shell_execute(sh, argc, args);
  }

  return 0;
}

static void shell_prompt_init(prompt_t* p)
{
  p->uid = getuid();

  struct passwd* pw = getpwuid(p->uid);

  gethostname(p->hostname, sizeof(p->hostname));

  snprintf(p->format,   sizeof(p->format),   "%s", "%u@%h:%w$ ");
  snprintf(p->username, sizeof(p->username), "%s",
           pw ? pw->pw_name : "unknown");
}

/* TODO: make this not hideous */
static void shell_prompt_update(prompt_t* p)
{
  bool special = false;
  char* putat = p->value;
  char cwdbuf[512];

  getcwd(cwdbuf, sizeof(cwdbuf));

  /* TODO: make sure we don't overflow the value buffer */
  for(char* c = p->format; *c; ++c) {
    if (*c == '%' && !special) {
      special = true;
    } else if (special) {
      switch(*c) {
      case 'h':
        putat += sprintf(putat, "%s", p->hostname);
        break;
      case 'u':
        putat += sprintf(putat, "%s", p->username);
        break;
      case 'w':
        putat += sprintf(putat, "%s", cwdbuf);
        break;
      default:
        *(putat++) = *c;
        break;
      }
      special = false;
    } else {
      *(putat++) = *c;
    }
  }
  *putat = '\0';
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

static int shell_execute(shell_t* sh, int argc, char** argv)
{
  /* for now, job = process and it's always in the foreground. */
  job_t* j = job_alloc();
  j->stdin  = STDIN_FILENO;
  j->stdout = STDOUT_FILENO;
  j->stderr = STDERR_FILENO;

  process_t* p = calloc(1, sizeof(process_t));
  p->argc = argc;
  p->argv = argv;

  j->procs = p;
  sh->jobs = j;

  job_launch(j);

  shell_job_to_fg(sh, j);

  job_free(j);
  free(p);
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
