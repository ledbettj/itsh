#ifndef _ITSH_PROCESS_H_
#define _ITSH_PROCESS_H_
#include <stdbool.h>
#include <sys/types.h>

typedef struct process {
  struct process* next;
  char** argv;
  int argc;
  pid_t pid;
  bool completed;
  bool stopped;
  int status;
} process_t;

void process_launch(process_t* p, pid_t pgid, int in, int out, int err);

#endif
