#ifndef _ITSH_PARSER_H_
#define _ITSH_PARSER_H_

char** parse_args(char* line, int *num_args);
void   free_args(char** args);

#endif
