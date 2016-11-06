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
#include <fcntl.h>
#include "shell2.h"

#ifndef TEST
int main(void) {
	char line[MAXLINE];
	COMMAND commands[MAXCOMMANDS];
	bool blocking = true;
	int ncmd = 0;

	prompt();
	while (get_next_line(line)) {
		if (is_blank(line)) {
			prompt();
			continue;
		}
		blocking = is_blocking_call(line);
		ncmd = get_commands(line, &commands);
		execute_commands(&commands, ncmd, blocking);
		reset_commands(&commands);
		prompt();
	}
	exit_shell();

	return EXIT_SUCCESS;
}
#endif

void prompt(void) {
	printf("$ ");
}

bool get_next_line(char line[]) {
	if (fgets(line, MAXLINE, stdin) != NULL) {
		line[strlen(line) - 1] = 0;
		return true;
	}
	return false;
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

int get_commands(char line[], COMMAND (*commands)[]) {
	char *cmdlines[MAXCOMMANDS];
	int ncmd = split(line, "|", cmdlines);
	for (int i = 0; i < ncmd; ++i) {
		get_command(cmdlines[i], (*commands) + i);
	}
	return ncmd;
}

int split(char s[], const char *delim, char *parts[]) {
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

void get_command(char line[], COMMAND *command) {
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

void execute_commands(COMMAND (*commands)[], int ncmd, bool blocking) {
	if (execute_builtin(*commands))
		return;
	int fds[2];

	for (int i = 0; i < ncmd; ++i) {
		COMMAND *cmd = *commands + i;
		if (i > 0) {
			pipe(fds);
			cmd->in_pipe[0] = fds[0];
			cmd->in_pipe[1] = fds[1];
			cmd->in_pipe_set = true;
			COMMAND *prev_cmd = *commands + (i - 1);
			prev_cmd->out_pipe[0] = fds[0];
			prev_cmd->out_pipe[1] = fds[1];
			prev_cmd->out_pipe_set = true;
		}
	}

	for (int i = ncmd - 1; i != -1; --i) {
		COMMAND *cmd = *commands + i;
		execute_command(cmd, i == 0 && blocking);
	}
}

bool execute_builtin(COMMAND *cmd) {
	char path[MAXLINE];
	bool command_found = false;

	if (!strcmp(cmd->args[0], "exit"))
		exit_shell();
	else if (!strcmp(cmd->args[0], "cd")) {
		if (cmd->args[1]) {
			if (cmd->args[1][0] != '/' && cmd->args[1][0] != '.') {
				strcpy(path, "./");
				strcat(path, cmd->args[1]);
			} else
				strcpy(path, cmd->args[1]);
		} else
			strcpy(path, "");

		if (chdir(path) == -1) {
			fprintf(stderr, "Cannot change directory to %s\n", path);
		}
		command_found = true;
	}

	return command_found;
}

void exit_shell(void) {
	printf("Good-bye!\n");
	exit(0);
}

int execute_command(COMMAND *cmd, bool block) {
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
			exec_command(cmd);
		}

		if (cmd->in_pipe_set) {
			close(cmd->in_pipe[0]);
			close(cmd->in_pipe[1]);
		}

		if (cmd->out_pipe_set) {
			close(cmd->out_pipe[0]);
			close(cmd->out_pipe[1]);
		}

		/*
		 * Hier zijn we in het de ouder (de shell).
		 */
		if (waitpid(pid, &status, 0) != pid)
			fprintf(stderr, "Error waiting for command to finish: %s",
					cmd->args[0]);

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
			exec_command(cmd);
		}

		if (cmd->in_pipe_set) {
			close(cmd->in_pipe[0]);
			close(cmd->in_pipe[1]);
		}

		if (cmd->out_pipe_set) {
			close(cmd->out_pipe[0]);
			close(cmd->out_pipe[1]);
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

void exec_command(COMMAND *cmd) {
	if (cmd->in_pipe_set) {
		close(cmd->in_pipe[1]);
		dup2(cmd->in_pipe[0], STDIN_FILENO);
		close(cmd->in_pipe[0]);
	} else if (cmd->input != NULL) {
		int in_filedes = open(cmd->input, O_RDONLY);
		dup2(in_filedes, STDIN_FILENO);
		close(in_filedes);
	}

	if (cmd->out_pipe_set) {
		close(cmd->out_pipe[0]);
		dup2(cmd->out_pipe[1], STDOUT_FILENO);
		close(cmd->out_pipe[1]);
	} else if (cmd->output != NULL) {
		printf("output-file for %s = %s\n", cmd->args[0], cmd->output);
		fflush(stdout);
		int out_filedes = open(cmd->output, O_WRONLY | O_CREAT,
		S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
		dup2(out_filedes, STDOUT_FILENO);
		close(out_filedes);
	}

	execvp(cmd->args[0], cmd->args);
	fprintf(stderr, "Could not execute command: %s\n", cmd->args[0]);
	exit(127);
}

void print_array(char *a[]) {
	printf("{ ");
	fflush(stdout);
	while (*a != NULL) {
		printf("%d", (int) a);
		printf("'%s', ", *a);
		fflush(stdout);
		++a;
	}
	printf(" }");
	fflush(stdout);
}

void debug(char *msg) {
	fprintf(stderr, "%s\n", msg);
	fflush(stderr);
}

void reset_commands(COMMAND (*commands)[]) {
	for (int i = 0; i < MAXCOMMANDS; ++i)
		reset_command(*commands + i);
}

void reset_command(COMMAND *command) {
	command->filled = false;
	command->args[0] = NULL;
	command->in_pipe_set = false;
	command->out_pipe_set = false;
	command->input = NULL;
	command->output = NULL;
}
