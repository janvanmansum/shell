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
#include <string.h>
#include "shell2.h"

#ifndef TEST
int main(void) {
	char line[MAXLINE];
	COMMAND commands[MAXCOMMANDS];
	bool blocking = true;

	prompt();
	while(get_next_line(line)) {
		blocking = is_blocking_call(line);
		get_commands(line, &commands);
		execute_commands(&commands, blocking);
		reset_commands(&commands);
		prompt();
	}

	return EXIT_SUCCESS;
}
#endif

void prompt(void) {
	printf("$ ");
}

bool get_next_line(char line[]) {
	if(fgets(line, MAXLINE, stdin) != NULL) {
		line[strlen(line) -1] = 0;
		return true;
	}
	return false;
}

bool is_blocking_call(char line[]) {
	char *p;

	if ((p = strrchr(line, '&')) != NULL) {
		*p = '\0';
		return false;
	}

	return true;
}

int get_commands(char line[], COMMAND (*commands)[]) {
	char *cmdlines[MAXCOMMANDS];
	int ncmd = split(line, "|", cmdlines);
	for(int i = 0; i < ncmd; ++i) {
		get_command(cmdlines[i], (*commands) + i);
	}
	return ncmd;
}

int split(char s[], const char *delim, char *parts[]) {
	char *p;
	int i = 0;
	p = strtok(s, delim);
	while(p != NULL) {
		if (parts != NULL) parts[i++] = p;
		p = strtok(NULL, delim);
	}
	if (parts != NULL) parts[i] = NULL;
	return i;
}


void get_command(char line[], COMMAND *command) {
	char line2[MAXLINE + 1];
	char *parts[MAXARGS + 3]; // MAXARGS + 1 input + 1 output + NULL
	int iargs = 0;
	reset_command(command);
	strcpy(line2, line);
	split(line, " \t><", parts);
	for(int i = 0; parts[i] != NULL; ++i) {
		int index = parts[i] - line;
		switch(get_part_type(index, line2)) {
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
	command->filled = true;
}

int get_part_type(int index, char s[]) {
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


void execute_commands(COMMAND (*commands)[], bool blocking) {
	if (execute_builtin(*commands)) return;
	int i = 0;

	// create pipes
	while ((*commands)[i].filled) {
		COMMAND *cmd = *commands + i;
		execute_command(cmd, blocking, 0, 0);
		++i;
	}
}

bool execute_builtin(COMMAND *cmd) {

	return false;
}

int run_command(char *args[], bool block);

void execute_command(COMMAND *cmd, bool blocking, int in_filedes, int out_filedes) {
	run_command(cmd->args, blocking);
}

void exec_command(char *args[]);

int run_command(char *args[], bool block) {
	pid_t pid;
	int status = 0;

	if (block) {
		/*
		 * Als het proces gewoon op de voorgrond kan draaien, hoeven we maar één keer te forken en wachten we daarna
		 * gewoon op het kind-proces.
		 */
		if ((pid = fork()) < 0)
			fprintf(stderr, "Fork error");
		else if (pid == 0) {
			/*
			 * Hier zijn we in het kind-proces.
			 */
			exec_command(args);
		}

		/*
		 * Hier zijn we in het de ouder (de shell).
		 */
		if (waitpid(pid, &status, 0) != pid)
			fprintf(stderr, "Error waiting for command to finish: %s", args[0]);

	} else {
		/*
		 * Fork twee keer om zombieproces te voorkomen. Uit "Advanced Programming in the Unix Environment"
		 * 1e Editie, par 8.6, blz. 202.
		 */
		if ((pid = fork()) < 0)
			fprintf(stderr, "Fork error");
		else if (pid == 0) {
			/*
			 * Hier zijn we in het kind-proces. De enige taak van dit proces is het starten van een kleinkind-proces.
			 * Daarna exit het. Doordat het kleinkind daardoor een "wees" wordt, maakt het besturingssysteem het init-
			 * proces de ouder van het kleinkind. Het init-proces zal altijd de status van een proces ophalen, wanneer
			 * dat proces exit. Een proces dat afgesloten is, maar er nog op wacht dat zijn status wordt opgehaald,
			 * wordt een "zombie" genoemd.
			 */
			if ((pid = fork()) < 0)
				fprintf(stderr, "Fork error");

			else if (pid > 0)
				exit(0);

			/*
			 * Hier zijn we in het kleinkind-proces. Het enige wat we hoeven te doen is het gevraagde commando uit te
			 * voeren. Als dat niet lukt, sluiten we af met een foutcode.
			 */
			exec_command(args);
		}

		/*
		 * Hier zijn we in de grootouder (de shell). We wachten wel op het kind-proces, anders zou dat een "zombie"
		 * worden.
		 */
		if (waitpid(pid, NULL, 0) != pid)
			fprintf(stderr, "Error waiting for background starter process");
	}

	return status;
}

void print_array(char *a[]) {
	printf("{ ");
	fflush(stdout);
	while (*a != NULL) {
		printf("%d", (int)a);
		printf("'%s', ", *a);
		fflush(stdout);
		++a;
	}
	printf(" }");
	fflush(stdout);
}

void debug(char *msg) {
	printf("%s\n", msg);
	fflush(stdout);
}

void exec_command(char *args[]) {
	execvp(args[0], args);
	fprintf(stderr, "Could not execute command: %s\n", args[0]);
	exit(127);
}

void reset_commands(COMMAND (*commands)[]) {
	for (int i = 0; i < MAXCOMMANDS; ++i)
		reset_command(*commands + i);
}

void reset_command(COMMAND *command) {
	command->filled = false;
	command->args[0] = NULL;
	command->input = NULL;
	command->output = NULL;
}
