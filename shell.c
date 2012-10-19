#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
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

int shell_init(shell_t* sh)
{
  memset(sh, 0, sizeof(sh));
  shell_prompt_init(&sh->prompt);
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
  /* nothing to do (yet) */
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


static int shell_execute(shell_t* sh, int argc, char** argv)
{
  pid_t pid;
  int status = -1, rc;

  switch((pid = fork())) {
  case -1:
    perror("fork");
    return -1;
  case 0:
    /* child */
    if (execvp(argv[0], argv) == -1) {
      perror(argv[0]);
      exit(-1);
    }
  default:
    while((rc = (waitpid(pid, &status, 0)) == -1
           && errno == EINTR)) /* retry */;
    sh->last_exit = WEXITSTATUS(status);
    return 0;
  }
}
