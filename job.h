#ifndef _ITSH_JOB_H_
#define _ITSH_JOB_H_
#include "process.h"
#include <sys/types.h>
#include <stdbool.h>
#include <termios.h>

typedef struct job {
  struct job* next;
  process_t* procs;
  pid_t pgid;
  bool notified;
  struct termios tmodes;
  int in;
  int out;
  int err;
} job_t;

bool   job_completed(job_t* j);
bool   job_stopped(job_t* j);
job_t* job_by_pgid(job_t* head, pid_t pgid);
job_t* job_alloc(void);
void   job_free(job_t* j);
void   job_launch(job_t* j);
#endif
