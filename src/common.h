#define PROD
#define LOG

#ifndef COMMON_H
#define COMMON_H

#define MAXCOMMANDS    20
#define MAXLINE 	 4096
#define MAXARGS		   20

#include <stdbool.h>

typedef struct {
	char *args[MAXARGS + 1];
	char *input;
	char *output;
	int in_pipe_index;
	int out_pipe_index;
} COMMAND;

#endif
