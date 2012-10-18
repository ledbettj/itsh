#ifndef _ITSH_BUILTIN_H_
#define _ITSH_BUILTIN_H_
#include <stdbool.h>
#include "shell.h"

typedef struct {
  const char* command;
  const char* help;
  int(*callback)(shell_t* sh, int argc, const char** argv);
} builtin_t;

builtin_t* builtin_lookup(const char* cmd);
void builtin_foreach(int(*on_each)(builtin_t* b));

#endif
