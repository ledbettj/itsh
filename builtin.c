#include "builtin.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>

int builtin_cd(shell_t* sh, int argc, const char** argv);
int builtin_exit(shell_t* sh, int argc, const char** argv);
int builtin_help(shell_t* sh, int argc, const char** argv);
int builtin_ls(shell_t* sh, int argc, const char** argv);
int builtin_builtin(shell_t* sh, int argc, const char** argv);

static builtin_t builtins[] = {
  {"cd",   "<path>",    BFLAG_ENABLED | BFLAG_INPARENT, builtin_cd},
  {"exit", "(value)",   BFLAG_ENABLED | BFLAG_INPARENT, builtin_exit},
  {"help", "(command)", BFLAG_ENABLED | BFLAG_INCHILD,  builtin_help},
  {"ls",   "(path)",    BFLAG_ENABLED | BFLAG_INCHILD,  builtin_ls},
  {"builtin", "<command> <on|off>", BFLAG_ENABLED | BFLAG_INPARENT, builtin_builtin},
  {NULL, NULL, BFLAG_NONE, NULL}
};

builtin_t* builtin_lookup(const char* cmd, uint16_t mask)
{
  builtin_t* b;
  for(int i = 0; builtins[i].command != NULL; ++i) {
    b = &builtins[i];
    if (((b->flags & mask) == mask) && !strcmp(b->command, cmd)) {
      return b;
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
    builtin_t* b = builtin_lookup(argv[1], BFLAG_NONE);
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


int builtin_ls(shell_t* sh, int argc, const char** argv)
{
  const char* path = (argc == 2 ? argv[1] : ".");
  DIR* d;
  struct dirent* ent;

  if ((d = opendir(path))) {
    while((ent = readdir(d))) {
      if (ent->d_name[0] == '.') {
        continue;
      }

      printf("%s\n", ent->d_name);
    }
    closedir(d);
  } else {
    perror(path);
    return 1;
  }

  return 0;
}

int builtin_builtin(shell_t* sh, int argc, const char** argv)
{
  if (argc < 2) {
    fprintf(stderr, "usage: builtin <cmd> (on|off)\n");
    return -1;
  }

  builtin_t* b = builtin_lookup(argv[1], BFLAG_NONE);

  if (!b) {
    fprintf(stderr, "no such builtin: %s\n", argv[1]);
    return -1;
  }

  if (argc < 3) {
    printf("builtin %s is currently %sabled\n", argv[1],
           b->flags & BFLAG_ENABLED ? "en" : "dis");
    return 0;
  }

  bool enable  = !strcmp(argv[2], "on");

  if (enable && b->flags & BFLAG_ENABLED) {
    fprintf(stderr, "builtin %s is already enabled\n", argv[1]);
    return -1;
  }

  if (!enable && !(b->flags & BFLAG_ENABLED)) {
    fprintf(stderr, "builtin %s is already disabled\n", argv[1]);
    return -1;
  }

  if (enable) {
    b->flags |= BFLAG_ENABLED;
    printf("now using builtin %s\n", argv[1]);
  } else {
    b->flags &= ~(BFLAG_ENABLED);
    printf("no longer using builtin %s\n", argv[1]);
  }

  return 0;
}
