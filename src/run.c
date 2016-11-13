#include "run.h"
#include "common.h"
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>

struct {
	int size;
	int pipes[MAXCOMMANDS - 1][2];
} pipes;

static bool execute_builtin(COMMAND *cmd);
static void create_pipes(int npipes);
static void install_pipes(COMMAND (*commands)[], int ncmd);
static void create_processes(COMMAND (*commands)[], bool blocking, int ncmd, pid_t (*pids)[]);
static pid_t create_process(COMMAND *cmd, bool foreground);
static void exec_command(COMMAND *cmd);
static void close_pipes();
static void wait_for_processes(COMMAND (*commands)[], int ncmd, pid_t (*pids)[]);

void run_commands(COMMAND (*commands)[], int ncmd, bool blocking) {
	pid_t pids[MAXCOMMANDS];

	if (execute_builtin(*commands))
		return;

	create_pipes(ncmd - 1);
	install_pipes(commands, ncmd);
	create_processes(commands, blocking, ncmd, &pids);
	close_pipes();
	wait_for_processes(commands, ncmd, &pids);
}

void exit_shell(void) {
	printf("Good-bye!\n");
	exit(0);
}

static void install_pipes(COMMAND (*commands)[], int ncmd) {
	for (int i = 0; i < ncmd; ++i) {
		COMMAND *cmd = *commands + i;
		if (i > 0) {
			cmd->in_pipe_index = i - 1;
			COMMAND *prev_cmd = *commands + (i - 1);
			prev_cmd->out_pipe_index = i - 1;
		}
	}
}

static void create_processes(COMMAND (*commands)[], bool blocking, int ncmd, pid_t (*pids)[]) {
	for (int i = 0; i != ncmd	; ++i) {
		COMMAND *cmd = *commands + i;
		(*pids)[i] = create_process(cmd, blocking);
	}
}

static void wait_for_processes(COMMAND (*commands)[], int ncmd, pid_t (*pids)[]) {
	int status;
	for (int i = 0; i != ncmd; ++i) {
		if (waitpid((*pids)[i], &status, 0) != (*pids)[i])
			fprintf(stderr, "Error waiting for command to finish: %s",
					(*commands)[i].args[0]);
	}
}

static bool execute_builtin(COMMAND *cmd) {
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

static pid_t create_process(COMMAND *cmd, bool foreground) {
	pid_t pid;
	if (foreground) {
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
	}

	return pid;
}

static void exec_command(COMMAND *cmd) {
	if (cmd->in_pipe_index != -1) {
		dup2(pipes.pipes[cmd->in_pipe_index][0], STDIN_FILENO);
	} else if (cmd->input != NULL) {
		int in_filedes = open(cmd->input, O_RDONLY);
		dup2(in_filedes, STDIN_FILENO);
		close(in_filedes);
	}

	if (cmd->out_pipe_index != -1) {
		dup2(pipes.pipes[cmd->out_pipe_index][1], STDOUT_FILENO);
	} else if (cmd->output != NULL) {
		int out_filedes = open(cmd->output, O_WRONLY | O_CREAT,
		S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
		dup2(out_filedes, STDOUT_FILENO);
		close(out_filedes);
	}

	close_pipes();
	execvp(cmd->args[0], cmd->args);
	fprintf(stderr, "Could not execute command: %s\n", cmd->args[0]);
	exit(127);
}

static void create_pipes(int npipes) {
	for (int i = 0; i != npipes; ++i)
		pipe(pipes.pipes[i]);
	pipes.size = npipes;
}

static void close_pipes() {
	for (int i = 0; i != pipes.size; ++i) {
		close(pipes.pipes[i][0]);
		close(pipes.pipes[i][1]);
	}
	pipes.size = 0;
}

