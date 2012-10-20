#ifndef _ITSH_PROCESS_H_
#define _ITSH_PROCESS_H_
#include <stdbool.h>
#include <sys/types.h>

typedef struct process {
  struct process* next;
  char** argv;
  int argc;
  int argm;
  pid_t pid;
  bool completed;
  bool stopped;
  int status;
} process_t;

process_t* process_alloc(int max_args);
void process_free(process_t* p);
void process_list_free(process_t* first);
void process_push_arg(process_t* p, char* arg);
void process_status(process_t* p, int status);
void process_launch(process_t* p, pid_t pgid, int in, int out, int err);
process_t* process_by_pid(process_t* first, pid_t pid);

#endif
