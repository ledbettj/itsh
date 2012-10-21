#ifndef _ITSH_SHELL_H_
#define _ITSH_SHELL_H_
#include <stdbool.h>
#include <sys/types.h>
#include <termios.h>
#include "job.h"
#include "prompt.h"

typedef struct {
  prompt_t prompt;
  int last_exit;
  bool running;
  pid_t pgid;
  int   terminal;
  struct termios tmodes;
  job_t* jobs;
} shell_t;

int  shell_init(shell_t* sh);
int  shell_run(shell_t* sh);
void shell_free(shell_t* sh);

#endif
