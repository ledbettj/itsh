#include "builtin.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int builtin_cd(shell_t* sh, int argc, const char** argv);
int builtin_exit(shell_t* sh, int argc, const char** argv);
int builtin_help(shell_t* sh, int argc, const char** argv);

static builtin_t builtins[] = {
  {"cd",   "<path>",    builtin_cd},
  {"exit", "(value)",   builtin_exit},
  {"help", "(command)", builtin_help},
  {NULL, NULL, NULL}
};

builtin_t* builtin_lookup(const char* cmd)
{
  for(int i = 0; builtins[i].command != NULL; ++i) {
    if (!strcmp(builtins[i].command, cmd)) {
      return &builtins[i];
    }
  }

  return NULL;
}

void builtin_foreach(int(*on_each)(builtin_t* b)) {
  for(int i = 0; builtins[i].command != NULL; ++i) {
    if (on_each(&builtins[i])) {
      break;
    }
  }
}

/* TODO: handle special case '-' to change to previous directory */
int builtin_cd(shell_t* sh, int argc, const char** argv)
{
  if (argc == 2) {
    int rc;

    while((rc = chdir(argv[1])) == -1 && errno == EINTR);
    if (rc == -1) {
      perror(argv[1]);
    }
    return rc;
  } else {
    return 0;
  }
}

int builtin_exit(shell_t* sh, int argc, const char** argv)
{
  int rc = 0;
  if (argc > 1) {
    rc = atoi(argv[1]);
  }

  sh->running = false;
  return rc;
}

static int help_helper(builtin_t* b)
{
  printf("  %-12s %s\n", b->command, b->help);
  return 0;
}

int builtin_help(shell_t* sh, int argc, const char* argv[])
{
  if (argc == 2) {
    builtin_t* b = builtin_lookup(argv[1]);
    if (!b) {
      printf("no such command '%s'\n", argv[1]);
    } else {
      printf("%s %s\n", b->command, b->help);
    }
  } else {
    printf("commands: \n");
    builtin_foreach(help_helper);
    printf("\n");
  }
  return 0;
}
