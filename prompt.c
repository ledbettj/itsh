#include "prompt.h"
#include <pwd.h>
#include <sys/utsname.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

static const char* DEFAULT_PROMPT = "\e[1;31m\\u@\\h:\e[0m\\w\e[0m %> ";

int prompt_init(prompt_t* p)
{
  snprintf(p->format, sizeof(p->format), "%s", DEFAULT_PROMPT);
  prompt_update(p);

  return 0;
}

void prompt_free(prompt_t* p)
{
  /* nothing to do */
}

void prompt_update(prompt_t* p)
{
  struct passwd* pwd = getpwuid(getuid());
  struct utsname unm;
  bool   special = false;
  char*  dest = p->value;
  int    left = sizeof(p->value) - 1;
  int    used = 0;

  char cwdbuf[512] = "\0";
  getcwd(cwdbuf, sizeof(cwdbuf));

  memset(&unm, 0, sizeof(unm));
  uname(&unm);
 
  /* this is still ugly. */
  for(char* c = p->format; *c && left > 0; ++c) {
    if (*c == '\\' && !special) {
      special = true;
    } else if (special) {
      special = false;
      switch(*c) {
      case 'h':
        used = snprintf(dest, left, "%s", unm.nodename);
        left -= used;
        dest += used;
        break;
      case 'u':
        used = snprintf(dest, left, "%s", pwd ? pwd->pw_name : "");
        left -= used;
        dest += used;
        break;
      case 'w':
        used = snprintf(dest, left, "%s", cwdbuf);
        left -= used;
        dest += used;
        break;
      default:
        *(dest++) = *c;
        left--;
        break;
      }
    } else {
      *(dest++) = *c;
      left--;
    }
  }

  *dest = '\0';

}
