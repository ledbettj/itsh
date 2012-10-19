#include "parser.h"
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

typedef enum {
  PSTATE_IN_SQUOTE,
  PSTATE_IN_DQUOTE,
  PSTATE_IN_SIMPLE,
  PSTATE_IN_SPACE
} PSTATE;

#define IN_SQUOTE(state) (state == PSTATE_IN_SQUOTE)
#define IN_DQUOTE(state) (state == PSTATE_IN_DQUOTE)
#define IN_SIMPLE(state) (state == PSTATE_IN_SIMPLE)
#define IN_SPACE(state)  (state == PSTATE_IN_SPACE)

/* TODO: handle escape characters, environmental variables, and ~ paths */
/* TODO: handle malloc/realloc failure */
char** parse_args(char* line, int *num_args)
{
  int    argm = 32; /* maximum number of arguments we can currently hold */
  int    argc = 0;
  char** argv = malloc(sizeof(char*) * argm);
  PSTATE s    = PSTATE_IN_SPACE;

  char* next_token = NULL;

  for(char* p = line; *p; p++) {
    char c = *p;
    /* eat whitespace */
    if (IN_SPACE(s) && isspace(c)) {
      continue;
    }

    if (IN_SPACE(s) && !isspace(c)) {
      if (c == '"' || c == '\'') { /* start of quoted token */
        next_token = (p + 1);
        s = (c == '"') ? PSTATE_IN_DQUOTE : PSTATE_IN_SQUOTE;
      } else { /* start of simple token */
        next_token = p;
        s = PSTATE_IN_SIMPLE;
      }
    } else if ((IN_SIMPLE(s) && isspace(c))|| /* end of simple token */
               (IN_SQUOTE(s) && c == '\'') || /* end of squote token */
               (IN_DQUOTE(s) && c == '"')) {  /* end of dquote token */
      *p = '\0';
      argv[argc++] = next_token;
      next_token = NULL;
      if (argc == argm) {
        argm *= 2;
        argv = realloc(argv, sizeof(char*) * argm);
      }
      s = PSTATE_IN_SPACE;
    } else {
      /* nothing to do */
    }
  }

  /* make sure we have room for at least 2 more arguments (trailing + NULL) */
  if (argc + 2 > argm) {
    argm = argc + 2;
    argv = realloc(argv, sizeof(char*) * argm);
  }

  /* did we have a trailing token? */
  if (next_token) {
    argv[argc++] = next_token;
  }

  /* terminate the array with a NULL pointer for execvp */
  argv[argc] = NULL;

  *num_args = argc;
  return argv;
}

void free_args(char** args)
{
  /* the individual strings in this array point to memory that was all
   * allocated together. so no need to free it (whoever allocated it should
   * be responsible for that)
   */
  free(args);
}
