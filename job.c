#include "job.h"
#include <stdlib.h>

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
