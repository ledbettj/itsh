#include "job.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

bool job_completed(job_t* j)
{
  for(process_t* p = j->procs; p; p = p->next) {
    if (!p->completed) {
      return false;
    }
  }

  return true;
}

bool job_stopped(job_t* j)
{
  for(process_t* p = j->procs; p; p = p->next) {
    if (!p->completed && !p->stopped) {
      return false;
    }
  }

  return true;
}

job_t* job_from_pgid(job_t* head, pid_t pgid)
{
  for(job_t* j = head; j; j = j->next) {
    if (j->pgid == pgid) {
      return j;
    }
  }
  return NULL;
}

job_t* job_alloc(void)
{
  return calloc(1, sizeof(job_t));
}

void job_free(job_t* j)
{
  free(j);
}

void job_launch(job_t* j)
{
  pid_t pid;
  int job_pipe[2], in, out;

  for(process_t* p = j->procs; p; p = p->next) {
    /* do we need to pipe our output to another process ? */
    if (p->next) {
      pipe(job_pipe);
      out = job_pipe[1];
    } else {
      out = j->stdout;
    }

    switch((pid = fork())) {
    case -1:
      perror("fork");
      exit(1);
    case 0:
      /* parent */
      p->pid = pid;
      if (!j->pgid) {
        j->pgid = pid;
        setpgid(pid, j->pgid);
      }

      if (in != j->stdin) {
        close(in);
      }
      if (out != j->stdout) {
        close(out);
      }
      in = job_pipe[0];

      break;
    default:
      /* child */
      process_launch(p, j->pgid, in, out, j->stderr);
      /* not reached */
      exit(0);
    }
  }
}
