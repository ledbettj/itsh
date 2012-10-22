#ifndef _ITSH_BUILTIN_H_
#define _ITSH_BUILTIN_H_
#include <stdbool.h>
#include <inttypes.h>
#include "shell.h"

typedef struct {
  const char* command;
  const char* help;
  uint16_t flags;
  int(*callback)(shell_t* sh, int argc, const char** argv);
} builtin_t;

#define BFLAG_NONE     (0)
#define BFLAG_INPARENT (1 << 0) /* cannot be run in a child process */
#define BFLAG_INCHILD  (1 << 1) /* should be run in a child process */

builtin_t* builtin_lookup(const char* cmd, uint16_t mask);
void builtin_foreach(int(*on_each)(builtin_t* b));

#endif
