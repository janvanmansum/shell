/*
 ============================================================================
 Name        : shell2.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "shell2.h"

#ifndef TEST
int main(void) {
	char line[MAXLINE];
	COMMAND commands[MAXCOMMANDS];
	int blocking = 1;

	while(get_next_line(line)) {
		blocking = is_blocking_call(line);
		get_commands(line, &commands);
		execute_commands(&commands, blocking);
		reset_commands(&commands);
	}

	return EXIT_SUCCESS;
}
#endif

bool get_next_line(char line[]) {
	return fgets(line, MAXLINE, stdin) != NULL;
}

bool is_blocking_call(char line[]) {
	return false;
}

int get_commands(char line[], COMMAND (*commands)[]) {
	char *cmdlines[MAXCOMMANDS];
	int ncmd = split(line, "|", cmdlines);
	for(int i = 0; i < ncmd; ++i) {
		get_command(cmdlines[i], (*commands) + i);
	}
	return ncmd;
}

void get_command(char line[], COMMAND *command) {

}

void execute_commands(COMMAND (*commands)[], int blocking) {
	if (execute_builtin(*commands)) return;
	int i = 0;

	while ((*commands)[i].filled) {
		// create pipes
		//excecute_command(cmd, /* in en out pipe */);


		// Geef bij de laatste de waarde van blocking door, bij de rest altijd 0
	}
}

bool execute_builtin(COMMAND *cmd) {

	return false;
}

void execute_command(COMMAND *cmd, bool blocking, FILE* in_pipe, FILE* out_pipe) {


}

int split(char s[], const char *delim, char *parts[]) {

	return 0;
}

void reset_commands(COMMAND (*commands)[]) {
	for (int i = 0; i < MAXCOMMANDS; ++i)
		(*commands)[i].filled = false;
}
