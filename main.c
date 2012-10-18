#include <stdlib.h>
#include <stdio.h>
#include "shell.h"

int main(int argc, char* argv[])
{
  shell_t sh;
  int     rc;

  if (!(rc = shell_init(&sh))) {
    rc = shell_run(&sh);
    shell_free(&sh);
  }

  return rc;
}
