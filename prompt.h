#ifndef _ITSH_PROMPT_H_
#define _ITSH_PROMPT_H_


typedef struct prompt {
  char format[32];
  char value[512];
} prompt_t;


int  prompt_init(prompt_t* p);
void prompt_free(prompt_t* p);
void prompt_update(prompt_t* p);

#endif
