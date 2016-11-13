#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"

#define INPUT_REDIRECT 	0
#define OUTPUT_REDIRECT 1
#define ARG				2

static int split(char s[], const char *delim, char *parts[]);
static void get_command(char line[], COMMAND *command);
static int get_part_type(int index, char s[]) ;
static void reset_command(COMMAND *command);


int get_commands(char line[], COMMAND (*commands)[]) {
	char *cmdlines[MAXCOMMANDS];
	int ncmd = split(line, "|", cmdlines);
	for (int i = 0; i < ncmd; ++i) {
		get_command(cmdlines[i], (*commands) + i);
	}
	return ncmd;
}

static int split(char s[], const char *delim, char *parts[]) {
	char *p;
	int i = 0;
	p = strtok(s, delim);
	while (p != NULL) {
		if (parts != NULL)
			parts[i++] = p;
		p = strtok(NULL, delim);
	}
	if (parts != NULL)
		parts[i] = NULL;
	return i;
}

static void get_command(char line[], COMMAND *command) {
	char line2[MAXLINE + 1];
	char *parts[MAXARGS + 3]; // MAXARGS + 1 input + 1 output + NULL
	int iargs = 0;
	reset_command(command);
	strcpy(line2, line);
	split(line, " \t><", parts);
	for (int i = 0; parts[i] != NULL; ++i) {
		int index = parts[i] - line;
		switch (get_part_type(index, line2)) {
		case INPUT_REDIRECT:
			command->input = parts[i];
			break;
		case OUTPUT_REDIRECT:
			command->output = parts[i];
			break;
		case ARG:
			command->args[iargs++] = parts[i];
			break;
		}
	}
	command->args[iargs] = 0;
}

void reset_commands(COMMAND (*commands)[]) {
	for (int i = 0; i < MAXCOMMANDS; ++i)
		reset_command(*commands + i);
}

static void reset_command(COMMAND *command) {
	command->args[0] = NULL;
	command->in_pipe_index = -1;
	command->out_pipe_index = -1;
	command->input = NULL;
	command->output = NULL;
}

static int get_part_type(int index, char s[]) {
	for (int i = index; i != -1; --i) {
		switch (s[i]) {
		case '<':
			return INPUT_REDIRECT;
		case '>':
			return OUTPUT_REDIRECT;
		}
	}
	return ARG;
}

bool is_blank(char line[]) {
	for (char *p = line; *p != 0; ++p)
		if (isspace(*p))
			continue;
		else
			return false;

	return true;
}

bool is_blocking_call(char line[]) {
	char *p;

	if ((p = strrchr(line, '&')) != NULL) {
		*p = '\0';
		return false;
	}

	return true;
}
