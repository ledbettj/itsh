#ifndef _ITSH_PROCESS_H_
#define _ITSH_PROCESS_H_
#include <stdbool.h>

typedef struct process {
  struct process* next;
  char** argv;
  int argc;
  bool completed;
  bool stopped;
  int status;
} process_t;

#endif
