#include "parser.h"
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include "process.h"

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

process_t* parse_line(char* line) {
  process_t* head = NULL;
  process_t* proc = process_alloc(32);
  process_t* next;

  char* next_token = NULL;
  PSTATE s = PSTATE_IN_SPACE;

  for(char* p = line; *p; ++p) {
    char c = *p;

    /* eat whitespace */
    if (IN_SPACE(s) && isspace(c)) {
      continue;
    }

    if (IN_SPACE(s) && !isspace(c)) {
      switch(c) {
      case '\'':
      case '"':
        /* start of quoted token */
        next_token = (p + 1);
        s = (c == '"' ? PSTATE_IN_DQUOTE : PSTATE_IN_SQUOTE);
        break;
      case '|':
        /* start of new process */
        process_push_arg(proc, NULL);

        if (!head) {
          head = proc;
        }

        next = process_alloc(32);
        proc->next = next;
        proc = next;
        s = PSTATE_IN_SPACE;
        break;
      default:
        /* start of new simple token */
        next_token = p;
        s = PSTATE_IN_SIMPLE;
        break;
      }
    } else if ((IN_SIMPLE(s) && isspace(c))|| /* end of simple token */
               (IN_SQUOTE(s) && c == '\'') || /* end of squote token */
               (IN_DQUOTE(s) && c == '"')) {  /* end of dquote token */
      *p = '\0';
      process_push_arg(proc, next_token);
      next_token = NULL;
      s = PSTATE_IN_SPACE;
    } else if (IN_SIMPLE(s) && c == '|') {
      /* also start of a new process, with a trailing token */
      *p = 0;
      process_push_arg(proc, next_token);
      process_push_arg(proc, NULL);
      next_token = NULL;

      if (!head) {
        head = proc;
      }

      next = process_alloc(32);
      proc->next = next;
      proc = next;
      s = PSTATE_IN_SPACE;
    }
  }

  if (next_token) {
    process_push_arg(proc, next_token);
  }
  process_push_arg(proc, NULL);

  if (!head) {
    head = proc;
  }

  return head;
}
