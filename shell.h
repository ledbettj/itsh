#ifndef _ITSH_SHELL_H_
#define _ITSH_SHELL_H_
#include <stdbool.h>
#include <sys/types.h>
#include <termios.h>
#include "job.h"

/* TODO: clean up this clusterfk */
typedef struct prompt {
  int  uid;
  char username[32];
  char hostname[32];
  char format[32];
  char value[512];
} prompt_t;

typedef struct {
  prompt_t prompt;
  int last_exit;
  bool running;
  pid_t pgid;
  struct termios tmodes;
  job_t* jobs;
} shell_t;

int  shell_init(shell_t* sh);
int  shell_run(shell_t* sh);
void shell_free(shell_t* sh);

#endif
